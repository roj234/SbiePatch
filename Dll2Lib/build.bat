pushd %~dp0
clang Dll2Lib.c -o Dll2Lib.exe -Wl,-O3,-icf=all,-s -O3 -flto=full
popd