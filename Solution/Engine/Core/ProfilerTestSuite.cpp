#include "stdafx.h"
#include "Core/Profiler.h"
#include "Core/ProfilerTestSuite.h"
#include "Core/AssetRepository.h"
#include "Core/DebugScriptRunner.h"
#include <FW_UnitTestSuite.h>
#include <cmath>
#include <share.h>

namespace Slush
{
	namespace ProfilerTestSuite
	{
		void TestNestedMarkersAndPendingWork()
		{
			Profiler profiler;
			const int parent = profiler.RegisterMarker("Parent");
			const int child = profiler.RegisterMarker("Child");
			const int leaf = profiler.RegisterMarker("Leaf");
			FW_TEST_CHECK(profiler.RegisterMarker("Parent") == parent, "Names aggregate across sites");
			profiler.BeginMarker(leaf, 0);
			profiler.EndMarker(leaf, 1000);
			profiler.BeginFrame(1000);
			profiler.BeginMarker(parent, 1000);
			profiler.BeginMarker(child, 2000);
			profiler.BeginMarker(leaf, 3000);
			profiler.EndMarker(leaf, 5000);
			profiler.EndMarker(child, 7000);
			profiler.EndMarker(parent, 11000);
			profiler.EndFrame(11000);
			const auto& markers = profiler.GetMarkers();
			FW_TEST_CHECK(markers[parent].myFrameSelfMs == 5, "Parent subtracts the whole child");
			FW_TEST_CHECK(markers[child].myFrameSelfMs == 3, "Child subtracts grandchild once");
			FW_TEST_CHECK(markers[leaf].myFrameSelfMs == 3, "Outside-frame work belongs to next frame");
			FW_TEST_CHECK(markers[leaf].myCalls == 2 && markers[leaf].myMaxCallsPerFrame == 2, "Repeated calls counted");
			profiler.BeginFrame(11000);
			profiler.EndFrame(12000);
			FW_TEST_CHECK(markers[leaf].myCalls / static_cast<double>(profiler.GetFrameCount()) == 1, "Calls/frame includes idle frames");
			FW_TEST_CHECK(markers[leaf].myFrameInclusiveMs == 3, "Pending totals reset after a frame");
		}

		void TestRankings()
		{
			Profiler profiler;
			const int expensive = profiler.RegisterMarker("OneExpensiveCall");
			const int frequent = profiler.RegisterMarker("ManyCheapCalls");
			const int parent = profiler.RegisterMarker("Parent");
			profiler.BeginFrame(0);
			profiler.BeginMarker(parent, 0);
			profiler.BeginMarker(expensive, 0);
			profiler.EndMarker(expensive, 10000);
			for (int i = 0; i < 20; ++i)
			{
				profiler.BeginMarker(frequent, 10000 + i * 1000);
				profiler.EndMarker(frequent, 11000 + i * 1000);
			}
			profiler.EndMarker(parent, 30000);
			profiler.EndFrame(30000);
			FW_TEST_CHECK(profiler.GetRanking(Profiler::Ranking::WorstSingleCall, 3)[1] == expensive, "Single expensive call outranks repeated cheap calls");
			FW_TEST_CHECK(profiler.GetRanking(Profiler::Ranking::HeaviestPerFrameAvg, 1)[0] == frequent, "Self ranking finds repeated cheap calls through parent");
			FW_TEST_CHECK(profiler.GetRanking(Profiler::Ranking::WorstFrameContribution, 1)[0] == parent, "Frame contribution uses inclusive totals");
			FW_TEST_CHECK(profiler.GetRanking(Profiler::Ranking::WorstSingleCall, 0).IsEmpty(), "Zero limit returns no rows");
		}

		void TestHistogramAndHistory()
		{
			Profiler profiler;
			for (int i = 1; i <= 1000; ++i)
			{
				profiler.BeginFrame(0);
				profiler.EndFrame(i * 100);
			}
			FW_TEST_CHECK(fabs(profiler.GetPercentile(50) - 50) <= 0.101, "p50 within one bucket");
			FW_TEST_CHECK(fabs(profiler.GetPercentile(95) - 95) <= 0.101, "p95 within one bucket");
			FW_TEST_CHECK(fabs(profiler.GetPercentile(99) - 99) <= 0.101, "p99 within one bucket");
			FW_TEST_CHECK(fabs(profiler.GetAverageMs() - 50.05) < 0.001, "Exact average");
			FW_TEST_CHECK(profiler.GetMinMs() == 0.1 && profiler.GetMaxMs() == 100, "Exact extrema");
			FW_TEST_CHECK(profiler.GetHistory().Count() == 600, "History stays bounded");
			FW_TEST_CHECK(profiler.GetHistory()[999 % 600].myIndex == 999, "Ring retains latest frame index");
			Profiler overflow;
			overflow.BeginFrame(0);
			overflow.EndFrame(1500000);
			FW_TEST_CHECK(overflow.GetPercentile(99) == 1500, "Overflow reports exact maximum");
		}

