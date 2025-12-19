#include "ResizableSlotWidget.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>

ResizableSlotWidget::ResizableSlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_resizable(true)
    , m_resizing(false)
    , m_dragging(false)
    , m_resizeEdge(None)
{
    setMouseTracking(true);
}

ResizableSlotWidget::~ResizableSlotWidget() {
}

ResizableSlotWidget::ResizeEdge ResizableSlotWidget::getResizeEdge(const QPoint& pos) {
    if (!m_resizable) {
        return None;
    }

    int edge = None;

    if (pos.x() <= RESIZE_MARGIN) {
        edge |= Left;
    } else if (pos.x() >= width() - RESIZE_MARGIN) {
        edge |= Right;
    }

    if (pos.y() <= RESIZE_MARGIN) {
        edge |= Top;
    } else if (pos.y() >= height() - RESIZE_MARGIN) {
        edge |= Bottom;
    }

    return static_cast<ResizeEdge>(edge);
}

void ResizableSlotWidget::updateCursor(const QPoint& pos) {
    if (!m_resizable) {
        setCursor(Qt::ArrowCursor);
        return;
    }

    ResizeEdge edge = getResizeEdge(pos);

    if (edge == (Top | Left) || edge == (Bottom | Right)) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (edge == (Top | Right) || edge == (Bottom | Left)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (edge & Left || edge & Right) {
        setCursor(Qt::SizeHorCursor);
    } else if (edge & Top || edge & Bottom) {
        setCursor(Qt::SizeVerCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void ResizableSlotWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_resizeEdge = getResizeEdge(event->pos());

        if (m_resizeEdge != None) {
            // 开始调整大小
            m_resizing = true;
            m_dragStartPos = event->pos();
            m_resizeStartSize = size();
            m_resizeStartGlobalPos = mapToGlobal(QPoint(0, 0));
            event->accept();
            return;
        } else {
            // 检查是否点击在中心区域（用于拖拽整个槽位）
            QRect centerRect = rect().adjusted(RESIZE_MARGIN, RESIZE_MARGIN,
                                              -RESIZE_MARGIN, -RESIZE_MARGIN);
            if (centerRect.contains(event->pos())) {
                m_dragging = true;
                m_dragStartPos = event->pos();
                emit dragStarted();
                event->accept();
                return;
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void ResizableSlotWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_resizing) {
        QPoint delta = event->pos() - m_dragStartPos;
        QSize newSize = m_resizeStartSize;

        // 根据调整的边缘计算新大小
        if (m_resizeEdge & Right) {
            newSize.setWidth(m_resizeStartSize.width() + delta.x());
        } else if (m_resizeEdge & Left) {
            newSize.setWidth(m_resizeStartSize.width() - delta.x());
        }

        if (m_resizeEdge & Bottom) {
            newSize.setHeight(m_resizeStartSize.height() + delta.y());
        } else if (m_resizeEdge & Top) {
            newSize.setHeight(m_resizeStartSize.height() - delta.y());
        }

        // 限制最小大小
        newSize = newSize.expandedTo(QSize(200, 300));

        emit sizeChangeRequested(newSize);
        event->accept();
        return;
    } else if (m_dragging) {
        QPoint globalPos = event->globalPosition().toPoint();
        emit dragMoved(globalPos);
        event->accept();
        return;
    } else {
        // 更新鼠标光标
        updateCursor(event->pos());
    }

    QWidget::mouseMoveEvent(event);
}

void ResizableSlotWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_resizing) {
            m_resizing = false;
            m_resizeEdge = None;
            event->accept();
            return;
        } else if (m_dragging) {
            m_dragging = false;
            emit dragEnded();
            event->accept();
            return;
        }
    }

    QWidget::mouseReleaseEvent(event);
}

void ResizableSlotWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    // 如果正在调整大小或拖拽，绘制高亮边框
    if (m_resizing || m_dragging) {
        QPainter painter(this);
        painter.setPen(QPen(QColor(100, 150, 255), 2));
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
    }
}

bool ResizableSlotWidget::event(QEvent *event) {
    // 处理鼠标进入/离开事件
    if (event->type() == QEvent::Enter) {
        // 鼠标进入widget时
    } else if (event->type() == QEvent::Leave) {
        if (!m_resizing && !m_dragging) {
            setCursor(Qt::ArrowCursor);
        }
    }

    return QWidget::event(event);
}
