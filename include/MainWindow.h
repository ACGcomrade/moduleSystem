#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QList>
#include "modules/ModuleManager.h"

/**
 * @brief 主窗口类（改进版）
 *
 * 提供模块化UI的主窗口，包含：
 * - 菜单栏用于创建模块
 * - 动态槽位用于放置模块（自动扩展）
 * - 支持模块的拖拽和重新排列
 * - 支持水平滚动以容纳更多模块
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

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

private:
    void setupUI();
    void setupMenuBar();
    void setupBoard();
    int findEmptySlot();
    int findEmptyVisibleSlot();  // 查找可见的空槽位
    void placeModuleInSlot(ModuleBase* module, int slotIndex);
    void removeModuleFromSlot(ModuleBase* module);
    int findSlotAtPosition(const QPoint& globalPos);
    bool isSlotVisible(int slotIndex);  // 检查槽位是否在可视区域内
    void highlightSlot(int slotIndex, bool highlight);
    void ensureMinimumSlots();
    void addNewSlot();

    // UI组件
    QWidget* m_centralWidget;
    QScrollArea* m_scrollArea;
    QWidget* m_boardWidget;
    QHBoxLayout* m_boardLayout;

    // 动态槽位管理（新架构：槽位只是占位符，不包含实际widget）
    struct Slot {
        QWidget* widget;        // 槽位占位符widget
        QLabel* placeholder;    // 显示"空槽位"或"已占用"的标签
        ModuleBase* module;     // 吸附到此槽位的模块（模块仍是独立窗口）
        QRect globalRect;       // 槽位在屏幕上的全局矩形
    };
    QList<Slot> m_slots;  // 动态槽位列表
    static const int MIN_SLOTS = 3;  // 最少保持3个空槽位
    static const int SLOT_MIN_WIDTH = 300;
    static const int SLOT_MIN_HEIGHT = 400;

    // 模块管理
    ModuleManager* m_moduleManager;

    // 所有模块列表（所有模块都是独立窗口）
    QList<ModuleBase*> m_allModules;

    // 更新槽位的全局位置
    void updateSlotGeometries();

    // 更新所有附着模块的位置和大小
    void updateAttachedModules();
};

#endif // MAINWINDOW_H