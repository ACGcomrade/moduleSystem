#ifndef RESIZABLESLOTWIDGET_H
#define RESIZABLESLOTWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

/**
 * @brief 可调整大小和拖拽的槽位widget
 */
class ResizableSlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResizableSlotWidget(QWidget *parent = nullptr);
    ~ResizableSlotWidget();

    void setResizable(bool resizable) { m_resizable = resizable; }
    bool isResizable() const { return m_resizable; }

signals:
    void sizeChangeRequested(const QSize& newSize);
    void dragStarted();
    void dragMoved(const QPoint& globalPos);
    void dragEnded();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private:
    enum ResizeEdge {
        None = 0,
        Left = 1,
        Right = 2,
        Top = 4,
        Bottom = 8
    };

    ResizeEdge getResizeEdge(const QPoint& pos);
    void updateCursor(const QPoint& pos);

    bool m_resizable;
    bool m_resizing;
    bool m_dragging;
    ResizeEdge m_resizeEdge;
    QPoint m_dragStartPos;
    QSize m_resizeStartSize;
    QPoint m_resizeStartGlobalPos;

    static const int RESIZE_MARGIN = 8;  // 边缘检测范围
};

#endif // RESIZABLESLOTWIDGET_H
