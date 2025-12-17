#include "modules/ExampleModule.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QDebug>

ExampleModule::ExampleModule(QWidget *parent)
    : ModuleBase(ModuleType::Example, "Example Module", parent)
{
    qDebug() << "[ExampleModule" << moduleId() << "] Created";

    // 创建内容widget
    m_contentWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_contentWidget);

    // 标题
    QLabel* titleLabel = new QLabel("Example Module");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    // 描述
    QLabel* descLabel = new QLabel("This is an example module demonstrating the module system.\n"
                                   "You can customize this module by modifying the contentWidget() method.");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("margin-bottom: 15px;");
    layout->addWidget(descLabel);

    // 功能区域
    QGroupBox* functionGroup = new QGroupBox("Functions");
    QVBoxLayout* functionLayout = new QVBoxLayout(functionGroup);

    // 按钮
    QPushButton* button1 = new QPushButton("Button 1");
    connect(button1, &QPushButton::clicked, [this]() {
        qDebug() << "[ExampleModule" << moduleId() << "] Button 1 clicked";
    });
    functionLayout->addWidget(button1);

    QPushButton* button2 = new QPushButton("Button 2");
    connect(button2, &QPushButton::clicked, [this]() {
        qDebug() << "[ExampleModule" << moduleId() << "] Button 2 clicked";
    });
    functionLayout->addWidget(button2);

    layout->addWidget(functionGroup);

    // 文本区域
    QGroupBox* textGroup = new QGroupBox("Text Area");
    QVBoxLayout* textLayout = new QVBoxLayout(textGroup);

    QTextEdit* textEdit = new QTextEdit();
    textEdit->setPlaceholderText("Enter some text here...");
    textEdit->setMaximumHeight(100);
    textLayout->addWidget(textEdit);

    layout->addWidget(textGroup);

    // 设置布局
    setLayout(layout);
}

ExampleModule::~ExampleModule() {
    qDebug() << "[ExampleModule" << moduleId() << "] Destroyed";
}

void ExampleModule::clear() {
    qDebug() << "[ExampleModule" << moduleId() << "] Clearing content";
    // 在这里清理模块状态
    // 例如：清除文本、重置按钮状态等
}

QWidget* ExampleModule::contentWidget() {
    return m_contentWidget;
}