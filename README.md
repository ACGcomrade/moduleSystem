# Module System - 通用模块化系统平台

一个跨平台的通用模块化系统，支持 Windows 和 macOS。提供灵活的白板和模块系统，允许用户通过代码设计自定义功能，无创建数量限制。

## ✨ 核心特性

### 🎯 无限制系统
- ✅ **无模块数量限制** - 创建任意数量的模块
- ✅ **无白板限制** - 支持无限扩展的槽位
- ✅ **动态槽位** - 自动管理和扩展白板空间
- ✅ **智能内存管理** - Qt 智能指针和自动清理

### 🎨 强大的白板系统
- **动态槽位管理** - 自动创建和维护至少 3 个空槽位
- **水平滚动支持** - 容纳任意数量的模块
- **拖拽排列** - 模块可以自由拖出成独立窗口
- **智能高亮** - 拖拽时实时反馈目标槽位
- **响应式布局** - 自适应窗口大小

### 🧩 灵活的模块系统
- **模块基类** - 统一的接口和行为
- **类型安全** - 强类型模块系统
- **生命周期管理** - 自动化的创建和销毁
- **信号槽通信** - 松耦合的模块间通信
- **易于扩展** - 快速添加新模块类型

## 🏗️ 架构设计

### 跨平台支持
- **C++ 核心算法** - 保持业务逻辑跨平台
- **Qt6 UI 框架** - 统一的跨平台界面
- **CMake 构建系统** - 支持多平台构建
- **Xcode 集成** - macOS 开发和调试支持

### 核心组件

```
ModuleSystem/
├── include/
│   ├── MainWindow.h           # 主窗口（白板管理）
│   └── modules/
│       ├── ModuleBase.h       # 模块基类
│       ├── ModuleManager.h    # 模块管理器
│       ├── ExampleModule.h    # 示例模块
│       ├── CalculatorModule.h # 计算器模块
│       └── CustomModuleTemplate.h # 自定义模块模板
└── src/
    ├── main.cpp
    ├── MainWindow.cpp
    └── modules/
        ├── ModuleBase.cpp
        ├── ModuleManager.cpp
        └── ...
```

## 🚀 快速开始

### 依赖要求

- **Qt6** (Core, Widgets)
- **CMake** 3.16+
- **C++17** 编译器
- **macOS**: Xcode 15+ 或 Command Line Tools
- **Windows**: MSVC 2019+ 或 MinGW

### macOS 构建

#### 方法 1: 使用 Xcode（推荐）

```bash
cd moduleSystem
bash generate_xcode.sh
open build_xcode/ModuleSystem.xcodeproj
```

在 Xcode 中：
1. 选择 **ModuleSystem** target
2. 点击 Run (⌘R)

#### 方法 2: 使用 CMake 命令行

```bash
cd moduleSystem
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
make
./bin/ModuleSystem
```

### Windows 构建

```cmd
cd moduleSystem
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" -G "Visual Studio 16 2019" ..
cmake --build . --config Release
.\bin\Release\ModuleSystem.exe
```

## 📚 使用指南

### 基本操作

1. **创建模块**
   - 通过菜单: `Modules > Create Calculator Module`
   - 或点击底部按钮: `Create Calculator Module`
   - 模块会自动放入空槽位

2. **拖拽模块**
   - 按住模块标题栏拖拽
   - 拖出主窗口变成独立窗口
   - 拖回白板放入槽位（会高亮显示目标位置）

3. **关闭模块**
   - 点击模块右上角的 × 按钮
   - 会弹出确认对话框

4. **双击切换**
   - 双击模块标题栏
   - 在嵌入/独立状态间切换

### 白板行为

- **自动扩展**: 当所有槽位被占用时，系统会自动创建新槽位
- **最少空槽位**: 始终保持至少 3 个空槽位可用
- **水平滚动**: 当模块过多时，可以水平滚动白板
- **智能高亮**: 拖拽时目标槽位会高亮显示（绿色边框）

## 🛠️ 开发指南

