#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMessageBox>
#include <QCursor>
#include <QTimer>
#include <QResizeEvent>
#include <QMoveEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_moduleManager(new ModuleManager(this))
{
    setWindowTitle("Module System - Unlimited");
    setMinimumSize(900, 650);

    setupUI();
    setupMenuBar();

    // 连接模块管理器信号
    connect(m_moduleManager, &ModuleManager::moduleCreated,
            this, &MainWindow::onModuleCreated);
    connect(m_moduleManager, &ModuleManager::moduleDestroyed,
            this, &MainWindow::onModuleDestroyed);

    // 安装事件过滤器，监听scrollArea的滚动事件
    m_scrollArea->installEventFilter(this);

    qDebug() << "[MainWindow] Initialized with dynamic board system";
}

MainWindow::~MainWindow() {
    qDebug() << "[MainWindow] Destroyed";
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    QVBoxLayout* centralLayout = new QVBoxLayout(m_centralWidget);
    centralLayout->setSpacing(10);
    centralLayout->setContentsMargins(10, 10, 10, 10);

    // 标题标签
    QLabel* titleLabel = new QLabel("Module System - Drag modules out or create new ones");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; padding: 5px;");
    centralLayout->addWidget(titleLabel);

    // 白板滚动区域
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f5f5f5; }");

    m_boardWidget = new QWidget();
    m_boardLayout = new QHBoxLayout(m_boardWidget);
    m_boardLayout->setSpacing(10);
    m_boardLayout->setContentsMargins(10, 10, 10, 10);
    m_boardLayout->setAlignment(Qt::AlignLeft);

    m_scrollArea->setWidget(m_boardWidget);
    centralLayout->addWidget(m_scrollArea);

    // 初始化白板槽位
    setupBoard();

    // 状态栏
    QLabel* statsLabel = new QLabel("Unlimited modules | Use menu to create modules");
    statsLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px;");
    centralLayout->addWidget(statsLabel);
}

void MainWindow::setupBoard() {
    // 创建初始的MIN_SLOTS个空槽位
    for (int i = 0; i < MIN_SLOTS; ++i) {
        addNewSlot();
    }
    qDebug() << "[MainWindow] Board initialized with" << MIN_SLOTS << "slots";
}

