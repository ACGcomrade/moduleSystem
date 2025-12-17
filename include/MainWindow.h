#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include "modules/ModuleManager.h"
#include "modules/CalculatorModule.h"

/**
 * @brief 主窗口类
 *
 * 提供模块化UI的主窗口，包含：
 * - 菜单栏用于创建模块
 * - 3个槽位用于放置模块
 * - 支持模块的拖拽和重新排列
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    void setupSlots();
    bool tryPlaceModuleInSlot(ModuleBase* module);
    void removeModuleFromSlot(ModuleBase* module);
    void updateSlotPlaceholders();

    // UI组件
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    QWidget* m_slots[3];  // 3个槽位
    QLabel* m_placeholders[3];  // 占位符标签
    ModuleBase* m_slotModules[3];  // 槽位中的模块

    // 模块管理
    ModuleManager* m_moduleManager;

    // 独立窗口列表
    QList<ModuleBase*> m_detachedModules;
};

#endif // MAINWINDOW_H