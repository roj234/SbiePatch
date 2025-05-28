@echo off
set PATH=%PATH%;E:\llvm-mingw-20250514-msvcrt-x86_64\bin
clang -v >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ��⵽��δ��װLLVM-Mingw����δ����ӵ�PATH����������
    echo ����Ŀ��ҪLLVM-Mingw�Ա����ں���������
    echo �����·���ַ����LLVM-Mingw��~100MB��
    echo https://github.com/mstorsjo/llvm-mingw/releases
    echo.
    pause
)

if not exist SbieSign\SbieSign.exe  (
    call SbieSign\build.bat
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to compile SbieSign ��%ERRORLEVEL%��
        exit /b 1
    )
)

if not exist Dll2Lib\Dll2Lib.exe  (
    call Dll2Lib\build.bat
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to compile Dll2Lib ��%ERRORLEVEL%��
        exit /b 1
    )
)

if not exist bin_loader\lua53-64.dll (
    call SbieLoader\build.bat
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to compile SbieLoader ��%ERRORLEVEL%��
        exit /b 1
    )
    move SbieLoader\lua53-64.dll bin_loader\
)

if not exist bin\SandMan.exe goto missingFile
if not exist bin\SbieCtrl.exe goto missingFile
if not exist bin\SbieSvc.exe goto missingFile
if not exist bin\Start.exe goto missingFile
if not exist bin\UpdUtil.exe goto missingFile
if not exist bin\SbieDll.dll goto missingFile

:: dll����Ҫ�޸ģ�����Ҳ����Ҫ����
copy bin\*.exe .

:: ����SbieWipe
pushd SbieWipe

:: ��SbieDll����.a�ļ�������ʽ����
llvm-objdump -x ..\bin\SbieDll.dll | ..\Dll2Lib\Dll2Lib
del libSbieDll.def

call build.bat
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to compile SbieWipe ��%ERRORLEVEL%��
    exit /b 1
)

del libSbieDll.a
move SbieWipe.exe ..\

popd

:: ����ǩ�����߲�������� ��Ϊ[����]��������.sig�ļ�
SbieSign\SbieSign.exe SbieWipe.exe > temp.txt 2> Certificate.dat

setlocal enabledelayedexpansion

:: ������һ���Ƿ�Ϊ"All done"
set "last_line="
for /f "delims=" %%a in (temp.txt) do set "last_line=%%a"
if not "%last_line%"=="All done" (
    echo Error: Signature failed!
    echo -----------------------------------
    echo %last_line%
    del temp.txt
    exit /b 1
)
endlocal

:: ��ȡ��һ������
set "first_line="
< temp.txt set /p first_line=
del temp.txt

:: �滻Դ�����е�ռλ��
powershell -Command "(Get-Content 'SbiePatch\SbiePatch.c.tpl') -replace 'PLACEHOLDER', '%first_line%' | Set-Content 'SbiePatch\SbiePatch.c'"
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to generate SbiePatch code
    exit /b 1
)

:: ִ�б���ű�
call SbiePatch\build.bat
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to compile SbiePatch ��%ERRORLEVEL%��
    exit /b 1
)

move /Y SbiePatch\SbiePatch.sys bin_loader\

echo All done
exit /b 0

:missingFile
echo ���в��ֻ�ȫ���ļ�ȱʧ���븴�Ƶ�bin�ļ���
echo 1. SandMan.exe
echo 2. SbieCtrl.exe
echo 3. SbieSvc.exe
echo 4. Start.exe
echo 5. UpdUtil.exe
echo 6. SbieDll.dll
exit /b 1