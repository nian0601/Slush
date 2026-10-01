@echo off
setlocal
set "BUILD_ROOT=%~dp0"
set "BUILD_CONFIGURATION=Debug"
set "BUILD_TARGET="

:ParseArguments
if "%~1"=="" goto FindMSBuild
if /i "%~1"=="Debug" (
	set "BUILD_CONFIGURATION=Debug"
	goto NextArgument
)
if /i "%~1"=="Release" (
	set "BUILD_CONFIGURATION=Release"
	goto NextArgument
)
if /i "%~1"=="rebuild" (
	set "BUILD_TARGET=-t:Rebuild"
	goto NextArgument
)
echo Usage: build.bat [Debug^|Release] [rebuild]
exit /b 1

:NextArgument
shift
goto ParseArguments

:FindMSBuild
set "BUILD_MSBUILD="
set "BUILD_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%BUILD_VSWHERE%" goto FindOnPath
for /f "usebackq delims=" %%I in (`"%BUILD_VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do if not defined BUILD_MSBUILD set "BUILD_MSBUILD=%%I"
if defined BUILD_MSBUILD goto Build

:FindOnPath
for /f "delims=" %%I in ('where MSBuild.exe 2^>nul') do if not defined BUILD_MSBUILD set "BUILD_MSBUILD=%%I"
if defined BUILD_MSBUILD goto Build
echo ERROR: MSBuild was not found via vswhere or PATH. Install Visual Studio with C++ build tools.
exit /b 1

:Build
if not exist "%BUILD_ROOT%Build_Output\" mkdir "%BUILD_ROOT%Build_Output"
if not exist "%BUILD_ROOT%Build_Output\" (
	echo ERROR: Could not create Build_Output.
	exit /b 1
)
rem Clear diagnostics so a failed invocation cannot reuse an earlier build's counts.
type nul > "%BUILD_ROOT%Build_Output\build-errors.log"
type nul > "%BUILD_ROOT%Build_Output\build-warnings.log"
"%BUILD_MSBUILD%" "%BUILD_ROOT%Solution\Solution.sln" -p:Configuration=%BUILD_CONFIGURATION% -p:Platform=x86 -m -nodeReuse:false -nologo %BUILD_TARGET% -noconsolelogger "-flp1:LogFile=%BUILD_ROOT%Build_Output\build.log;Verbosity=normal" "-flp2:LogFile=%BUILD_ROOT%Build_Output\build-errors.log;ErrorsOnly;NoSummary" "-flp3:LogFile=%BUILD_ROOT%Build_Output\build-warnings.log;WarningsOnly;NoSummary"
set "BUILD_EXIT_CODE=%ERRORLEVEL%"
powershell.exe -NoProfile -Command "$errors = @(Get-Content -LiteralPath ($env:BUILD_ROOT + 'Build_Output\build-errors.log') | Where-Object { $_.Trim() } | Select-Object -Unique); $warnings = @(Get-Content -LiteralPath ($env:BUILD_ROOT + 'Build_Output\build-warnings.log') | Where-Object { $_.Trim() }); $errors | ForEach-Object { Write-Output $_ }; Write-Output ('Warnings: ' + $warnings.Count); $result = if ($env:BUILD_EXIT_CODE -eq '0') { 'SUCCEEDED' } else { 'FAILED' }; Write-Output ('BUILD ' + $result + ' (' + $errors.Count + ' errors, ' + $warnings.Count + ' warnings), full log: ' + $env:BUILD_ROOT + 'Build_Output\build.log')"
exit /b %BUILD_EXIT_CODE%
