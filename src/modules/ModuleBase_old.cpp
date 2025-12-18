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
    , m_isDetached(false)
    , m_dragging(false)
    , m_titleBarDragging(false)
    , m_lastPos(-1, -1)
    , m_lastMoveEventPos(-1, -1)
{
    setMinimumSize(300, 400);
    setAttribute(Qt::WA_DeleteOnClose, false);  // 不自动删除

    // 创建移动停止检测定时器
    m_moveTimer = new QTimer(this);
    m_moveTimer->setSingleShot(true);
    m_moveTimer->setInterval(300);  // 300ms没有移动就认为停止了
    connect(m_moveTimer, &QTimer::timeout, this, &ModuleBase::onMoveTimeout);
}

ModuleBase::~ModuleBase() {
    qDebug() << "[Module" << m_id << "] Destroyed:" << m_title;
}

void ModuleBase::setDetached(bool detached, const QPoint& globalMousePos) {
    if (m_isDetached == detached) {
        return; // 避免重复设置导致闪烁
    }

    m_isDetached = detached;
    if (detached) {
        // 保存当前widget在全局坐标系中的位置
        QPoint oldGlobalPos = mapToGlobal(QPoint(0, 0));

        setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        setWindowTitle(m_title);

        // 计算窗口位置：让鼠标保持在widget内的相对位置
        if (!globalMousePos.isNull() && !m_dragStartPos.isNull()) {
            // windowPos = 鼠标全局位置 - 鼠标在widget内的偏移
            QPoint windowPos = globalMousePos - m_dragStartPos;
            move(windowPos);
            qDebug() << "[Module" << m_id << "] Detached at window pos:" << windowPos
                     << "mouse:" << globalMousePos
                     << "local drag offset:" << m_dragStartPos
                     << "old widget pos:" << oldGlobalPos;
        }

        show();
        raise();
        activateWindow();

        // 保持拖拽状态，让窗口继续跟随鼠标
        m_dragging = true;
    } else {
        // 重新嵌入时清除窗口标志
        setWindowFlags(Qt::Widget);
        m_dragging = false;
        qDebug() << "[Module" << m_id << "] Reattached to main window";
    }
}

bool ModuleBase::event(QEvent *event) {
    // 捕获窗口移动开始事件（标题栏拖动）
    if (event->type() == QEvent::NonClientAreaMouseButtonPress) {
        if (m_isDetached) {
            m_titleBarDragging = true;
            qDebug() << "[Module" << m_id << "] System title bar drag started";
        }
    }
    else if (event->type() == QEvent::NonClientAreaMouseButtonRelease) {
        if (m_titleBarDragging) {
            qDebug() << "[Module" << m_id << "] System title bar drag released";
            m_titleBarDragging = false;

            // 清除高亮
            emit dragPositionChanged(this, QPoint(-1, -1));

            // 启动定时器检查智能放回
            if (m_isDetached) {
                m_moveTimer->stop();
                m_moveTimer->start();
            }
        }
    }

    return QWidget::event(event);
}

