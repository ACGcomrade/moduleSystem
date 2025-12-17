#ifndef CALCULATORMODULE_H
#define CALCULATORMODULE_H

#include "ModuleBase.h"

/**
 * @brief 计算器模块示例
 *
 * 展示如何创建功能性的自定义模块
 */
class CalculatorModule : public ModuleBase {
    Q_OBJECT

public:
    explicit CalculatorModule(QWidget *parent = nullptr);
    ~CalculatorModule();

    void clear() override;
    QWidget* contentWidget() override;

    static ModuleType staticModuleType() { return Calculator; }

private slots:
    void onButtonClicked();
    void calculate();

private:
    QWidget* m_contentWidget;
    QLabel* m_display;
    QString m_currentExpression;
    bool m_waitingForOperand;

    void createButtons();
    double compute(const QString& expression);
};

#endif // CALCULATORMODULE_H