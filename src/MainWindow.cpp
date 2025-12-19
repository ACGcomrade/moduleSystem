#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QScrollArea>

// DraggableBoardWidget 实现
DraggableBoardWidget::DraggableBoardWidget(QWidget *parent)
    : QWidget(parent), m_dragging(false)
{
    setMouseTracking(true);
}

void DraggableBoardWidget::addModuleWidget(QWidget* moduleWidget, const QPoint& pos) {
    if (moduleWidget && !m_moduleWidgets.contains(moduleWidget)) {
        m_moduleWidgets.append(moduleWidget);
        qDebug() << "[DraggableBoardWidget] Added module widget at:" << pos;
    }
}

void DraggableBoardWidget::removeModuleWidget(QWidget* moduleWidget) {
    if (moduleWidget) {
        m_moduleWidgets.removeAll(moduleWidget);
        qDebug() << "[DraggableBoardWidget] Removed module widget";
    }
}

void DraggableBoardWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    // Optional: Could draw grid lines or other visual elements here
}

void DraggableBoardWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = event->pos();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void DraggableBoardWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        QPoint delta = event->pos() - m_dragStartPos;
        move(pos() + delta);
        emit boardMoved(delta);  // 发出白板移动信号
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void DraggableBoardWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

// MainWindow 实现
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_moduleManager(new ModuleManager(this))
{
    setupUI();
    setupMenuBar();

    // 连接模块管理器信号
    connect(m_moduleManager, &ModuleManager::moduleCreated,
            this, &MainWindow::onModuleCreated);
    connect(m_moduleManager, &ModuleManager::moduleDestroyed,
            this, &MainWindow::onModuleDestroyed);

    // 创建定时器以持续更新吸附模块位置
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);  // 约60 FPS
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateAttachedModulesPosition);
    m_updateTimer->start();

    qDebug() << "[MainWindow] Initialized with draggable board and update timer";
}

MainWindow::~MainWindow() {
    qDebug() << "[MainWindow] Destroyed";
}

void MainWindow::setupUI() {
    setWindowTitle("Module System - Multi-Module Board");
    resize(1400, 900);

    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    // 使用QScrollArea来容纳可拖拽的白板
    QVBoxLayout* centralLayout = new QVBoxLayout(m_centralWidget);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(false);  // 不自动调整大小
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #e0e0e0; }");

    // 创建可拖拽的白板
    m_boardWidget = new DraggableBoardWidget();
    m_boardWidget->setFixedSize(BOARD_WIDTH, BOARD_HEIGHT);
    m_boardWidget->setStyleSheet("background-color: #f8f8f8; border: 3px solid #888;");

    QVBoxLayout* boardLayout = new QVBoxLayout(m_boardWidget);
    boardLayout->setAlignment(Qt::AlignCenter);

    m_boardLabel = new QLabel("Draggable Board\n\nDrag modules here to attach (must be fully inside)\nYou can drag this board around within the window");
    m_boardLabel->setAlignment(Qt::AlignCenter);
    m_boardLabel->setStyleSheet("color: #666; font-size: 14px; border: none;");
    boardLayout->addWidget(m_boardLabel);

    scrollArea->setWidget(m_boardWidget);
    centralLayout->addWidget(scrollArea);

    // 创建左上角通知标签
    m_notificationLabel = new QLabel(this);
    m_notificationLabel->setStyleSheet(
        "background-color: rgba(33, 150, 243, 0.9); "
        "color: white; "
        "padding: 8px 12px; "
        "border-radius: 4px; "
        "font-size: 13px;"
    );
    m_notificationLabel->setAlignment(Qt::AlignCenter);
    m_notificationLabel->hide();  // 初始隐藏
    m_notificationLabel->raise();  // 确保在最上层

    // 初始化白板全局矩形
    QTimer::singleShot(100, this, [this]() {
        updateBoardGlobalRect();
        qDebug() << "[MainWindow] Board global rect:" << m_boardGlobalRect;

        // 设置通知标签位置（左上角）
        m_notificationLabel->move(10, 30);  // 留出菜单栏的空间
    });

    // 连接白板移动信号
    connect(m_boardWidget, &DraggableBoardWidget::boardMoved,
            this, &MainWindow::onBoardMoved);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();

    QMenu* moduleMenu = menuBar->addMenu("Modules");

    QAction* createExampleAction = moduleMenu->addAction("Create Example Module");
    connect(createExampleAction, &QAction::triggered, this, &MainWindow::onCreateExampleModule);

    QAction* createCustomAction = moduleMenu->addAction("Create Custom Module");
    connect(createCustomAction, &QAction::triggered, this, &MainWindow::onCreateCustomModule);

    moduleMenu->addSeparator();

    QAction* exitAction = moduleMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::onCreateExampleModule() {
    QString performanceReason;
    ExampleModule* module = m_moduleManager->createExampleModule(&performanceReason);

    if (!module) {
        QMessageBox::warning(this, "性能限制",
            QString("无法创建新模块\n\n%1").arg(performanceReason));
        return;
    }

    qDebug() << "[MainWindow] Example module created";
}

void MainWindow::onCreateCustomModule() {
    QString performanceReason;
    CustomModuleTemplate* module = m_moduleManager->createCustomModule(&performanceReason);

    if (!module) {
        QMessageBox::warning(this, "性能限制",
            QString("无法创建新模块\n\n%1").arg(performanceReason));
        return;
    }

    qDebug() << "[MainWindow] Custom module created";
}

void MainWindow::onModuleCreated(ModuleBase* module) {
    qDebug() << "[MainWindow] Module created:" << module->moduleTitle();

    // 连接模块信号
    connect(module, &ModuleBase::detachRequested, this, &MainWindow::onModuleDetachRequested);
    connect(module, &ModuleBase::reattachRequested, this, &MainWindow::onModuleReattachRequested);
    connect(module, &ModuleBase::closeRequested, this, &MainWindow::onModuleCloseRequested);
    connect(module, &ModuleBase::dragPositionChanged, this, &MainWindow::onModuleDragPositionChanged);

    // 添加到模块列表
    m_allModules.append(module);

    // 创建时不自动吸附，让用户手动拖拽
    module->show();

    // 设置初始位置在白板中心附近
    updateBoardGlobalRect();
    int offsetX = (m_allModules.size() - 1) * 30;
    int offsetY = (m_allModules.size() - 1) * 30;
    module->move(m_boardGlobalRect.x() + 50 + offsetX,
                 m_boardGlobalRect.y() + 50 + offsetY);

    qDebug() << "[MainWindow] Module positioned at:" << module->pos();
}

void MainWindow::onModuleDestroyed(ModuleBase* module) {
    qDebug() << "[MainWindow] Module destroyed:" << module->moduleTitle();
    m_allModules.removeAll(module);
}

void MainWindow::onModuleDetachRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Detach requested for:" << module->moduleTitle();

    // 找到并移除关联的卡槽
    for (auto it = m_slots.begin(); it != m_slots.end(); ) {
        Slot* slot = *it;
        if (slot->module == module) {
            removeSlot(slot);
            it = m_slots.erase(it);
        } else {
            ++it;
        }
    }

    module->detachFromSlot();

    // 隐藏通知
    m_notificationLabel->hide();
}

