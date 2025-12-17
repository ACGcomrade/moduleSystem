#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QString>

/**
 * @brief 性能监控类
 *
 * 监控系统的CPU和内存使用情况，用于智能限制模块创建
 */
class PerformanceMonitor : public QObject {
    Q_OBJECT

public:
    struct PerformanceMetrics {
        double cpuUsagePercent;      // CPU使用率 (0-100)
        quint64 memoryUsedMB;        // 已用内存 (MB)
        quint64 memoryTotalMB;       // 总内存 (MB)
        double memoryUsagePercent;   // 内存使用率 (0-100)
        quint64 processMemoryMB;     // 当前进程内存使用 (MB)
    };

    explicit PerformanceMonitor(QObject *parent = nullptr);
    ~PerformanceMonitor();

    // 获取当前性能指标
    PerformanceMetrics getCurrentMetrics();

    // 检查是否可以安全创建新模块
    bool canCreateNewModule(QString* reason = nullptr);

    // 性能阈值配置
    void setCPUThreshold(double percent) { m_cpuThreshold = percent; }
    void setMemoryThreshold(double percent) { m_memoryThreshold = percent; }
    void setProcessMemoryThreshold(quint64 mb) { m_processMemoryThreshold = mb; }

    double cpuThreshold() const { return m_cpuThreshold; }
    double memoryThreshold() const { return m_memoryThreshold; }
    quint64 processMemoryThreshold() const { return m_processMemoryThreshold; }

signals:
    void performanceWarning(const QString& message);
    void performanceCritical(const QString& message);

private:
    void updateMetrics();
    double getCPUUsage();
    quint64 getSystemMemoryUsed();
    quint64 getSystemMemoryTotal();
    quint64 getProcessMemoryUsage();

    QTimer* m_updateTimer;
    PerformanceMetrics m_currentMetrics;

    // 性能阈值
    double m_cpuThreshold;           // CPU阈值 (默认80%)
    double m_memoryThreshold;        // 系统内存阈值 (默认85%)
    quint64 m_processMemoryThreshold; // 进程内存阈值 (默认1GB)

    // CPU使用率计算相关
    quint64 m_lastCPUTime;
    quint64 m_lastSystemTime;
};

#endif // PERFORMANCEMONITOR_H
