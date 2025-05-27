PUSHD %~dp0
clang SbiePatch.c -shared -o SbiePatch.sys -l ntoskrnl -fms-extensions -Wl,-s,--entry=DriverEntry -nostartfiles -nodefaultlibs -ffreestanding -Wl,--disable-dynamicbase,--disable-high-entropy-va,--disable-nxcompat,--disable-runtime-pseudo-reloc -O3 @arg_tiny_unsafe.txt && del SbiePatch.c
popd