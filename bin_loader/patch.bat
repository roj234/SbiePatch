pushd %~dp0
:: 必然启动失败，只是为了加载驱动
net start SbieSvc
:: 补丁
richstuff-x86_64 -load_by_driver SbiePatch.sys
:: 启动服务 sc start 和 net start 的区别是后者阻塞，在最后不用等它启动成功
sc start SbieSvc
popd