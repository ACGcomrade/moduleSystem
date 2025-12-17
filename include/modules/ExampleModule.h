#ifndef EXAMPLEMODULE_H
#define EXAMPLEMODULE_H

#include "ModuleBase.h"

/**
 * @brief 示例模块
 *
 * 展示如何创建自定义模块的示例
 */
class ExampleModule : public ModuleBase {
    Q_OBJECT

public:
    explicit ExampleModule(QWidget *parent = nullptr);
    ~ExampleModule();

    void clear() override;
    QWidget* contentWidget() override;

    static ModuleType staticModuleType() { return Example; }

private:
    QWidget* m_contentWidget;
};

#endif // EXAMPLEMODULE_H