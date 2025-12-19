#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QMenuBar>
#include <QList>
#include <QRect>
#include <QPoint>
#include <QMouseEvent>
#include "modules/ModuleManager.h"

/**
 * @brief 无限大的可拖拽白板widget
 */
class DraggableBoardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DraggableBoardWidget(QWidget *parent = nullptr);

    // 添加模块到白板
    void addModuleWidget(QWidget* moduleWidget, const QPoint& pos);
    void removeModuleWidget(QWidget* moduleWidget);

signals:
    void boardMoved(const QPoint& delta);  // 白板移动时发出信号

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_dragging;
    QPoint m_dragStartPos;
    QList<QWidget*> m_moduleWidgets;  // 白板上的模块widget列表
};

/**
 * @brief 主窗口类（多模块吸附版）
 *
 * 提供模块化UI的主窗口，包含：
 * - 菜单栏用于创建模块
 * - 可拖拽的白板区域
 * - 多个模块可以吸附到白板
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 获取白板的全局矩形区域
    QRect getBoardGlobalRect() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private slots:
    // 模块创建
    void onCreateExampleModule();
    void onCreateCustomModule();

    // 模块事件处理
    void onModuleCreated(ModuleBase* module);
    void onModuleDestroyed(ModuleBase* module);
    void onModuleDetachRequested(ModuleBase* module);
    void onModuleReattachRequested(ModuleBase* module);
    void onModuleCloseRequested(ModuleBase* module);
    void onModuleDragPositionChanged(ModuleBase* module, const QPoint& globalPos);

    // 白板移动处理
    void onBoardMoved(const QPoint& delta);

    // 定时更新吸附模块位置
    void updateAttachedModulesPosition();

private:
    void setupUI();
    void setupMenuBar();
    void updateBoardGlobalRect();

    // 检查模块是否完全在白板内
    bool isModuleFullyInBoard(ModuleBase* module);

    // 卡槽结构
    struct Slot {
        QWidget* widget;       // 卡槽widget
        QRect localRect;       // 卡槽在白板中的本地坐标
        ModuleBase* module;    // 吸附的模块
        bool isOccupied;       // 是否被占用
    };

    // 创建临时卡槽
    Slot* createTemporarySlot(const QRect& moduleRect);
    void removeSlot(Slot* slot);

    // UI组件
    QWidget* m_centralWidget;
    DraggableBoardWidget* m_boardWidget;  // 白板区域（可拖拽）
    QLabel* m_boardLabel;    // 白板提示标签
    QLabel* m_notificationLabel;  // 左上角通知标签

    // 模块管理
    ModuleManager* m_moduleManager;

    // 所有模块列表（所有模块都是独立窗口）
    QList<ModuleBase*> m_allModules;

    // 卡槽列表
    QList<Slot*> m_slots;

    // 白板的全局矩形
    QRect m_boardGlobalRect;

    // 定时器用于更新吸附模块位置
    QTimer* m_updateTimer;

    // 白板的初始大小（可以无限扩展）
    static const int BOARD_WIDTH = 3000;
    static const int BOARD_HEIGHT = 2000;
};

#endif // MAINWINDOW_H
