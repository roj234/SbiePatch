
pushd %~dp0
clang -O3 -flto=full -ffunction-sections -fdata-sections -pipe SbieWipe.c -o SbieWipe.exe -Wl,-s,-O3,-icf=all,--gc-sections,--disable-runtime-pseudo-reloc,libSbieDll.a
popd