		void TestSpikes()
		{
			Profiler profiler;
			FW_StaticArray<int, 6> indices;
			for (int i = 0; i < 6; ++i)
			{
				char name[32];
				sprintf_s(name, "Marker%d", i);
				indices[i] = profiler.RegisterMarker(name);
			}
			for (int i = 0; i < 60; ++i)
			{
				profiler.BeginFrame(0);
				profiler.EndFrame(i == 0 ? 100000 : 1000);
			}
			FW_TEST_CHECK(profiler.GetSpikes().IsEmpty(), "Warm-up never spikes");
			profiler.BeginFrame(0);
			profiler.EndFrame(8000);
			FW_TEST_CHECK(profiler.GetSpikes().IsEmpty(), "8 ms floor is strict");
			profiler.SetGameStateName("level");
			profiler.BeginFrame(0);
			for (int i = 0; i < 6; ++i)
			{
				profiler.BeginMarker(indices[i], i * 10000);
				profiler.EndMarker(indices[i], i * 10000 + (i + 1) * 1000);
			}
			profiler.EndFrame(60000);
			FW_TEST_REQUIRE(profiler.GetSpikes().Count() == 1, "Relative spike detected");
			const Profiler::Spike& spike = profiler.GetSpikes()[0];
			FW_TEST_CHECK(spike.myFrameIndex == 61 && spike.myGameState == "level", "Spike context captured");
			for (int i = 0; i < 5; ++i)
				FW_TEST_CHECK(spike.myTop[i].myIndex == indices[5 - i] && spike.myTop[i].mySample.myCalls == 1, "Top five ordered by self time with calls");
			for (int i = 0; i < 700; ++i)
			{
				profiler.BeginFrame(0);
				profiler.EndFrame(1000);
			}
			profiler.BeginFrame(0);
			profiler.EndFrame(9000);
			FW_TEST_CHECK(profiler.GetSpikes().Count() == 2, "Rolling window survives ring wrap");
		}

