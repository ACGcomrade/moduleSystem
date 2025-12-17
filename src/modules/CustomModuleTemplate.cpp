#include "modules/CustomModuleTemplate.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QDebug>

CustomModuleTemplate::CustomModuleTemplate(QWidget *parent)
    : ModuleBase(ModuleType::Custom, "Custom Module", parent)
{
    qDebug() << "[CustomModuleTemplate" << moduleId() << "] Created";

    // 创建内容widget
    m_contentWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_contentWidget);

    // 标题
    QLabel* titleLabel = new QLabel("Custom Module Template");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    // 描述
    QLabel* descLabel = new QLabel("This is a template for creating custom modules.\n"
                                   "Modify this class to implement your own functionality.\n\n"
                                   "Instructions:\n"
                                   "1. Copy this file and rename it\n"
                                   "2. Change the class name\n"
                                   "3. Implement contentWidget() to define your UI\n"
                                   "4. Implement clear() to reset state\n"
                                   "5. Add creation method to ModuleManager\n"
                                   "6. Add menu item to MainWindow");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("margin-bottom: 15px; background-color: #f0f0f0; padding: 10px; border-radius: 5px;");
    layout->addWidget(descLabel);

    // 示例功能区域
    QGroupBox* exampleGroup = new QGroupBox("Example Functions");
    QVBoxLayout* exampleLayout = new QVBoxLayout(exampleGroup);

    // 示例按钮
    QPushButton* exampleButton = new QPushButton("Click Me!");
    connect(exampleButton, &QPushButton::clicked, [this]() {
        qDebug() << "[CustomModuleTemplate" << moduleId() << "] Example button clicked";
        // 在这里添加你的自定义功能
    });
    exampleLayout->addWidget(exampleButton);

    // 示例文本输入
    QTextEdit* exampleText = new QTextEdit();
    exampleText->setPlaceholderText("This is where you can add custom UI elements...");
    exampleText->setMaximumHeight(80);
    exampleLayout->addWidget(exampleText);

    layout->addWidget(exampleGroup);

    // 提示信息
    QLabel* tipLabel = new QLabel("💡 Tip: Modify the contentWidget() method to create your custom UI!");
    tipLabel->setStyleSheet("color: #666; font-style: italic; margin-top: 10px;");
    layout->addWidget(tipLabel);

    layout->addStretch(); // 添加伸缩空间

    // 设置布局
    setLayout(layout);
}

CustomModuleTemplate::~CustomModuleTemplate() {
    qDebug() << "[CustomModuleTemplate" << moduleId() << "] Destroyed";
}

void CustomModuleTemplate::clear() {
    qDebug() << "[CustomModuleTemplate" << moduleId() << "] Clearing content";
    // 在这里清理模块状态
    // 例如：清除文本、重置按钮状态、释放资源等

    // 示例：可以在这里重置UI状态
    // 如果你添加了成员变量来跟踪状态，在这里清理它们
}

QWidget* CustomModuleTemplate::contentWidget() {
    return m_contentWidget;
}