void MainWindow::onModuleReattachRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Reattach requested for:" << module->moduleTitle();

    updateBoardGlobalRect();

    // 检查模块是否完全在白板内
    if (isModuleFullyInBoard(module)) {
        // 获取模块当前的全局矩形
        QRect moduleGlobalRect = module->frameGeometry();

        // 创建临时卡槽在模块当前位置
        Slot* slot = createTemporarySlot(moduleGlobalRect);

        if (slot) {
            // 将模块吸附到卡槽
            slot->module = module;
            slot->isOccupied = true;

            // 计算卡槽的全局矩形
            QRect slotGlobalRect = QRect(
                m_boardWidget->mapToGlobal(slot->localRect.topLeft()),
                slot->localRect.size()
            );

            // 使用旧的attachToSlot逻辑
            module->attachToSlot(slotGlobalRect);

            // 显示吸附成功通知
            m_notificationLabel->setText("已吸附到白板");
            m_notificationLabel->setStyleSheet(
                "background-color: rgba(76, 175, 80, 0.9); "
                "color: white; "
                "padding: 8px 12px; "
                "border-radius: 4px; "
                "font-size: 13px;"
            );
            m_notificationLabel->adjustSize();
            m_notificationLabel->show();
            m_notificationLabel->raise();

            // 2秒后自动隐藏通知
            QTimer::singleShot(2000, this, [this]() {
                m_notificationLabel->hide();
            });

            qDebug() << "[MainWindow] Module attached to slot at:" << slotGlobalRect;
        }
    } else {
        m_notificationLabel->hide();
        qDebug() << "[MainWindow] Module not fully in board, cannot attach";
    }
}

void MainWindow::onModuleCloseRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Close requested for:" << module->moduleTitle();

    // 直接删除，不需要确认
    m_moduleManager->destroyModule(module);
}

