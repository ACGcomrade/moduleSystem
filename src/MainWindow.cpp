#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_moduleManager(new ModuleManager(this))
{
    setWindowTitle("Module System Template");
    setMinimumSize(800, 600);

    setupUI();
    setupMenuBar();

    // 连接模块管理器信号
    connect(m_moduleManager, &ModuleManager::moduleCreated,
            this, &MainWindow::onModuleCreated);
    connect(m_moduleManager, &ModuleManager::moduleDestroyed,
            this, &MainWindow::onModuleDestroyed);

    qDebug() << "[MainWindow] Initialized";
}

MainWindow::~MainWindow() {
    qDebug() << "[MainWindow] Destroyed";
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    setupSlots();
}

void MainWindow::setupSlots() {
    for (int i = 0; i < 3; ++i) {
        m_slots[i] = new QWidget();
        m_slots[i]->setMinimumSize(250, 400);
        m_slots[i]->setStyleSheet("border: 2px dashed #aaa; border-radius: 5px;");

        QVBoxLayout* slotLayout = new QVBoxLayout(m_slots[i]);
        slotLayout->setAlignment(Qt::AlignCenter);

        m_placeholders[i] = new QLabel(QString("Slot %1\nDrop module here").arg(i + 1));
        m_placeholders[i]->setAlignment(Qt::AlignCenter);
        m_placeholders[i]->setStyleSheet("color: #666; font-size: 14px;");
        slotLayout->addWidget(m_placeholders[i]);

        m_slotModules[i] = nullptr;
        m_mainLayout->addWidget(m_slots[i]);
    }
}

void MainWindow::setupMenuBar() {
    // 创建菜单栏
    QMenuBar* menuBar = this->menuBar();

    // 模块菜单
    QMenu* moduleMenu = menuBar->addMenu("Modules");

    QAction* createExampleAction = moduleMenu->addAction("Create Example Module");
    connect(createExampleAction, &QAction::triggered, this, &MainWindow::onCreateExampleModule);

    QAction* createCustomAction = moduleMenu->addAction("Create Custom Module");
    connect(createCustomAction, &QAction::triggered, this, &MainWindow::onCreateCustomModule);

    QAction* createCalculatorAction = moduleMenu->addAction("Create Calculator Module");
    connect(createCalculatorAction, &QAction::triggered, this, &MainWindow::onCreateCalculatorModule);

    moduleMenu->addSeparator();

    QAction* exitAction = moduleMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::onCreateExampleModule() {
    ExampleModule* module = m_moduleManager->createExampleModule();
    if (!module) {
        QMessageBox::information(this, "Limit Reached",
            "Maximum number of Example modules reached.");
        return;
    }
    // 模块会通过信号自动处理
}

void MainWindow::onCreateCustomModule() {
    CustomModuleTemplate* module = m_moduleManager->createCustomModule();
    if (!module) {
        QMessageBox::information(this, "Limit Reached",
            "Maximum number of Custom modules reached.");
        return;
    }
    // 模块会通过信号自动处理
}

void MainWindow::onCreateCalculatorModule() {
    CalculatorModule* module = m_moduleManager->createCalculatorModule();
    if (!module) {
        QMessageBox::information(this, "Limit Reached",
            "Maximum number of Calculator modules reached.");
        return;
    }
    // 模块会通过信号自动处理
}

void MainWindow::onModuleCreated(ModuleBase* module) {
    qDebug() << "[MainWindow] Module created:" << module->moduleTitle();

    // 连接模块信号
    connect(module, &ModuleBase::detachRequested, this, &MainWindow::onModuleDetachRequested);
    connect(module, &ModuleBase::reattachRequested, this, &MainWindow::onModuleReattachRequested);
    connect(module, &ModuleBase::closeRequested, this, &MainWindow::onModuleCloseRequested);
    connect(module, &ModuleBase::dragPositionChanged, this, &MainWindow::onModuleDragPositionChanged);

    // 尝试放入空槽位
    if (!tryPlaceModuleInSlot(module)) {
        // 如果没有空槽位，设置为独立窗口
        module->setDetached(true);
        m_detachedModules.append(module);
    }
}

void MainWindow::onModuleDestroyed(ModuleBase* module) {
    qDebug() << "[MainWindow] Module destroyed:" << module->moduleTitle();

    // 从槽位或独立窗口列表中移除
    removeModuleFromSlot(module);
    m_detachedModules.removeAll(module);
}

void MainWindow::onModuleDetachRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Detach requested for:" << module->moduleTitle();

    // 从槽位移除
    removeModuleFromSlot(module);

    // 设置为独立窗口
    module->setDetached(true);
    m_detachedModules.append(module);
}

void MainWindow::onModuleReattachRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Reattach requested for:" << module->moduleTitle();

    // 从独立窗口列表移除
    m_detachedModules.removeAll(module);

    // 尝试重新放入槽位
    if (!tryPlaceModuleInSlot(module)) {
        // 如果没有空槽位，保持独立状态
        module->setDetached(true);
        m_detachedModules.append(module);
    }
}

void MainWindow::onModuleCloseRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Close requested for:" << module->moduleTitle();

    // 确认关闭
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Close Module",
        QString("Are you sure you want to close the module '%1'?").arg(module->moduleTitle()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_moduleManager->destroyModule(module);
    }
}

void MainWindow::onModuleDragPositionChanged(ModuleBase* module, const QPoint& globalPos) {
    // 可以在这里添加拖拽预览效果
    Q_UNUSED(module)
    Q_UNUSED(globalPos)
}

bool MainWindow::tryPlaceModuleInSlot(ModuleBase* module) {
    for (int i = 0; i < 3; ++i) {
        if (!m_slotModules[i]) {
            // 找到空槽位
            m_slotModules[i] = module;
            module->setDetached(false);

            // 替换占位符
            QVBoxLayout* slotLayout = qobject_cast<QVBoxLayout*>(m_slots[i]->layout());
            if (slotLayout) {
                // 移除占位符
                slotLayout->removeWidget(m_placeholders[i]);
                m_placeholders[i]->hide();

                // 添加模块
                slotLayout->addWidget(module);
            }

            qDebug() << "[MainWindow] Module placed in slot" << i;
            return true;
        }
    }
    return false;
}

void MainWindow::removeModuleFromSlot(ModuleBase* module) {
    for (int i = 0; i < 3; ++i) {
        if (m_slotModules[i] == module) {
            m_slotModules[i] = nullptr;

            // 从布局中移除模块
            QVBoxLayout* slotLayout = qobject_cast<QVBoxLayout*>(m_slots[i]->layout());
            if (slotLayout) {
                slotLayout->removeWidget(module);
                module->setParent(nullptr);

                // 重新显示占位符
                slotLayout->addWidget(m_placeholders[i]);
                m_placeholders[i]->show();
            }

            qDebug() << "[MainWindow] Module removed from slot" << i;
            break;
        }
    }
}

void MainWindow::updateSlotPlaceholders() {
    // 更新占位符显示（如果需要）
}