### 创建自定义模块

#### 步骤 1: 创建模块类

复制并修改 [CustomModuleTemplate.h](include/modules/CustomModuleTemplate.h):

```cpp
// MyCustomModule.h
#ifndef MYCUSTOMMODULE_H
#define MYCUSTOMMODULE_H

#include "modules/ModuleBase.h"
#include <QWidget>
#include <QVBoxLayout>

class MyCustomModule : public ModuleBase {
    Q_OBJECT

public:
    explicit MyCustomModule(QWidget *parent = nullptr);
    ~MyCustomModule();

    void clear() override;
    QWidget* contentWidget() override;

    static ModuleType staticModuleType() { return ModuleBase::Custom; }

private:
    // 你的自定义成员变量
    QWidget* m_contentWidget;
};

#endif
```

#### 步骤 2: 实现模块功能

```cpp
// MyCustomModule.cpp
#include "MyCustomModule.h"

MyCustomModule::MyCustomModule(QWidget *parent)
    : ModuleBase(ModuleBase::Custom, "My Custom Module", parent)
{
    m_contentWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_contentWidget);

    // 添加你的UI组件
    // layout->addWidget(...);

    setMinimumSize(300, 400);
}

MyCustomModule::~MyCustomModule() {}

void MyCustomModule::clear() {
    // 清理模块状态
}

QWidget* MyCustomModule::contentWidget() {
    return m_contentWidget;
}
```

#### 步骤 3: 注册到 ModuleManager

在 [ModuleManager.h](include/modules/ModuleManager.h) 中添加:

```cpp
#include "MyCustomModule.h"

class ModuleManager : public QObject {
    // ...
public:
    MyCustomModule* createMyCustomModule();
private:
    QList<MyCustomModule*> m_myCustomModules;
};
```

在 [ModuleManager.cpp](src/modules/ModuleManager.cpp) 中实现:

```cpp
MyCustomModule* ModuleManager::createMyCustomModule() {
    MyCustomModule* module = new MyCustomModule();
    registerModule(module);
    return module;
}
```

#### 步骤 4: 添加到菜单

在 [MainWindow.cpp](src/MainWindow.cpp) 的 `setupMenuBar()` 中:

```cpp
QAction* createMyModuleAction = moduleMenu->addAction("Create My Module");
connect(createMyModuleAction, &QAction::triggered, this, [this]() {
    m_moduleManager->createMyCustomModule();
});
```

### 示例：文本编辑器模块

完整代码见上面的步骤说明。

### 模块间通信

使用 Qt 信号槽机制：

```cpp
class DataModule : public ModuleBase {
    Q_OBJECT
signals:
    void dataChanged(const QString& data);
};

class DisplayModule : public ModuleBase {
    Q_OBJECT
public slots:
    void onDataChanged(const QString& data) {
        // 更新显示
    }
};

// 在 MainWindow 或 ModuleManager 中连接
connect(dataModule, &DataModule::dataChanged,
        displayModule, &DisplayModule::onDataChanged);
```

## 🔧 技术细节

### 白板系统实现

- **动态槽位结构**
  ```cpp
  struct Slot {
      QWidget* widget;        // 槽位容器
      QLabel* placeholder;    // 占位符
      ModuleBase* module;     // 当前模块（nullptr表示空）
  };
  QList<Slot> m_slots;
  ```

- **自动扩展算法**
  - 创建模块时检查空槽位
  - 无空槽位则调用 `addNewSlot()`
  - 创建后调用 `ensureMinimumSlots()` 保持至少 MIN_SLOTS 个空槽位

- **拖拽检测**
  - `ModuleBase::mouseMoveEvent()` - 距离超过 50 像素触发 detach
  - `MainWindow::onModuleDragPositionChanged()` - 实时高亮目标槽位
  - `ModuleBase::mouseReleaseEvent()` - 触发 reattach 请求

### 内存管理

- 所有模块由 `ModuleManager` 管理
- Qt 父子关系自动处理内存释放
- 使用 `deleteLater()` 安全删除

