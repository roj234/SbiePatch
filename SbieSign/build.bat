pushd %~dp0
clang SbieSign.c -o SbieSign.exe -l bcrypt -l crypt32 -Wl,-O3,-icf=all,-s -O3 -flto=full
popd