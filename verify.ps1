$ErrorActionPreference = 'Stop'

function RunGame
{
	param([string] $aGame, [string[]] $anArguments, [double] $aTimeout)
	$executable = Join-Path $PSScriptRoot "Workbed\$aGame\$aGame.exe"
	$process = Start-Process -FilePath $executable -ArgumentList $anArguments -WorkingDirectory $PSScriptRoot -WindowStyle Hidden -PassThru
	try
	{
		if (!$process.WaitForExit([int]($aTimeout * 1000)))
		{
			Write-Host "$aGame timeout after ${aTimeout}s (PID $($process.Id)); stopping only this process"
			try { $process.Kill() } catch { if (!$process.HasExited) { throw } }
			$process.WaitForExit()
			return -1
		}
		return $process.ExitCode
	}
	finally
	{
		$process.Dispose()
	}
}

$passed = $true
try
{
	$configuration = 'Debug'
	$rebuild = $false
	$timeout = 120.0
	$scenarios = @()
	$games = @('ActionGame', 'BossMonster', 'TopDownGame')
	for ($index = 0; $index -lt $args.Count; ++$index)
	{
		$argument = $args[$index]
		if ($argument -in @('Debug', 'Release'))
		{
			$configuration = $argument
		}
		elseif ($argument -eq 'rebuild')
		{
			$rebuild = $true
		}
		elseif ($argument -eq '-timeout')
		{
			++$index
			if ($index -ge $args.Count -or ![double]::TryParse($args[$index], [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref] $timeout) -or
				[double]::IsNaN($timeout) -or [double]::IsInfinity($timeout) -or $timeout -lt 0.001 -or $timeout -gt 2147483)
			{
				throw '-timeout requires seconds between 0.001 and 2147483'
			}
		}
		elseif ($argument -in $games)
		{
			++$index
			if ($index -ge $args.Count) { throw "Missing scenario path after $argument" }
			$scenario = $args[$index]
			if (![IO.Path]::IsPathRooted($scenario)) { $scenario = Join-Path $PSScriptRoot $scenario }
			if (!(Test-Path -LiteralPath $scenario -PathType Leaf)) { throw "Scenario not found: $scenario" }
			$game = $games | Where-Object { $_ -eq $argument }
			$scenarios += [pscustomobject]@{ Game = $game; Path = [IO.Path]::GetFullPath($scenario) }
		}
		else
		{
			throw "Unknown argument '$argument'. Usage: verify.bat [Debug|Release] [rebuild] [<Game> <scenario-path>]... [-timeout <s>]"
		}
	}

	$buildArguments = @($configuration)
	if ($rebuild) { $buildArguments += 'rebuild' }
	& (Join-Path $PSScriptRoot 'build.bat') @buildArguments
	if ($LASTEXITCODE -ne 0) { throw 'Build failed; no games were launched' }

	foreach ($game in $games)
	{
		$report = Join-Path $PSScriptRoot "Workbed\$game\Debug\unittest_results.txt"
		if (Test-Path -LiteralPath $report) { Remove-Item -LiteralPath $report }
		$exitCode = RunGame $game @('-runtestsonly') 120
		$lines = @(Get-Content -LiteralPath $report)
		Write-Host "$game unit tests (exit $exitCode):"
		$lines | Where-Object { $_ -match '^(UNITTEST_RESULT:|FAIL )' } | ForEach-Object { Write-Host $_ }
		if ($exitCode -ne 0 -or !$lines.Count -or $lines[0] -notmatch '^UNITTEST_RESULT: PASS \(0 failed /' -or @($lines | Where-Object { $_ -match '^FAIL ' }).Count)
		{
			$passed = $false
		}
	}

	$counts = @{}
	foreach ($scenario in $scenarios)
	{
		$game = $scenario.Game
		if (!$counts.ContainsKey($game)) { $counts[$game] = 0 }
		++$counts[$game]
		$logName = 'verify_' + $game.ToLowerInvariant()
		if ($counts[$game] -gt 1) { $logName += '_' + $counts[$game] }
		$log = Join-Path $PSScriptRoot "Workbed\$game\Debug\$logName.txt"
		if (Test-Path -LiteralPath $log) { Remove-Item -LiteralPath $log }
		$exitCode = RunGame $game @('-hidewindow', '-debugscript', ('"' + $scenario.Path + '"'), '-logname', $logName) $timeout
		$lines = @(Get-Content -LiteralPath $log)
		$errors = @($lines | Where-Object { $_ -match '^\[ERROR\]' })
		$warnings = @($lines | Where-Object { $_ -match '^\[WARN\]' })
		Write-Host "$game scenario '$($scenario.Path)': exit $exitCode, $($errors.Count) errors, $($warnings.Count) warnings; log: $log"
		$errors | ForEach-Object { Write-Host $_ }
		if ($exitCode -ne 0 -or $errors.Count) { $passed = $false }
	}
}
catch
{
	Write-Host "ERROR: $_"
	$passed = $false
}

if ($passed)
{
	Write-Host 'VERIFY PASSED'
	exit 0
}
Write-Host 'VERIFY FAILED'
exit 1
