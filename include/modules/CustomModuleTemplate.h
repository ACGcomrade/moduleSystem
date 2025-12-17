#ifndef CUSTOMMODULETEMPLATE_H
#define CUSTOMMODULETEMPLATE_H

#include "ModuleBase.h"

/**
 * @brief 自定义模块模板
 *
 * 这是一个模板类，用户可以复制并修改来创建自己的模块。
 *
 * 要创建自定义模块：
 * 1. 复制这个文件和对应的.cpp文件
 * 2. 重命名类名和文件名
 * 3. 实现 contentWidget() 方法来定义UI
 * 4. 实现 clear() 方法来清理状态
 * 5. 在 ModuleManager 中添加创建方法
 * 6. 在 MainWindow 中添加菜单项
 */
class CustomModuleTemplate : public ModuleBase {
    Q_OBJECT

public:
    explicit CustomModuleTemplate(QWidget *parent = nullptr);
    ~CustomModuleTemplate();

    void clear() override;
    QWidget* contentWidget() override;

    static ModuleType staticModuleType() { return Custom; }

private:
    QWidget* m_contentWidget;
};

#endif // CUSTOMMODULETEMPLATE_H