### 信号流程

```
用户操作
  ↓
ModuleBase 事件 (mousePressEvent/mouseMove/mouseRelease)
  ↓
发射信号 (detachRequested/reattachRequested/closeRequested)
  ↓
MainWindow 接收并处理
  ↓
更新白板状态（添加/移除槽位，高亮等）
  ↓
调用 ModuleManager 管理生命周期
```

## 📖 API 参考

### ModuleBase 核心方法

| 方法 | 说明 |
|------|------|
| `ModuleType moduleType()` | 获取模块类型 |
| `QString moduleTitle()` | 获取模块标题 |
| `int moduleId()` | 获取唯一ID |
| `void clear()` | 清理模块状态（纯虚） |
| `QWidget* contentWidget()` | 获取内容widget（纯虚） |
| `void setDetachedState(bool)` | 设置独立/嵌入状态 |

### ModuleManager 核心方法

| 方法 | 说明 |
|------|------|
| `T* createModule<T>()` | 模板方法创建模块 |
| `void destroyModule(ModuleBase*)` | 销毁指定模块 |
| `QList<ModuleBase*> allModules()` | 获取所有模块 |
| `ModuleBase* moduleById(int)` | 根据ID查找模块 |
| `int totalModuleCount()` | 获取总数 |

### MainWindow 核心方法

| 方法 | 说明 |
|------|------|
| `int findEmptySlot()` | 查找第一个空槽位 |
| `void placeModuleInSlot(module, index)` | 将模块放入槽位 |
| `void removeModuleFromSlot(module)` | 从槽位移除模块 |
| `void addNewSlot()` | 添加新槽位 |
| `void ensureMinimumSlots()` | 确保最少空槽位数 |
| `void highlightSlot(index, highlight)` | 高亮/取消高亮槽位 |

## 🎓 最佳实践

### 1. 模块设计原则
- 保持模块独立性，最小化依赖
- 使用信号槽进行模块间通信
- 实现 `clear()` 方法以支持状态重置
- 设置合理的最小尺寸（推荐 300x400）

### 2. 性能优化
- 避免在模块中进行耗时操作
- 使用 `QTimer::singleShot` 延迟初始化
- 大量数据使用 `QAbstractItemModel`
- 考虑使用线程处理计算密集任务

### 3. UI 设计建议
- 使用一致的配色方案
- 提供清晰的操作反馈
- 支持键盘快捷键
- 实现撤销/重做功能

## 🐛 故障排除

### Qt6 找不到

**macOS**:
```bash
# 安装 Qt6
brew install qt@6

# 设置环境变量
export Qt6_DIR="/opt/homebrew/opt/qt@6"
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6:$CMAKE_PREFIX_PATH"
```

**Windows**:
- 下载并安装 Qt6 从 https://www.qt.io/download
- 在 CMake 命令中指定路径：
  ```cmd
  cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
  ```

### 编译警告

类型转换警告（`qsizetype` to `int`）是正常的，不影响功能。可以通过类型转换消除：
```cpp
int count = static_cast<int>(m_slots.size());
```

### 应用崩溃

1. 检查模块的 `clear()` 和析构函数实现
2. 确保信号槽连接正确
3. 查看控制台日志输出
4. 使用 Qt Creator 或 Xcode 调试器

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

### 开发流程
1. Fork 项目
2. 创建特性分支: `git checkout -b feature/AmazingFeature`
3. 提交更改: `git commit -m 'Add some AmazingFeature'`
4. 推送到分支: `git push origin feature/AmazingFeature`
5. 提交 Pull Request

## 📄 许可证

MIT License - 详见 LICENSE 文件

## 🙏 致谢

- Qt 框架提供强大的跨平台支持
- 设计灵感来自 commentaryLibrary 项目

## 📞 联系方式

有问题或建议？请通过以下方式联系：
- 提交 GitHub Issue
- 发送邮件到项目维护者

---

**Happy Coding! 🚀**