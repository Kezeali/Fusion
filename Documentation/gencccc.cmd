@echo off 

set CCCC_ENV="E:\Program Files\CCCC\cccc_env.bat"
set CCCC_EXE=cccc.exe

if not exist %CCCC_ENV% goto noEnv
call %CCCC_ENV%

if not exist %CCCC_PATH_DIR%\%CCCC_EXE% goto noExe
if not exist %CCCC_WORK_DIR% goto noWorkDir

path %CCCC_PATH_DIR%;%path%

rem Start the shell
rem %COMSPEC% 
goto done

:noEnv
echo The file %CCCC_ENV% was not found.
echo This file is required to define the command line environment for CCCC.
echo Please uninstall and reinstall CCCC to get a working command line environment.
pause
goto end

:noExe
echo The executable %CCCC_EXE% was not found in %CCCC_PATH_DIR%.
echo Please uninstall and reinstall CCCC to get a working command line environment.
pause
goto end

:noWorkDir
echo The shell variable CCCC_WORK_DIR is defined as %CCCC_WORK_DIR% but this 
echo directory does not exist.
echo This variable is required to define the initial work directory
echo for a session using the CCCC command line environment.
echo Please uninstall and reinstall CCCC to get a working command line environment.
pause
goto end

:done
dir /B /S ..\FusionEngine\ | cccc.exe - --outdir=cccc

:end
