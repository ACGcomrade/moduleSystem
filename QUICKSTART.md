# Module System - 快速上手指南

## 🚀 5分钟快速开始

### macOS 用户

```bash
# 1. 进入项目目录
cd /path/to/moduleSystem

# 2. 生成 Xcode 项目
bash generate_xcode.sh

# 3. 在 Xcode 中打开
open build_xcode/ModuleSystem.xcodeproj

# 4. 点击 Run (⌘R)
```

### 或者使用命令行

```bash
# 1. 配置和构建
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
make

# 2. 运行
./bin/ModuleSystem
```

## 📱 基本操作演示

### 1️⃣ 创建第一个模块

启动应用后：
- 点击菜单 `Modules > Create Calculator Module`
- 或点击底部绿色按钮 `Create Calculator Module`

模块会自动出现在白板的第一个槽位中。

### 2️⃣ 创建多个模块

继续创建更多模块：
- 每个新模块会自动放入下一个空槽位
- 当所有槽位满了，系统会自动创建新槽位
- 白板会出现水平滚动条

### 3️⃣ 拖拽模块

**拖出成独立窗口**：
1. 按住模块顶部标题栏
2. 向外拖拽超过 50 像素
3. 释放鼠标 → 模块变成独立窗口

**拖回白板**：
1. 按住独立窗口的标题栏
2. 拖回主窗口白板区域
3. 目标槽位会高亮显示（绿色边框）
4. 释放鼠标 → 模块回到白板

### 4️⃣ 双击切换

- 双击模块标题栏
- 在嵌入/独立状态间快速切换

### 5️⃣ 关闭模块

- 点击模块右上角的 **×** 按钮
- 确认对话框中选择 "Yes"
- 模块被删除，槽位变成空的

## 🎨 界面元素说明

```
┌─────────────────────────────────────────────────┐
│  Module System - Drag modules out or create... │ ← 标题栏
├─────────────────────────────────────────────────┤
│  ┌─────────┐  ┌─────────┐  ┌─────────┐         │
│  │ Module  │  │ Module  │  │  Slot 3 │         │ ← 白板槽位
│  │    1    │  │    2    │  │  Empty  │   ═══►  │   (水平滚动)
│  │         │  │         │  │         │         │
│  └─────────┘  └─────────┘  └─────────┘         │
├─────────────────────────────────────────────────┤
│  [Create Calculator Module]  Unlimited modules  │ ← 控制区
└─────────────────────────────────────────────────┘
```

## 🔍 功能测试清单

试试这些功能：

- [ ] 创建 1 个模块
- [ ] 创建 5+ 个模块（观察白板自动扩展）
- [ ] 拖出一个模块成独立窗口
- [ ] 拖回一个模块到白板
- [ ] 双击切换模块状态
- [ ] 关闭一个模块
- [ ] 在多个模块间切换焦点
- [ ] 调整主窗口大小
- [ ] 水平滚动白板

## 💡 提示和技巧

### 白板管理
- **自动扩展**: 系统始终保持至少 3 个空槽位
- **无限制**: 可以创建任意数量的模块
- **智能排列**: 新模块总是放入第一个空槽位

### 拖拽技巧
- 拖拽时会有绿色高亮反馈
- 拖出距离需要超过 50 像素
- 可以在多个显示器间拖拽

### 键盘快捷键
- `⌘Q` - 退出应用
- `⌘W` - 关闭主窗口

## 🔨 下一步

### 创建自己的模块

1. 复制 `include/modules/CustomModuleTemplate.h`
2. 修改类名和功能
3. 在 `ModuleManager` 中注册
4. 在 `MainWindow` 菜单中添加创建选项

详细步骤见 [README.md](README.md) 的"开发指南"部分。

### 学习源码

推荐阅读顺序：
1. [ModuleBase.h](include/modules/ModuleBase.h) - 理解模块基类
2. [ModuleManager.h](include/modules/ModuleManager.h) - 理解模块管理
3. [MainWindow.h](include/MainWindow.h) - 理解白板系统
4. [CalculatorModule.h](include/modules/CalculatorModule.h) - 参考实现

## ❓ 常见问题

**Q: 模块拖不出来？**
A: 确保拖拽距离超过 50 像素，系统会自动检测。

**Q: 如何删除空槽位？**
A: 空槽位会自动管理，始终保持 3 个以便随时创建新模块。

**Q: 模块数据会保存吗？**
A: 当前版本不保存状态，重启后需要重新创建。可以自己实现持久化。

**Q: 可以改变槽位大小吗？**
A: 在 `MainWindow.h` 中修改 `SLOT_MIN_WIDTH` 和 `SLOT_MIN_HEIGHT` 常量。

**Q: Windows 版本何时发布？**
A: 代码已经跨平台，只需在 Windows 上用相同的 CMake 命令构建。

## 📚 更多资源

- [完整 README](README.md)
- [API 文档](README.md#-api-参考)
- [开发指南](README.md#-开发指南)
- [最佳实践](README.md#-最佳实践)

---

**祝你使用愉快！** 🎉

有问题随时查看完整文档或提交 Issue。
