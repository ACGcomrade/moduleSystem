#include "modules/ModuleBase.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QMoveEvent>
#include <QCursor>
#include <QEvent>

int ModuleBase::s_nextId = 1;

ModuleBase::ModuleBase(ModuleType type, const QString& title, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_title(title)
    , m_id(s_nextId++)
    , m_isAttached(false)
    , m_dragging(false)
    , m_titleBarDragging(false)
    , m_lastPos(-1, -1)
    , m_lastMoveEventPos(-1, -1)
{
    // 新架构：始终创建为独立窗口
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    setWindowTitle(title);

    // 初始大小设置为典型槽位大小
    resize(300, 400);
    // 不设置最小/最大限制，让attachToSlot时设置
    setAttribute(Qt::WA_DeleteOnClose, false);

    // 安装事件过滤器以捕获关闭事件
    installEventFilter(this);

    // 创建移动停止检测定时器
    m_moveTimer = new QTimer(this);
    m_moveTimer->setSingleShot(true);
    m_moveTimer->setInterval(300);
    connect(m_moveTimer, &QTimer::timeout, this, &ModuleBase::onMoveTimeout);

    qDebug() << "[Module" << m_id << "] Created as independent window:" << m_title;
}

ModuleBase::~ModuleBase() {
    qDebug() << "[Module" << m_id << "] Destroyed:" << m_title;
}

// 新方法：附着到槽位
void ModuleBase::attachToSlot(const QRect& slotGlobalRect) {
    m_isAttached = true;
    m_attachedSlotRect = slotGlobalRect;

    // 先临时显示窗口以获取准确的框架尺寸
    show();

    // 强制处理事件，确保窗口框架已创建
    QApplication::processEvents();

    // 获取窗口框架（包括标题栏）的大小
    QRect frameRect = frameGeometry();
    QRect contentRect = geometry();

    // 计算标题栏和边框的额外高度和宽度
    int frameTopHeight = frameRect.y() - contentRect.y();  // 这应该是负数，取绝对值
    int frameBottomHeight = (frameRect.height() - contentRect.height()) - abs(frameTopHeight);
    int frameLeftWidth = contentRect.x() - frameRect.x();  // 这应该是负数，取绝对值
    int frameRightWidth = (frameRect.width() - contentRect.width()) - abs(frameLeftWidth);

    // 修正为绝对值
    frameTopHeight = abs(frameTopHeight);
    frameLeftWidth = abs(frameLeftWidth);

    qDebug() << "[Module" << m_id << "] Frame geometry:" << frameRect;
    qDebug() << "[Module" << m_id << "] Content geometry:" << contentRect;
    qDebug() << "[Module" << m_id << "] Frame margins - Top:" << frameTopHeight
             << "Bottom:" << frameBottomHeight << "Left:" << frameLeftWidth
             << "Right:" << frameRightWidth;

    // 计算内容区域的目标大小（槽位大小减去框架）
    QSize targetContentSize(
        slotGlobalRect.width() - frameLeftWidth - frameRightWidth,
        slotGlobalRect.height() - frameTopHeight - frameBottomHeight
    );

    // 窗口位置：槽位位置就是窗口框架的左上角位置
    QPoint targetPos = slotGlobalRect.topLeft();

    // 设置内容区域的大小限制
    setMaximumSize(targetContentSize);
    setMinimumSize(targetContentSize);

    // 先设置大小
    resize(targetContentSize);

    // 再移动到目标位置（move会移动框架的左上角）
    move(targetPos);

    raise();

    qDebug() << "[Module" << m_id << "] Attached to slot at:" << slotGlobalRect
             << "Target content size:" << targetContentSize << "Window position:" << targetPos;
}

// 新方法：从槽位分离
void ModuleBase::detachFromSlot() {
    m_isAttached = false;
    m_attachedSlotRect = QRect();

    // 恢复大小限制，允许自由调整
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    setMinimumSize(300, 400);

    qDebug() << "[Module" << m_id << "] Detached from slot";
}

// 新方法：移动到全局位置
void ModuleBase::moveToGlobalPos(const QPoint& globalPos) {
    move(globalPos);
}

// 事件处理：捕获标题栏拖动和关闭事件
bool ModuleBase::event(QEvent *event) {
    if (event->type() == QEvent::Close) {
        qDebug() << "[Module" << m_id << "] Close event detected";
        // 发送关闭请求信号
        emit closeRequested(this);
        // 接受事件但不立即关闭，让MainWindow处理
        event->accept();
        return true;
    }
    else if (event->type() == QEvent::NonClientAreaMouseButtonPress) {
        m_titleBarDragging = true;
        if (m_isAttached) {
            emit detachRequested(this);
        }
        qDebug() << "[Module" << m_id << "] System title bar drag started";
    }
    else if (event->type() == QEvent::NonClientAreaMouseButtonRelease) {
        if (m_titleBarDragging) {
            qDebug() << "[Module" << m_id << "] System title bar drag released";
            m_titleBarDragging = false;

            // 清除高亮
            emit dragPositionChanged(this, QPoint(-1, -1));

            // 启动定时器检查智能放回
            m_moveTimer->stop();
            m_moveTimer->start();
        }
    }

    return QWidget::event(event);
}

// 鼠标按下：开始内容区拖拽
void ModuleBase::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint localPos = event->pos();

        // 检查是否点击在标题栏区域（顶部30像素）
        if (localPos.y() < 30 && localPos.y() >= 0 && localPos.x() >= 0) {
            m_dragging = true;
            m_dragStartPos = localPos;

            // 如果当前附着，发送detach请求
            if (m_isAttached) {
                emit detachRequested(this);
            }

            qDebug() << "[Module" << m_id << "] Mouse pressed at:" << localPos;
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

// 鼠标移动：拖拽窗口
void ModuleBase::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        QPoint globalPos = event->globalPosition().toPoint();

        // 计算新的窗口位置：鼠标全局位置 - 点击偏移
        QPoint newPos = globalPos - m_dragStartPos;
        move(newPos);

        // 发送位置信号用于槽位高亮
        emit dragPositionChanged(this, globalPos);

        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

// 鼠标松开：检查是否应该附着
void ModuleBase::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_dragging) {
        qDebug() << "[Module" << m_id << "] Content drag released";

        // 发送reattach请求
        emit reattachRequested(this);
        emit dragPositionChanged(this, QPoint(-1, -1));

        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

// 双击标题栏：切换附着/自由状态
void ModuleBase::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->pos().y() < 30) {
        // 双击标题栏
        if (m_isAttached) {
            emit detachRequested(this);
        } else {
            emit reattachRequested(this);
        }
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

// 窗口移动事件：发送位置信号
void ModuleBase::moveEvent(QMoveEvent *event) {
    QWidget::moveEvent(event);

    // 如果用户正在拖拽（内容区或标题栏），发送位置信号
    if (m_dragging || m_titleBarDragging) {
        QPoint currentPos = pos();

        if (currentPos != m_lastMoveEventPos) {
            m_lastMoveEventPos = currentPos;
            QPoint mouseGlobalPos = QCursor::pos();
            emit dragPositionChanged(this, mouseGlobalPos);

            // 重启定时器
            m_moveTimer->stop();
            m_moveTimer->start();
        }
    }
}

// 移动停止定时器：检查智能放回
void ModuleBase::onMoveTimeout() {
    // 只有在用户已经松手的情况下才尝试智能放回
    if (!m_dragging && !m_titleBarDragging && !m_isAttached) {
        QPoint mouseGlobalPos = QCursor::pos();
        qDebug() << "[Module" << m_id << "] Move stopped, checking for reattach at:" << mouseGlobalPos;
        emit reattachRequested(this);
        emit dragPositionChanged(this, QPoint(-1, -1));
    }
}
