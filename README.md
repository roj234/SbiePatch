# SbiePatch
> 只有魔法才能打败魔法！  

![CLang-20](https://img.shields.io/badge/NO-WDK-59900C?logo=llvm)

## 📖 简介
免费且易于使用的[Sandboxie-Plus](https://github.com/sandboxie-plus/Sandboxie)  

本项目部分基于[CECheater](https://github.com/TechForBad/CECheater)并对其代码做出如下更改
1. 通过驱动名称传入PSLoadedModuleList以便遍历模块 （它那个IoCtl啥都缺，算不上真正的加载驱动）
2. 删除了所有对C++标准库的使用

## 🧩 构成

| 项目名称           | 输出文件          | 功能描述                  |
|----------------|---------------|-----------------------|
| **SbieLoader(CECheater)** | lua53-64.dll  | 替换CheatEngine导入的同名dll |
| **SbiePatch**  | SbiePatch.sys | 动态替换SbieDrv驱动程序的公钥    |
| **SbieSign**   | SbieSign.exe  | 生成新的证书、公钥和签名          |

## 🛠️ 编译

### 😈 环境要求
- LLVM-MinGW (CLang 20+)
- Sandboxie-Plus 原始二进制文件
- 无需Windows SDK或Windows驱动SDK

### 🐙 编译
1. 从官方Sandboxie-Plus复制以下文件到 `bin/` 目录：
    - SandMan.exe
    - SbieCtrl.exe
    - SbieSvc.exe
    - Start.exe
    - UpdUtil.exe

2. 执行构建脚本：
```bash
build.bat
```

3. 替换原版文件：  
将项目文件夹中生成的上述程序修改版、它们的签名、Certificate.dat替换原版文件  
* 编译时还生成了bin_loader/SbiePatch.sys

## 🚀 部署

### ⚙ 驱动修补
1. 执行`bin_loader/patch.bat`并将其设为开机启动
```bash
# 以管理员身份执行驱动修补
.\bin_loader\patch.bat

# 设置开机自启动
schtasks /create /tn "SbiePatch" /tr "%~dp0\bin_loader\patch.bat" /sc onstart
```

### 📕 证书配置
1. 将生成的 `Certificate.dat` 复制到Sandboxie安装目录

2. 或在管理界面手动导入证书：
   ```
   SandMan -> Configure -> System Certificates
   ```

### 😋 Enjoy
1. 享受你的完整沙盒体验

## ⚠️ 安全声明
> 本方案涉及系统驱动修改，作者不对产生的任何问题负责。

可能产生的问题包括但不限于：
1. 🟦 系统蓝屏
2. 🔥 数据丢失
3. 💥 沙箱逃逸
4. 🤬 (原)作者跑路

## 📜 许可证
本项目除SbieLoader(CECheater)外的部分以 GPL 协议开源。  
CECheater开源协议未知，若作者有意见，可以Diff形式发布更改。  
CheatEngine开源协议未知，若作者有意见，可以删去。