void MainWindow::addNewSlot() {
    Slot slot;

    slot.widget = new QWidget();
    slot.widget->setMinimumSize(SLOT_MIN_WIDTH, SLOT_MIN_HEIGHT);
    slot.widget->setMaximumWidth(SLOT_MIN_WIDTH);
    slot.widget->setStyleSheet("border: 2px dashed #bbb; border-radius: 8px; background-color: white;");

    QVBoxLayout* slotLayout = new QVBoxLayout(slot.widget);
    slotLayout->setAlignment(Qt::AlignCenter);
    slotLayout->setContentsMargins(10, 10, 10, 10);

    slot.placeholder = new QLabel(QString("Slot %1\n\nDrag module here\nor create new").arg(m_slots.size() + 1));
    slot.placeholder->setAlignment(Qt::AlignCenter);
    slot.placeholder->setStyleSheet("color: #999; font-size: 13px; border: none;");
    slotLayout->addWidget(slot.placeholder);

    slot.module = nullptr;

    m_slots.append(slot);
    m_boardLayout->addWidget(slot.widget);

    qDebug() << "[MainWindow] Added new slot, total slots:" << m_slots.size();
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

    moduleMenu->addSeparator();

    QAction* exitAction = moduleMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::onCreateExampleModule() {
    QString performanceReason;
    ExampleModule* module = m_moduleManager->createExampleModule(&performanceReason);

    if (!module) {
        // 性能限制，显示警告
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
        // 性能限制，显示警告
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

    // 新架构：更新槽位几何信息
    updateSlotGeometries();

    // 查找空槽位
    int emptySlot = findEmptySlot();
    if (emptySlot >= 0) {
        // 附着到空槽位
        placeModuleInSlot(module, emptySlot);
    } else {
        // 没有空槽位，创建新槽位
        addNewSlot();
        updateSlotGeometries();  // 更新几何信息
        placeModuleInSlot(module, m_slots.size() - 1);
    }

    // 确保至少保持MIN_SLOTS个空槽位
    ensureMinimumSlots();
}

void MainWindow::onModuleDestroyed(ModuleBase* module) {
    qDebug() << "[MainWindow] Module destroyed:" << module->moduleTitle();

    // 新架构：从槽位和模块列表中移除
    removeModuleFromSlot(module);
    m_allModules.removeAll(module);
}

void MainWindow::onModuleDetachRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Detach requested for:" << module->moduleTitle();

    // 新架构：只需从槽位移除（模块已经是独立窗口）
    removeModuleFromSlot(module);

    // 模块现在处于自由状态，可以自由移动
}

void MainWindow::onModuleReattachRequested(ModuleBase* module) {
    qDebug() << "[MainWindow] Reattach requested for:" << module->moduleTitle();

    // 获取当前鼠标位置
    QPoint mouseGlobalPos = QCursor::pos();

    // 先尝试查找鼠标下方的具体槽位
    int targetSlot = findSlotAtPosition(mouseGlobalPos);

    qDebug() << "[MainWindow] Mouse global pos:" << mouseGlobalPos;
    qDebug() << "[MainWindow] Target slot:" << targetSlot;

    if (targetSlot >= 0) {
        // 找到了具体的可见槽位
        if (!m_slots[targetSlot].module) {
            // 槽位为空，附着到此槽位
            qDebug() << "[MainWindow] Attaching module to slot:" << targetSlot;
            placeModuleInSlot(module, targetSlot);
        } else {
            // 槽位已占用，查找其他可见的空槽位
            int emptySlot = findEmptyVisibleSlot();
            if (emptySlot >= 0) {
                qDebug() << "[MainWindow] Target slot occupied, using empty visible slot:" << emptySlot;
                placeModuleInSlot(module, emptySlot);
            } else {
                // 没有可见的空槽位，保持自由状态
                qDebug() << "[MainWindow] No visible empty slots, module stays free";
                // 不需要做任何事，模块保持当前位置
            }
        }
    } else {
        // 没有找到可见槽位，检查是否在白板区域附近
        QPoint scrollAreaLocalPos = m_scrollArea->mapFromGlobal(mouseGlobalPos);
        QRect scrollAreaRect = m_scrollArea->rect();

        // 扩大白板区域检测范围
        scrollAreaRect = scrollAreaRect.adjusted(-50, -50, 50, 50);

        if (scrollAreaRect.contains(scrollAreaLocalPos)) {
            // 在白板附近，找个可见的空槽位附着
            int emptySlot = findEmptyVisibleSlot();
            if (emptySlot >= 0) {
                qDebug() << "[MainWindow] Near board area, attaching to empty visible slot:" << emptySlot;
                placeModuleInSlot(module, emptySlot);
            } else {
                // 没有可见的空槽位，保持自由状态
                qDebug() << "[MainWindow] Near board but no visible slots, module stays free";
            }
        } else {
            // 远离白板区域，保持自由状态
            qDebug() << "[MainWindow] Module stays free (far from board)";
        }
    }

    // 清除拖拽高亮
    for (int i = 0; i < m_slots.size(); ++i) {
        highlightSlot(i, false);
    }

    // 确保足够的空槽位
    ensureMinimumSlots();
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
    // 查找拖拽位置下的槽位并高亮
    int targetSlot = findSlotAtPosition(globalPos);

    // 清除所有槽位的高亮
    for (int i = 0; i < m_slots.size(); ++i) {
        highlightSlot(i, false);
    }

    // 高亮目标槽位
    if (targetSlot >= 0 && !m_slots[targetSlot].module) {
        highlightSlot(targetSlot, true);
    }

    Q_UNUSED(module);
}

int MainWindow::findEmptySlot() {
    for (int i = 0; i < m_slots.size(); ++i) {
        if (!m_slots[i].module) {
            return i;
        }
    }
    return -1;  // 没有空槽位
}

int MainWindow::findEmptyVisibleSlot() {
    // 首先尝试找可见的空槽位
    for (int i = 0; i < m_slots.size(); ++i) {
        if (!m_slots[i].module && isSlotVisible(i)) {
            qDebug() << "[MainWindow] Found empty visible slot:" << i;
            return i;
        }
    }
    qDebug() << "[MainWindow] No empty visible slot found";
    return -1;  // 没有可见的空槽位
}

void MainWindow::placeModuleInSlot(ModuleBase* module, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= m_slots.size()) {
        qWarning() << "[MainWindow] Invalid slot index:" << slotIndex;
        return;
    }

    Slot& slot = m_slots[slotIndex];

    // 如果槽位已有模块，先移除
    if (slot.module) {
        removeModuleFromSlot(slot.module);
    }

    // 新架构：模块仍是独立窗口，只是附着到槽位
    slot.module = module;

    // 更新槽位全局矩形
    updateSlotGeometries();

    // 让模块附着到这个槽位
    module->attachToSlot(slot.globalRect);

    // 更新占位符文字
    slot.placeholder->setText("已占用");
    slot.placeholder->setStyleSheet("color: #999; font-style: italic;");

    qDebug() << "[MainWindow] Module" << module->moduleTitle() << "attached to slot" << slotIndex
             << "at" << slot.globalRect;
}

