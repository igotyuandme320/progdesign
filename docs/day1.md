# 神气的盒子：第1天版本

这是第一天的工程和界面原型，可以编译运行、填写表单并切换四个标签页。灰色按钮尚未接入业务，不验证密码、不保存文件。没有截图功能。

## 三人当天的代码工作

| 人员 | 编写内容 | 对应文件 |
| --- | --- | --- |
| 我（Leader、构思者） | 确定功能和结构，建立工程、程序入口与业务接口骨架 | CMakeLists.txt、main.cpp、boxmanager.h / .cpp |
| 组员A | 根据方案编写窗口、表单布局和标签导航 | mainwindow.h / .cpp |
| 组员B | 根据字段约定编写账户、航班、乘客结构体及注释 | models.h |

第二天增加登录、字段检查、登记发放和文件保存；第三天增加回收与遗失查询；第四天增加评价、意见和抽奖；第五天联调完善。

## Mac 构建运行

需要 CMake 3.21+、C++17 编译器、Qt 6.5+ Widgets。在 day1 目录执行：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew
cmake --build build -j 4
./build/MagicBox.app/Contents/MacOS/MagicBox
```

本机编译后的应用也可以直接双击 `build/MagicBox.app`。该开发构建使用本机 Qt。

## Windows + VS Code

安装 Qt MinGW 64 位及配套 MinGW、CMake、Ninja，安装 VS Code 的 C/C++、CMake Tools 扩展。用 VS Code 打开 **day1 文件夹**，在 PowerShell 执行：

```powershell
Copy-Item .vscode\windows\*.json .vscode\ -Force
```

把 `.vscode/settings.json` 中 Qt、MinGW、CMake、Ninja 的示例路径全部换成本机路径；如已安装 clangd，在该工作区禁用它。重载窗口后按 Ctrl+Shift+B 编译，再从「终端 → 运行任务」选择「MagicBox: 运行 Windows」。Windows 尚未实测。