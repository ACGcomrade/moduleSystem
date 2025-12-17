# Module System Template

这是一个模块化UI系统模板，允许用户通过代码自定义模块的UI和功能。

## 功能特点

- **模块化架构**: 支持动态创建、销毁和重新排列模块
- **拖拽支持**: 模块可以拖出为独立窗口或重新嵌入
- **类型安全**: 强类型模块系统，支持不同类型的模块
- **可扩展**: 易于添加新的模块类型
- **用户友好**: 提供模板让用户快速创建自定义模块

## 架构概述

### 核心组件

1. **ModuleBase**: 所有模块的基类，提供通用功能
2. **ModuleManager**: 模块生命周期和绑定的管理器
3. **MainWindow**: 主窗口，管理模块的布局和交互
4. **CustomModuleTemplate**: 用户自定义模块的模板

### 模块类型

- **ExampleModule**: 示例模块，展示基本功能
- **CustomModuleTemplate**: 用户自定义模块的起点

## 如何使用

### 1. 构建项目

```bash
mkdir build
cd build
cmake ..
make
```

### 2. 运行

```bash
./bin/ModuleSystem
```

### 3. 创建自定义模块

要创建自定义模块，请：

1. 复制 `CustomModuleTemplate.h` 和 `CustomModuleTemplate.cpp`
2. 重命名文件和类名
3. 实现 `contentWidget()` 方法来定义UI
4. 实现 `clear()` 方法来清理状态
5. 在 `ModuleManager` 中添加创建方法
6. 在 `MainWindow` 中添加菜单项

#### 示例：创建文本编辑器模块

```cpp
// TextEditorModule.h
#ifndef TEXTEDITORMODULE_H
#define TEXTEDITORMODULE_H

#include "modules/ModuleBase.h"

class TextEditorModule : public ModuleBase {
    Q_OBJECT

public:
    explicit TextEditorModule(QWidget *parent = nullptr);
    ~TextEditorModule();

    void clear() override;
    QWidget* contentWidget() override;

private:
    QTextEdit* m_textEdit;
};

#endif // TEXTEDITORMODULE_H

// TextEditorModule.cpp
#include "TextEditorModule.h"
#include <QTextEdit>
#include <QVBoxLayout>

TextEditorModule::TextEditorModule(QWidget *parent)
    : ModuleBase(ModuleType::Custom, "Text Editor", parent)
{
    // 创建UI
    QWidget* content = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(content);

    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("在这里输入文本...");
    layout->addWidget(m_textEdit);

    setLayout(layout);
}

TextEditorModule::~TextEditorModule() {}

void TextEditorModule::clear() {
    m_textEdit->clear();
}

QWidget* TextEditorModule::contentWidget() {
    return this; // 返回自身作为内容widget
}
```

## 依赖

- Qt6 (Core, Widgets)
- CMake 3.16+

## 许可证

MIT License