#ifndef MODULEBASE_H
#define MODULEBASE_H

#include <QWidget>
#include <QString>
#include <QMouseEvent>
#include <QTimer>

/**
 * @brief 所有模块的基类
 *
 * 提供通用的模块功能：
 * - 拖拽独立窗口
 * - 标题栏和关闭按钮
 * - 模块标识和类型
 * - 统一的生命周期管理
 */
class ModuleBase : public QWidget {
    Q_OBJECT

public:
    enum ModuleType {
        // 基础模块类型
        Example,        // 示例模块，用于演示
        Custom,         // 通用自定义模块

        // 专用模块类型（可扩展）
        TextEditor,     // 文本编辑器模块
        Calculator,     // 计算器模块
        FileBrowser,    // 文件浏览器模块
        ImageViewer,    // 图片查看器模块
        DataVisualizer, // 数据可视化模块
        Settings,       // 设置模块

        // 用户自定义类型从这里开始
        UserDefined = 1000  // 用户自定义模块类型的起始值
    };

    explicit ModuleBase(ModuleType type, const QString& title, QWidget *parent = nullptr);
    virtual ~ModuleBase();

    ModuleType moduleType() const { return m_type; }
    QString moduleTitle() const { return m_title; }
    int moduleId() const { return m_id; }

    // 静态方法用于模板
    static ModuleType staticModuleType() { return Example; }  // 默认实现，子类应该重写

    // 纯虚函数，子类必须实现
    virtual void clear() = 0;
    virtual QWidget* contentWidget() = 0;

    // 新架构：窗口模式 vs 嵌入模式切换
    void attachToSlot(const QRect& slotGlobalRect);  // 旧方法，兼容性保留
    void attachToBoard();                             // 切换到嵌入模式（无窗口框架）
    void detachFromSlot();                            // 切换到窗口模式（有窗口框架）
    bool isAttached() const { return m_isAttached; }

    // 获取内容widget（用于嵌入到白板）
    QWidget* getContentWidget();

    // 移动到指定全局位置
    void moveToGlobalPos(const QPoint& globalPos);

signals:
    void detachRequested(ModuleBase* module);
    void closeRequested(ModuleBase* module);
    void reattachRequested(ModuleBase* module);
    void dragPositionChanged(ModuleBase* module, const QPoint& globalPos);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void onMoveTimeout();

private:
    ModuleType m_type;
    QString m_title;
    int m_id;
    bool m_isAttached;            // 是否附着到槽位（新架构）
    QRect m_attachedSlotRect;     // 附着的槽位全局矩形

    // 拖拽相关
    bool m_dragging;              // 用户拖拽标志
    bool m_titleBarDragging;      // 标题栏拖拽标志（Qt系统拖动）
    QPoint m_dragStartPos;        // 拖拽起始位置（相对widget）
    QPoint m_lastPos;             // 记录上一次位置，用于检测移动
    QTimer* m_moveTimer;          // 检测移动停止的定时器
    QPoint m_lastMoveEventPos;    // 记录moveEvent的上一次位置

    static int s_nextId;
};

#endif // MODULEBASE_H