void MainWindow::onModuleDragPositionChanged(ModuleBase* module, const QPoint& globalPos) {
    if (globalPos.x() < 0 || globalPos.y() < 0) {
        // 拖拽结束，隐藏通知
        m_notificationLabel->hide();
        return;
    }

    updateBoardGlobalRect();

    // 检查模块是否完全在白板内
    if (isModuleFullyInBoard(module)) {
        // 显示通知：可以吸附
        m_notificationLabel->setText("可以放入白板");
        m_notificationLabel->setStyleSheet(
            "background-color: rgba(33, 150, 243, 0.9); "
            "color: white; "
            "padding: 8px 12px; "
            "border-radius: 4px; "
            "font-size: 13px;"
        );
        m_notificationLabel->adjustSize();
        m_notificationLabel->show();
        m_notificationLabel->raise();
    } else {
        // 隐藏通知
        m_notificationLabel->hide();
    }
}

bool MainWindow::isModuleFullyInBoard(ModuleBase* module) {
    if (!module) return false;

    updateBoardGlobalRect();

    // 获取模块的框架矩形（包括标题栏）
    QRect moduleRect = module->frameGeometry();

    // 检查模块是否完全在白板内
    bool fullyInside = m_boardGlobalRect.contains(moduleRect);

    qDebug() << "[MainWindow] Module rect:" << moduleRect
             << "Board rect:" << m_boardGlobalRect
             << "Fully inside:" << fullyInside;

    return fullyInside;
}

QRect MainWindow::getBoardGlobalRect() const {
    return m_boardGlobalRect;
}

void MainWindow::updateBoardGlobalRect() {
    if (m_boardWidget) {
        m_boardGlobalRect = QRect(m_boardWidget->mapToGlobal(QPoint(0, 0)),
                                   m_boardWidget->size());
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateBoardGlobalRect();
}

void MainWindow::moveEvent(QMoveEvent *event) {
    QMainWindow::moveEvent(event);
    updateBoardGlobalRect();
}

void MainWindow::onBoardMoved(const QPoint& delta) {
    qDebug() << "[MainWindow] Board moved by delta:" << delta;

    // 更新白板的全局矩形
    updateBoardGlobalRect();

    // 更新所有卡槽中模块的位置
    for (Slot* slot : m_slots) {
        if (slot->isOccupied && slot->module) {
            // 计算新的卡槽全局位置
            QRect slotGlobalRect = QRect(
                m_boardWidget->mapToGlobal(slot->localRect.topLeft()),
                slot->localRect.size()
            );

            // 更新模块位置以匹配卡槽
            slot->module->move(slotGlobalRect.topLeft());

            qDebug() << "[MainWindow] Updated module" << slot->module->moduleId()
                     << "position to match slot:" << slotGlobalRect.topLeft();
        }
    }
}

MainWindow::Slot* MainWindow::createTemporarySlot(const QRect& moduleGlobalRect) {
    // 将全局坐标转换为白板本地坐标
    QPoint localTopLeft = m_boardWidget->mapFromGlobal(moduleGlobalRect.topLeft());

    // 创建卡槽widget
    QWidget* slotWidget = new QWidget(m_boardWidget);
    slotWidget->setGeometry(QRect(localTopLeft, moduleGlobalRect.size()));
    slotWidget->setStyleSheet(
        "background-color: rgba(33, 150, 243, 0.2); "
        "border: 2px dashed #2196F3;"
    );
    slotWidget->show();

    // 创建卡槽结构
    Slot* slot = new Slot();
    slot->widget = slotWidget;
    slot->localRect = QRect(localTopLeft, moduleGlobalRect.size());
    slot->module = nullptr;
    slot->isOccupied = false;

    m_slots.append(slot);

    qDebug() << "[MainWindow] Created slot at local pos:" << localTopLeft
             << "size:" << moduleGlobalRect.size();

    return slot;
}

void MainWindow::removeSlot(Slot* slot) {
    if (!slot) return;

    qDebug() << "[MainWindow] Removing slot";

    // 删除卡槽widget
    if (slot->widget) {
        slot->widget->deleteLater();
        slot->widget = nullptr;
    }

    delete slot;
}

void MainWindow::updateAttachedModulesPosition() {
    // 遍历所有卡槽，更新吸附模块的位置
    for (Slot* slot : m_slots) {
        if (slot->isOccupied && slot->module) {
            // 计算卡槽的当前全局位置
            QRect slotGlobalRect = QRect(
                m_boardWidget->mapToGlobal(slot->localRect.topLeft()),
                slot->localRect.size()
            );

            // 获取模块当前位置
            QPoint currentModulePos = slot->module->pos();
            QPoint targetPos = slotGlobalRect.topLeft();

            // 如果位置不匹配，更新模块位置
            if (currentModulePos != targetPos) {
                slot->module->move(targetPos);
                // 只在位置变化时打印日志，避免刷屏
                // qDebug() << "[MainWindow] Updated module" << slot->module->moduleId()
                //          << "to slot position:" << targetPos;
            }
        }
    }
}
