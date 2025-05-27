@echo off
set PATH=%PATH%;E:\llvm-mingw-20250514-msvcrt-x86_64\bin
clang -v >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo 检测到您未安装Clang，或未其添加到PATH环境变量中
    echo 本项目需要Clang以编译内核驱动程序
    echo 可在下方网址下载LLVM-Mingw（100MB）
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

:: 运行签名工具并捕获输出
SbieSign\SbieSign.exe > temp.txt 2> Certificate.dat

:: 检查最后一行是否为"All done"
set "last_line="
for /f "delims=" %%a in (temp.txt) do set "last_line=%%a"
if not "!last_line!"=="All done" (
    echo Error: Signature failed!
    echo -----------------------------------
    echo !last_line!
    del temp.txt
    exit /b 1
)

:: 获取第一行内容
set "first_line="
< temp.txt set /p first_line=
del temp.txt

:: 替换源代码中的占位符
powershell -Command "(Get-Content 'SbiePatch\SbiePatch.c.tpl') -replace 'PLACEHOLDER', '!first_line!' | Set-Content 'SbiePatch\SbiePatch.c'"
if !errorlevel! neq 0 (
    echo Error: Failed to update source code
    exit /b 1
)

:: 执行编译脚本
call SbiePatch\build.bat
if !errorlevel! neq 0 (
    echo Error: Build failed with code !errorlevel!
    exit /b 1
)

:: 复制生成的DLL文件
move /Y SbiePatch\SbiePatch.sys bin_loader\
if !errorlevel! neq 0 (
    echo Error: Failed to copy DLL file
    exit /b 1
)

echo All done
endlocal
exit /b 0

:missingFile
echo 下列部分或全部文件缺失，请复制到bin文件夹
echo 1. SandMan.exe
echo 2. SbieCtrl.exe
echo 3. SbieSvc.exe
echo 4. Start.exe
echo 5. UpdUtil.exe
exit /b 1