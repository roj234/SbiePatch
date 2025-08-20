@echo off
set PATH=%PATH%;E:\llvm-mingw-20250514-msvcrt-x86_64\bin
clang -v >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo 检测到您未安装LLVM-Mingw，或未其添加到PATH环境变量中
    echo 本项目需要LLVM-Mingw以编译内核驱动程序
    echo 可在下方网址下载LLVM-Mingw（~100MB）
    echo https://github.com/mstorsjo/llvm-mingw/releases
    echo.
    pause
)

if not exist SbieSign\SbieSign.exe  (
    call SbieSign\build.bat
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to compile SbieSign （%ERRORLEVEL%）
        exit /b 1
    )
)

if not exist Dll2Lib\Dll2Lib.exe  (
    call Dll2Lib\build.bat
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to compile Dll2Lib （%ERRORLEVEL%）
        exit /b 1
    )
)

if not exist bin_loader\lua53-64.dll (
    call SbieLoader\build.bat
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to compile SbieLoader （%ERRORLEVEL%）
        exit /b 1
    )
    move SbieLoader\lua53-64.dll bin_loader\
)

if not exist bin\SandMan.exe goto missingFile
if not exist bin\SbieCtrl.exe goto missingFile
if not exist bin\SbieSvc.exe goto missingFile
if not exist bin\Start.exe goto missingFile
if not exist bin\SbieDll.dll goto missingFile

:: dll不需要修改，所以也不需要复制
copy bin\*.exe .

:: 构建SbieWipe
pushd SbieWipe

:: 从SbieDll生成.a文件用于隐式链接
llvm-objdump -x ..\bin\SbieDll.dll | ..\Dll2Lib\Dll2Lib
del libSbieDll.def

call build.bat
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to compile SbieWipe （%ERRORLEVEL%）
    exit /b 1
)

del libSbieDll.a
move SbieWipe.exe ..\

popd

set SbieUserName=SbiePatch
set /P SbieUserName=证书名称[SbiePatch]:

:: 运行签名工具并捕获输出 会为[参数]额外生成.sig文件
SbieSign\SbieSign.exe %SbieUserName% SbieSvc.exe SandMan.exe "" Start.exe SbieSvc.exe SbieCtrl.exe SandMan.exe SbieWipe.exe > temp.txt 2> Certificate.dat

setlocal enabledelayedexpansion

:: 检查最后一行是否为"All done"
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

:: 获取第一行内容
set "first_line="
< temp.txt set /p first_line=
del temp.txt

:: 替换源代码中的占位符
powershell -Command "(Get-Content 'SbiePatch\SbiePatch.c.tpl') -replace 'PLACEHOLDER', '%first_line%' | Set-Content 'SbiePatch\SbiePatch.c'"
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to generate SbiePatch code
    exit /b 1
)

:: 执行编译脚本
call SbiePatch\build.bat
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to compile SbiePatch （%ERRORLEVEL%）
    exit /b 1
)

move /Y SbiePatch\SbiePatch.sys bin_loader\

echo All done
exit /b 0

:missingFile
echo 下列部分或全部文件缺失，请复制到bin文件夹
echo 1. SandMan.exe
echo 2. SbieCtrl.exe
echo 3. SbieSvc.exe
echo 4. Start.exe
echo 5. SbieDll.dll
exit /b 1
