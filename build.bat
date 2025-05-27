@echo off
set PATH=%PATH%;E:\llvm-mingw-20250514-msvcrt-x86_64\bin
clang -v >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ��⵽��δ��װClang����δ����ӵ�PATH����������
    echo ����Ŀ��ҪClang�Ա����ں���������
    echo �����·���ַ����LLVM-Mingw��100MB��
    echo https://github.com/mstorsjo/llvm-mingw/releases
    echo.
    pause
)

setlocal enabledelayedexpansion

if not exist SbieSign\SbieSign.exe call SbieSign\build.bat
if not exist bin_loader\lua53-64.dll (
    call SbieLoader\build.bat
    if !errorlevel! neq 0 (
        echo Error: Failed to compile SbieLoader
        exit /b 1
    )
    move SbieLoader\lua53-64.dll bin_loader\
)

if not exist bin\SandMan.exe goto missingFile
if not exist bin\SbieCtrl.exe goto missingFile
if not exist bin\SbieSvc.exe goto missingFile
if not exist bin\Start.exe goto missingFile
if not exist bin\UpdUtil.exe goto missingFile

copy bin\* .

:: ����ǩ�����߲��������
SbieSign\SbieSign.exe > temp.txt 2> Certificate.dat

:: ������һ���Ƿ�Ϊ"All done"
set "last_line="
for /f "delims=" %%a in (temp.txt) do set "last_line=%%a"
if not "!last_line!"=="All done" (
    echo Error: Signature failed!
    echo -----------------------------------
    echo !last_line!
    del temp.txt
    exit /b 1
)

:: ��ȡ��һ������
set "first_line="
< temp.txt set /p first_line=
del temp.txt

:: �滻Դ�����е�ռλ��
powershell -Command "(Get-Content 'SbiePatch\SbiePatch.c.tpl') -replace 'PLACEHOLDER', '!first_line!' | Set-Content 'SbiePatch\SbiePatch.c'"
if !errorlevel! neq 0 (
    echo Error: Failed to update source code
    exit /b 1
)

:: ִ�б���ű�
call SbiePatch\build.bat
if !errorlevel! neq 0 (
    echo Error: Build failed with code !errorlevel!
    exit /b 1
)

:: �������ɵ�DLL�ļ�
move /Y SbiePatch\SbiePatch.sys bin_loader\
if !errorlevel! neq 0 (
    echo Error: Failed to copy DLL file
    exit /b 1
)

echo All done
endlocal
exit /b 0

:missingFile
echo ���в��ֻ�ȫ���ļ�ȱʧ���븴�Ƶ�bin�ļ���
echo 1. SandMan.exe
echo 2. SbieCtrl.exe
echo 3. SbieSvc.exe
echo 4. Start.exe
echo 5. UpdUtil.exe
exit /b 1