void MainWindow::removeModuleFromSlot(ModuleBase* module) {
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].module == module) {
            Slot& slot = m_slots[i];
            slot.module = nullptr;

            // 新架构：模块分离
            module->detachFromSlot();

            // 恢复占位符文字
            slot.placeholder->setText("Empty Slot");
            slot.placeholder->setStyleSheet("color: #999; font-style: italic;");

            qDebug() << "[MainWindow] Module removed from slot" << i;
            break;
        }
    }
}

int MainWindow::findSlotAtPosition(const QPoint& globalPos) {
    // 将全局坐标转换为白板坐标
    QPoint boardLocalPos = m_boardWidget->mapFromGlobal(globalPos);

    // 检查每个槽位
    for (int i = 0; i < m_slots.size(); ++i) {
        QWidget* slotWidget = m_slots[i].widget;

        // 获取槽位在白板中的几何区域
        QRect slotRect = slotWidget->geometry();

        // 扩大检测区域，使其更容易被检测到
        // 这样即使模块大小改变，也能更容易放回
        slotRect = slotRect.adjusted(-20, -20, 20, 20);

        // 检查鼠标是否在此槽位内，并且槽位必须在可视区域内
        if (slotRect.contains(boardLocalPos) && isSlotVisible(i)) {
            qDebug() << "[MainWindow] Slot" << i << "detected at pos:" << boardLocalPos
                     << "slot rect:" << slotRect << "(visible)";
            return i;
        }
    }

    qDebug() << "[MainWindow] No visible slot found at pos:" << boardLocalPos;
    return -1;
}

bool MainWindow::isSlotVisible(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= m_slots.size()) {
        return false;
    }

    QWidget* slotWidget = m_slots[slotIndex].widget;

    // 获取槽位在白板中的几何区域
    QRect slotRect = slotWidget->geometry();

    // 将槽位的矩形转换到滚动区域的viewport坐标系
    QRect viewportRect = m_scrollArea->viewport()->rect();

    // 计算槽位在viewport中的可见区域
    // 需要考虑滚动偏移
    int scrollX = m_scrollArea->horizontalScrollBar()->value();
    QRect slotInViewport = slotRect.translated(-scrollX, 0);

    // 计算交集，判断是否有可见部分
    QRect visiblePart = slotInViewport.intersected(viewportRect);

    // 如果可见部分的宽度至少是槽位宽度的30%，认为是可见的
    bool visible = visiblePart.width() >= slotRect.width() * 0.3;

    if (!visible) {
        qDebug() << "[MainWindow] Slot" << slotIndex << "is not visible"
                 << "slotRect:" << slotRect
                 << "scrollX:" << scrollX
                 << "viewportRect:" << viewportRect
                 << "visiblePart:" << visiblePart;
    }

    return visible;
}