		void TestReportAndLiveSpikes()
		{
			Profiler profiler;
			FW_String path;
			AssetRepository::GetDebugFilePath("profiler_test.txt", path);
			FW_String htmlPath;
			AssetRepository::GetDebugFilePath("profiler_test.html", htmlPath);
			FW_TEST_REQUIRE(profiler.StartReport(path.GetBuffer()), "Open live report");
			const int marker = profiler.RegisterMarker("KnownMarker");
			const int escaped = profiler.RegisterMarker("Marker<&>\"'");
			profiler.SetGameStateName("level<&>\"'");
			for (int i = 0; i < 61; ++i)
			{
				profiler.BeginFrame(0);
				profiler.BeginMarker(escaped, 0);
				profiler.EndMarker(escaped, 0);
				profiler.BeginMarker(marker, 0);
				profiler.EndMarker(marker, i == 60 ? 50000 : 1000);
				profiler.EndFrame(i == 60 ? 50000 : 1000);
			}
			profiler.AppendNewSpikes();
			FILE* file = nullptr;
			file = _fsopen(path.GetBuffer(), "r", _SH_DENYNO);
			FW_TEST_REQUIRE(file != nullptr, "Live report can be read before shutdown");
			char line[4096];
			fgets(line, sizeof(line), file);
			FW_TEST_CHECK(strncmp(line, "PERF_RESULT: INCOMPLETE", 23) == 0, "Live report has incomplete sentinel");
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			FW_TEST_CHECK(strstr(line, "marker=KnownMarker") != nullptr, "Spike flushed live");
			fclose(file);
			FW_TEST_REQUIRE(profiler.FinishReport(), "Final report writes successfully");
			fopen_s(&file, htmlPath.GetBuffer(), "r");
			FW_TEST_REQUIRE(file != nullptr, "Final report can be read");
			fgets(line, sizeof(line), file);
			FW_TEST_CHECK(strstr(line, "<!doctype html>") == line, "Complete report is HTML");
			const char* sections[] = { "id=\"Frame\"", "id=\"Spikes\"", "id=\"WorstSingleCall\"", "id=\"HeaviestPerFrameAvg\"", "id=\"WorstFrameContribution\"", "id=\"AllMarkers\"" };
			int section = 0;
			int rankingRows = 0;
			int tables = 0;
			int closedTables = 0;
			int escapedMarkers = 0;
			bool summary = false;
			bool escapedState = false;
			bool complete = false;
			while (fgets(line, sizeof(line), file))
			{
				if (strstr(line, "<h2 "))
				{
					FW_TEST_CHECK(section < 6 && strstr(line, sections[section]) != nullptr, "Section order");
					++section;
				}
				if (section >= 3 && section <= 5 && strstr(line, "<tr><td>KnownMarker</td><td>61</td><td>1.000</td>"))
					++rankingRows;
				for (const char* table = line; (table = strstr(table, "<table>")) != nullptr; ++table)
					++tables;
				for (const char* table = line; (table = strstr(table, "</table>")) != nullptr; ++table)
					++closedTables;
				if (strstr(line, "Marker&lt;&amp;&gt;&quot;&#39;"))
					++escapedMarkers;
				summary |= strstr(line, "PERF_RESULT: frames=61") != nullptr;
				escapedState |= strstr(line, "level&lt;&amp;&gt;&quot;&#39;") != nullptr;
				complete |= strstr(line, "</body></html>") != nullptr;
			}
			fclose(file);
			FW_TEST_CHECK(section == 6 && rankingRows == 3, "All sections and expected rankings present");
			FW_TEST_CHECK(tables == 7 && closedTables == tables && complete, "Frame, spike details and all marker tables form a complete document");
			FW_TEST_CHECK(summary && escapedState && escapedMarkers == 5, "Summary preserved and dynamic HTML text escaped in spikes and rankings");
			FW_TEST_REQUIRE(profiler.StartReport(path.GetBuffer()), "Restart report");
			fopen_s(&file, htmlPath.GetBuffer(), "r");
			FW_TEST_CHECK(file == nullptr, "Restart removes stale completed HTML");
			if (file)
				fclose(file);
			FW_TEST_CHECK(profiler.GetFrameCount() == 0 && profiler.GetSpikes().IsEmpty(), "New run resets test measurements");
			FW_TEST_CHECK(profiler.RegisterMarker("KnownMarker") == marker && profiler.GetMarkers()[marker].myCalls == 0, "Reset preserves cached indices");
			profiler.BeginFrame(0);
			profiler.EndFrame(1000);
			FW_TEST_CHECK(profiler.GetHistory()[0].myMarkers[marker].myCalls == 0, "History reset discards old marker totals");
			FW_TEST_CHECK(profiler.FinishReport(), "Empty-marker run writes successfully");
			remove(path.GetBuffer());
			remove(htmlPath.GetBuffer());
		}

		void TestPerfSpikeCommand()
		{
			const char* invalid[] = { "perfspike", "perfspike -1", "perfspike abc", "perfspike nan", "perfspike inf", "perfspike 1e999", "perfspike 1ms", "perfspike 0 extra", "perfspike \"\"" };
			for (const char* line : invalid)
			{
				DebugScriptRunner runner;
				runner.QueueLine(line);
				runner.Step(0);
				FW_TEST_CHECK(runner.HasFailed(), "Invalid perfspike arguments fail the script");
			}
			DebugScriptRunner runner;
			runner.QueueLine("perfspike 0");
			runner.QueueLine("perfspike 0.1");
			runner.Step(0);
			FW_TEST_CHECK(!runner.HasFailed(), "Zero and fractional durations are valid");
		}

		void RunTests()
		{
			FW_UnitTestSuite::BeginSuite("Profiler");
			FW_RUN_TEST(TestNestedMarkersAndPendingWork);
			FW_RUN_TEST(TestRankings);
			FW_RUN_TEST(TestHistogramAndHistory);
			FW_RUN_TEST(TestSpikes);
			FW_RUN_TEST(TestReportAndLiveSpikes);
			FW_RUN_TEST(TestPerfSpikeCommand);
		}
	}
}
