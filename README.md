# SbiePatch
> 只有魔法才能打败魔法！  

![CLang-20](https://img.shields.io/badge/NO-WDK-59900C?logo=llvm)

## 📖 简介
免费且易于使用的[Sandboxie-Plus](https://github.com/sandboxie-plus/Sandboxie)  

本项目部分基于[CECheater](https://github.com/TechForBad/CECheater)并对其代码做出如下更改
1. 通过驱动名称传入PSLoadedModuleList以便遍历模块 （它那个IoCtl啥都缺，算不上真正的加载驱动）
2. 删除了所有对C++标准库的使用

## 🧩 构成

| 项目名称                      | 输出文件          | 功能描述                  |
|---------------------------|---------------|-----------------------|
| **Dll2Lib**               | Dll2Lib.exe   | 从DLL文件生成.a用于链接        |
| **SbieLoader(CECheater)** | lua53-64.dll  | 替换CheatEngine导入的同名dll |
| **SbiePatch**             | SbiePatch.sys | 动态替换SbieDrv驱动程序的公钥    |
| **SbieSign**              | SbieSign.exe  | 生成新的证书、公钥和签名          |
| **SbieWipe**              | SbieWipe.exe  | 清除证书屏蔽列表方便降级          |

## 🛠️ 编译

### 😈 环境要求
- LLVM-MinGW (CLang 20+)
- Sandboxie-Plus 原始二进制文件
- 无需Windows SDK或Windows驱动SDK

### 🖊 编译配置 (通过修改build.bat)
1. 注意SbieSign的私钥为了安全是不保存的一次性的，意味着**一旦程序退出，就不能再生成相同公钥的签名了**！
2. (v1.2) 构建不是无人参与的，需要你设置生成证书的用户名
3. 构建在10秒内完成，也无需联网，所以不用担心要在电脑之前等很久
4. 你可以修改构建脚本来为你提供的其它程序生成sig文件
5. **由于GIT会强制把文件转换为UTF8编码，所以大部分包含中文的bat脚本无法执行** 两种解决方案如下
   1. 使用合适的工具，例如IDE，将它们转换为你电脑的本地编码（GBK）
   2. 在终端中执行chcp 65001以切换到UTF8模式

### 🐙 编译
1. 从官方Sandboxie-Plus复制以下文件到 `bin/` 目录：
    - SandMan.exe
    - SbieCtrl.exe
    - SbieSvc.exe
    - Start.exe
    - SbieDll.Dll

2. 执行构建脚本：
```bash
build.bat
```

3. 替换原版文件：  
将项目文件夹(build.bat)目录中生成的所有exe和sig文件替换原版文件  
* 编译时还生成了bin_loader/SbiePatch.sys

## 🚀 部署

### ⚙ 修补驱动
> 为了防止蓝屏，每次开机只能进行一次修补，如果忘记在修补之前加载SbieDrv请重启电脑

以管理员身份执行下列命令 (你可以将`bin_loader`复制到任何你喜欢的目录，例如沙箱安装位置)
```bash
# 禁止沙箱服务开机自启
sc config SbieSvc start=demand
# 停止沙箱服务
net stop SbieSvc
# 修补驱动程序
.\bin_loader\patch.bat
# 设置开机自启动
schtasks /create /tn "SbiePatch" /tr "%~dp0\bin_loader\patch.bat" /sc onstart /rl highest
```

### 📕 导入证书
> 修补驱动后需要重启UI才能在UI中导入证书 
* 将生成的 `Certificate.dat` 复制到Sandboxie安装目录

* **或**在管理界面手动导入证书：
   ```
   SandMan -> 选项 -> 捐赠支持
   ```

### 😋 Enjoy
1. 享受你的完整沙盒体验

## ⚠️ 安全声明
> 本方案涉及系统驱动修改，作者不对产生的任何问题负责。

可能产生的问题包括并不限于：
1. 🟦 系统蓝屏
2. 🔥 数据丢失
3. 💥 沙箱逃逸
4. 🤬 (原)作者跑路

## 📜 许可证
本项目除SbieLoader(CECheater)外的部分以 GPL 协议开源。  
CECheater开源协议未知，若作者有意见，可以Diff形式发布更改。  
CheatEngine开源协议未知，若作者有意见，可以删去。
