pushd %~dp0
clang dllmain.cpp -ffunction-sections -fdata-sections -Wno-invalid-source-encoding -shared -DUNICODE -o lua53-64.dll -fms-extensions -Wl,-s,-O3,-icf=all,--gc-sections,--disable-runtime-pseudo-reloc,--entry=DllMain -O3 -flto=full
popd