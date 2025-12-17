#include "modules/ModuleBase.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QMoveEvent>
#include <QCursor>

int ModuleBase::s_nextId = 1;

ModuleBase::ModuleBase(ModuleType type, const QString& title, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_title(title)
    , m_id(s_nextId++)
    , m_isDetached(false)
    , m_dragging(false)
    , m_lastPos(-1, -1)
{
    setMinimumSize(300, 400);
    setAttribute(Qt::WA_DeleteOnClose, false);  // 不自动删除

    // 创建移动停止检测定时器
    m_moveTimer = new QTimer(this);
    m_moveTimer->setSingleShot(true);
    m_moveTimer->setInterval(200);  // 200ms没有移动就认为停止了
    connect(m_moveTimer, &QTimer::timeout, this, &ModuleBase::onMoveTimeout);
}

ModuleBase::~ModuleBase() {
    qDebug() << "[Module" << m_id << "] Destroyed:" << m_title;
}

void ModuleBase::setDetached(bool detached, const QPoint& globalMousePos) {
    m_isDetached = detached;
    if (detached) {
        setWindowFlags(Qt::Window);
        setWindowTitle(m_title);
        // 设置窗口初始位置到鼠标下方，减去拖拽偏移
        if (!globalMousePos.isNull()) {
            move(globalMousePos - m_dragStartPos);
        }
        show();
        qDebug() << "[Module" << m_id << "] Detached as independent window at pos:" << pos();
        // 保持拖拽不中断
        m_dragging = true;
    } else {
        setWindowFlags(Qt::Widget);
        qDebug() << "[Module" << m_id << "] Reattached to main window";
    }
}

void ModuleBase::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击在标题栏区域（顶部30像素）
        if (event->pos().y() < 30) {
            m_dragging = true;
            m_dragStartPos = event->pos();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void ModuleBase::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        if (!m_isDetached && (event->globalPosition().toPoint() - m_dragStartPos).manhattanLength() > 50) {
            // 触发detach时，传递当前鼠标全局坐标
            setDetached(true, event->globalPosition().toPoint());
            emit detachRequested(this);
            // 拖拽不中断，直接进入下一个分支
        }
        if (m_isDetached) {
            QPoint newPos = event->globalPosition().toPoint() - m_dragStartPos;
            move(newPos);
            emit dragPositionChanged(this, event->globalPosition().toPoint());
        }
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void ModuleBase::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 在detached状态下，任何鼠标释放都应该尝试reattach
        // 不管是拖拽内容区域还是系统标题栏
        if (m_isDetached) {
            // 发送释放位置信号，由MainWindow检查是否在槽位内
            QPoint releasePos = event->globalPosition().toPoint();
            qDebug() << "[Module" << m_id << "] Released at global position:" << releasePos;
            emit reattachRequested(this);
        }
        m_dragging = false;
        // 清除拖动预览
        emit dragPositionChanged(this, QPoint(-1, -1));
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

    // 只在detached状态下发送位置信号
    if (m_isDetached) {
        QPoint currentPos = pos();

        // 检查位置是否真的改变了（避免重复信号）
        if (currentPos != m_lastPos) {
            m_lastPos = currentPos;

            // 获取鼠标全局位置
            QPoint mouseGlobalPos = QCursor::pos();

            // 发送拖动位置信号，用于检测槽位
            emit dragPositionChanged(this, mouseGlobalPos);

            // 重启定时器，用于检测移动停止
            m_moveTimer->start();
        }
    }
}

void ModuleBase::onMoveTimeout() {
    // 窗口停止移动后，检查是否应该reattach
    if (m_isDetached) {
        QPoint mousePos = QCursor::pos();
        qDebug() << "[Module" << m_id << "] Movement stopped at mouse position:" << mousePos;

        // 清除拖动预览
        emit dragPositionChanged(this, QPoint(-1, -1));

        // 发送reattach请求，由MainWindow检查是否在槽位内
        emit reattachRequested(this);
    }
}