void ModuleBase::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint localPos = event->pos();

        // 检查是否点击在标题栏区域（顶部30像素）
        if (localPos.y() < 30 && localPos.y() >= 0 && localPos.x() >= 0) {
            // 使用自定义拖拽
            m_dragging = true;

            // 关键修复：使用全局坐标系统来计算偏移
            // 保存：鼠标点击位置相对于窗口左上角的偏移
            QPoint globalMousePos = event->globalPosition().toPoint();
            QPoint widgetGlobalPos = mapToGlobal(QPoint(0, 0));
            m_dragStartPos = globalMousePos - widgetGlobalPos;

            // 如果计算出的偏移不合理（负数或过大），使用event->pos()作为备选
            if (m_dragStartPos.x() < 0 || m_dragStartPos.y() < 0 ||
                m_dragStartPos.x() > width() || m_dragStartPos.y() > height()) {
                qDebug() << "[Module" << m_id << "] WARNING: Invalid calculated offset, using event->pos()";
                // 在嵌入状态下，如果坐标计算有问题，使用event->pos()
                // 这会在detach时被正确重新计算
                if (!m_isDetached && localPos.x() >= 0 && localPos.y() >= 0) {
                    m_dragStartPos = localPos;
                }
            }

            qDebug() << "[Module" << m_id << "] Mouse pressed:"
                     << "globalMouse:" << globalMousePos
                     << "widgetGlobalPos:" << widgetGlobalPos
                     << "calculated offset:" << m_dragStartPos
                     << "event->pos():" << localPos
                     << "isDetached:" << m_isDetached;

            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void ModuleBase::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        QPoint globalPos = event->globalPosition().toPoint();

        // 如果还未分离，检查是否应该分离
        if (!m_isDetached) {
            // 计算从按下位置开始的拖拽距离
            QPoint globalDragStart = mapToGlobal(m_dragStartPos);
            QPoint mouseMoveVector = globalPos - globalDragStart;

            if (mouseMoveVector.manhattanLength() > 50) {
                qDebug() << "[Module" << m_id << "] Triggering detach, distance:" << mouseMoveVector.manhattanLength();
                qDebug() << "[Module" << m_id << "] globalPos:" << globalPos << "globalDragStart:" << globalDragStart;
                qDebug() << "[Module" << m_id << "] m_dragStartPos (local):" << m_dragStartPos;

                emit detachRequested(this);

                // setDetached会用globalPos - m_dragStartPos计算窗口位置
                setDetached(true, globalPos);

                // 关键：detach后，m_dragStartPos保持不变（仍然是局部坐标）
                // 这样后续的移动计算才能正确
                qDebug() << "[Module" << m_id << "] After detach, m_dragStartPos:" << m_dragStartPos;
            }
        } else {
            // 已分离状态，窗口跟随鼠标移动
            // m_dragStartPos是最初点击时在widget内的局部坐标
            QPoint newPos = globalPos - m_dragStartPos;

            qDebug() << "[Module" << m_id << "] Moving detached window:"
                     << "globalPos:" << globalPos
                     << "m_dragStartPos:" << m_dragStartPos
                     << "newPos:" << newPos;

            move(newPos);
            emit dragPositionChanged(this, globalPos);
        }

        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void ModuleBase::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_dragging) {
        if (m_isDetached) {
            // 独立窗口：立即尝试智能放回
            QPoint releasePos = event->globalPosition().toPoint();
            qDebug() << "[Module" << m_id << "] Content drag released at:" << releasePos;

            emit reattachRequested(this);
            emit dragPositionChanged(this, QPoint(-1, -1));
        }

        // 清除拖拽标志
        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void ModuleBase::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->pos().y() < 30) {
        // 双击标题栏切换独立/嵌入状态
        if (m_isDetached) {
            emit reattachRequested(this);
        } else {
            emit detachRequested(this);
        }
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void ModuleBase::moveEvent(QMoveEvent *event) {
    QWidget::moveEvent(event);

    // 在detached状态下处理位置变化
    if (m_isDetached) {
        QPoint currentPos = pos();

        // 检查位置是否真的改变了
        if (currentPos != m_lastMoveEventPos) {
            m_lastMoveEventPos = currentPos;

            // 如果是用户主动拖拽（内容区或标题栏），发送位置信号用于高亮
            if (m_dragging || m_titleBarDragging) {
                QPoint mouseGlobalPos = QCursor::pos();
                emit dragPositionChanged(this, mouseGlobalPos);

                // 重启定时器
                m_moveTimer->stop();
                m_moveTimer->start();
            }
        }
    }
}

void ModuleBase::onMoveTimeout() {
    // 只有在窗口已经分离，但用户已经松手的情况下才尝试智能放回
    if (m_isDetached && !m_dragging && !m_titleBarDragging) {
        QPoint mouseGlobalPos = QCursor::pos();
        qDebug() << "[Module" << m_id << "] Move stopped (user released), checking for reattach at:" << mouseGlobalPos;
        emit reattachRequested(this);
        // 清除拖动预览
        emit dragPositionChanged(this, QPoint(-1, -1));
    } else if (m_isDetached && (m_dragging || m_titleBarDragging)) {
        qDebug() << "[Module" << m_id << "] Move stopped but user still dragging, ignore timeout";
    }
}