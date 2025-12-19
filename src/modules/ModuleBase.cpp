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

// 新方法：附着到白板（简化版 - 直接使用白板坐标）
void ModuleBase::attachToSlot(const QRect& boardGlobalRect) {
    m_isAttached = true;
    m_attachedSlotRect = boardGlobalRect;

    // 显示窗口
    show();

    // 简单地将模块窗口移动到白板位置并调整为白板大小
    // 用户可以使用Qt原生的窗口控制来调整大小
    setGeometry(boardGlobalRect);

    // 移除所有大小限制，允许用户自由调整
    setMinimumSize(200, 150);  // 只设置一个合理的最小值
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    raise();

    qDebug() << "[Module" << m_id << "] Attached to board at:" << boardGlobalRect;
}

// 新方法：附着到白板（切换到无边框模式）
void ModuleBase::attachToBoard() {
    m_isAttached = true;

    // 切换到无边框窗口（模拟嵌入模式）
    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::FramelessWindowHint;  // 添加无边框标志
    flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);

    // 显示窗口
    show();
    raise();

    qDebug() << "[Module" << m_id << "] Attached to board (frameless mode)";
}

// 新方法：从白板分离（切换到正常窗口模式）
void ModuleBase::detachFromSlot() {
    m_isAttached = false;
    m_attachedSlotRect = QRect();

    // 恢复正常窗口模式（有标题栏）
    Qt::WindowFlags flags = Qt::Window | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setWindowTitle(m_title);

    // 显示窗口
    show();
    raise();

    qDebug() << "[Module" << m_id << "] Detached from board (window mode)";
}

// 获取内容widget
QWidget* ModuleBase::getContentWidget() {
    return contentWidget();
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