void MainWindow::highlightSlot(int slotIndex, bool highlight) {
    if (slotIndex < 0 || slotIndex >= m_slots.size()) {
        return;
    }

    Slot& slot = m_slots[slotIndex];
    if (highlight) {
        slot.widget->setStyleSheet(
            "border: 3px solid #4CAF50; "
            "border-radius: 8px; "
            "background-color: #E8F5E9;"
        );
    } else {
        slot.widget->setStyleSheet(
            "border: 2px dashed #bbb; "
            "border-radius: 8px; "
            "background-color: white;"
        );
    }
}

void MainWindow::ensureMinimumSlots() {
    // 计算当前空槽位数量
    int emptySlots = 0;
    for (const Slot& slot : m_slots) {
        if (!slot.module) {
            emptySlots++;
        }
    }

    // 如果空槽位不足MIN_SLOTS，添加新槽位
    while (emptySlots < MIN_SLOTS) {
        addNewSlot();
        emptySlots++;
    }

    qDebug() << "[MainWindow] Ensured" << emptySlots << "empty slots available";
}
// 新架构：更新槽位的全局几何信息
void MainWindow::updateSlotGeometries() {
    for (int i = 0; i < m_slots.size(); ++i) {
        Slot& slot = m_slots[i];

        // 获取槽位widget在屏幕上的全局矩形
        QPoint globalPos = slot.widget->mapToGlobal(QPoint(0, 0));
        QSize size = slot.widget->size();
        slot.globalRect = QRect(globalPos, size);
    }
}

// 更新所有附着模块的位置和大小
void MainWindow::updateAttachedModules() {
    for (int i = 0; i < m_slots.size(); ++i) {
        Slot& slot = m_slots[i];
        if (slot.module && slot.module->isAttached()) {
            // 获取窗口框架信息
            QRect frameRect = slot.module->frameGeometry();
            QRect contentRect = slot.module->geometry();

            // 计算框架的额外尺寸
            int frameTopHeight = frameRect.y() - contentRect.y();
            int frameBottomHeight = (frameRect.height() - contentRect.height()) - abs(frameTopHeight);
            int frameLeftWidth = contentRect.x() - frameRect.x();
            int frameRightWidth = (frameRect.width() - contentRect.width()) - abs(frameLeftWidth);

            // 修正为绝对值
            frameTopHeight = abs(frameTopHeight);
            frameLeftWidth = abs(frameLeftWidth);

            // 计算内容区域的目标大小（槽位大小减去框架）
            QSize targetContentSize(
                slot.globalRect.width() - frameLeftWidth - frameRightWidth,
                slot.globalRect.height() - frameTopHeight - frameBottomHeight
            );

            // 窗口位置：槽位位置就是窗口框架的左上角位置
            QPoint targetPos = slot.globalRect.topLeft();

            // 先更新大小限制，允许模块跟随槽位变化
            slot.module->setMaximumSize(targetContentSize);
            slot.module->setMinimumSize(targetContentSize);

            // 先设置大小
            slot.module->resize(targetContentSize);

            // 再移动到目标位置
            slot.module->move(targetPos);
        }
    }
}

// 监听白板窗口的resize事件
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    // 立即更新槽位和模块（不延迟，实时响应）
    updateSlotGeometries();
    updateAttachedModules();
}

// 监听白板窗口的move事件
void MainWindow::moveEvent(QMoveEvent *event) {
    QMainWindow::moveEvent(event);

    // 立即更新槽位和模块位置（不延迟，实时跟随）
    updateSlotGeometries();
    updateAttachedModules();
}

// 事件过滤器：监听scrollArea的滚动事件
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_scrollArea) {
        if (event->type() == QEvent::Resize) {
            // scrollArea大小改变时立即更新
            updateSlotGeometries();
            updateAttachedModules();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
