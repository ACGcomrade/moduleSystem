#include "modules/CalculatorModule.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <cmath>

CalculatorModule::CalculatorModule(QWidget *parent)
    : ModuleBase(ModuleType::Calculator, "Calculator", parent)
    , m_waitingForOperand(true)
{
    qDebug() << "[CalculatorModule" << moduleId() << "] Created";

    m_contentWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_contentWidget);

    // 显示屏
    m_display = new QLabel("0");
    m_display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_display->setStyleSheet(
        "QLabel {"
        "    background-color: #f0f0f0;"
        "    border: 1px solid #ccc;"
        "    padding: 5px;"
        "    font-size: 18px;"
        "    font-family: monospace;"
        "    min-height: 30px;"
        "}"
    );
    mainLayout->addWidget(m_display);

    // 按钮网格
    createButtons();

    setLayout(mainLayout);
}

CalculatorModule::~CalculatorModule() {
    qDebug() << "[CalculatorModule" << moduleId() << "] Destroyed";
}

void CalculatorModule::createButtons() {
    QGridLayout* buttonLayout = new QGridLayout();

    // 按钮定义
    const char* buttonTexts[5][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"0", ".", "=", "+"},
        {"C", "CE", "", ""}
    };

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 4; ++col) {
            const char* text = buttonTexts[row][col];
            if (strlen(text) == 0) continue;

            QPushButton* button = new QPushButton(QString::fromUtf8(text));
            button->setMinimumSize(40, 40);
            button->setStyleSheet(
                "QPushButton {"
                "    font-size: 14px;"
                "    padding: 5px;"
                "}"
                "QPushButton:hover {"
                "    background-color: #e0e0e0;"
                "}"
            );

            connect(button, &QPushButton::clicked, this, &CalculatorModule::onButtonClicked);
            buttonLayout->addWidget(button, row, col);
        }
    }

    // 添加到主布局
    m_contentWidget->layout()->addItem(buttonLayout);
}

void CalculatorModule::onButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString buttonText = button->text();

    if (buttonText == "C") {
        // 清除所有
        m_currentExpression.clear();
        m_display->setText("0");
        m_waitingForOperand = true;
    } else if (buttonText == "CE") {
        // 清除当前输入
        m_display->setText("0");
        m_waitingForOperand = true;
    } else if (buttonText == "=") {
        // 计算结果
        calculate();
    } else {
        // 数字或运算符
        if (m_waitingForOperand) {
            m_display->setText(buttonText);
            m_waitingForOperand = false;
        } else {
            m_display->setText(m_display->text() + buttonText);
        }
    }
}

void CalculatorModule::calculate() {
    QString expression = m_display->text();

    try {
        double result = compute(expression);
        m_display->setText(QString::number(result));
        m_waitingForOperand = true;
    } catch (...) {
        m_display->setText("Error");
        m_waitingForOperand = true;
    }
}

double CalculatorModule::compute(const QString& expression) {
    // 简单的计算器实现
    // 这里可以集成更复杂的表达式解析器
    // 为了演示，我们只实现基本的四则运算

    // 这是一个简化的实现，实际应用中应该使用更健壮的表达式解析器
    QString expr = expression;

    // 处理乘法和除法
    QRegExp mulDiv("(\\d+\\.?\\d*)\\s*([*/])\\s*(\\d+\\.?\\d*)");
    while (mulDiv.indexIn(expr) != -1) {
        double left = mulDiv.cap(1).toDouble();
        double right = mulDiv.cap(3).toDouble();
        double result;

        if (mulDiv.cap(2) == "*") {
            result = left * right;
        } else if (mulDiv.cap(2) == "/" && right != 0) {
            result = left / right;
        } else {
            throw std::runtime_error("Division by zero");
        }

        expr.replace(mulDiv.cap(0), QString::number(result));
    }

    // 处理加法和减法
    QRegExp addSub("(\\d+\\.?\\d*)\\s*([+-])\\s*(\\d+\\.?\\d*)");
    while (addSub.indexIn(expr) != -1) {
        double left = addSub.cap(1).toDouble();
        double right = addSub.cap(3).toDouble();
        double result;

        if (addSub.cap(2) == "+") {
            result = left + right;
        } else {
            result = left - right;
        }

        expr.replace(addSub.cap(0), QString::number(result));
    }

    return expr.toDouble();
}

void CalculatorModule::clear() {
    qDebug() << "[CalculatorModule" << moduleId() << "] Clearing content";
    m_currentExpression.clear();
    m_display->setText("0");
    m_waitingForOperand = true;
}

QWidget* CalculatorModule::contentWidget() {
    return m_contentWidget;
}