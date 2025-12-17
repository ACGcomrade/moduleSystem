#include "PerformanceMonitor.h"
#include <QDebug>

#ifdef Q_OS_MACOS
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

PerformanceMonitor::PerformanceMonitor(QObject *parent)
    : QObject(parent)
    , m_cpuThreshold(80.0)
    , m_memoryThreshold(85.0)
    , m_processMemoryThreshold(1024) // 1GB
    , m_lastCPUTime(0)
    , m_lastSystemTime(0)
{
    // 初始化定时器，每2秒更新一次性能数据
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &PerformanceMonitor::updateMetrics);
    m_updateTimer->start(2000);

    // 立即更新一次
    updateMetrics();

    qDebug() << "[PerformanceMonitor] Initialized with thresholds:"
             << "CPU:" << m_cpuThreshold << "%"
             << "Memory:" << m_memoryThreshold << "%"
             << "Process:" << m_processMemoryThreshold << "MB";
}

PerformanceMonitor::~PerformanceMonitor() {
    qDebug() << "[PerformanceMonitor] Destroyed";
}

void PerformanceMonitor::updateMetrics() {
    m_currentMetrics.cpuUsagePercent = getCPUUsage();
    m_currentMetrics.memoryUsedMB = getSystemMemoryUsed();
    m_currentMetrics.memoryTotalMB = getSystemMemoryTotal();
    m_currentMetrics.processMemoryMB = getProcessMemoryUsage();

    if (m_currentMetrics.memoryTotalMB > 0) {
        m_currentMetrics.memoryUsagePercent =
            (double)m_currentMetrics.memoryUsedMB / m_currentMetrics.memoryTotalMB * 100.0;
    } else {
        m_currentMetrics.memoryUsagePercent = 0.0;
    }

    // 检查是否超过警告阈值
    if (m_currentMetrics.cpuUsagePercent > m_cpuThreshold * 0.9) {
        emit performanceWarning(QString("CPU usage high: %1%").arg(m_currentMetrics.cpuUsagePercent, 0, 'f', 1));
    }
    if (m_currentMetrics.memoryUsagePercent > m_memoryThreshold * 0.9) {
        emit performanceWarning(QString("Memory usage high: %1%").arg(m_currentMetrics.memoryUsagePercent, 0, 'f', 1));
    }
}

PerformanceMonitor::PerformanceMetrics PerformanceMonitor::getCurrentMetrics() {
    return m_currentMetrics;
}

bool PerformanceMonitor::canCreateNewModule(QString* reason) {
    PerformanceMetrics metrics = getCurrentMetrics();

    // 检查CPU使用率
    if (metrics.cpuUsagePercent > m_cpuThreshold) {
        if (reason) {
            *reason = QString("CPU使用率过高 (%1% > %2%)\n"
                            "当前系统负载较重，创建更多模块可能导致性能下降")
                        .arg(metrics.cpuUsagePercent, 0, 'f', 1)
                        .arg(m_cpuThreshold, 0, 'f', 1);
        }
        return false;
    }

    // 检查系统内存使用率
    if (metrics.memoryUsagePercent > m_memoryThreshold) {
        if (reason) {
            *reason = QString("系统内存使用率过高 (%1% > %2%)\n"
                            "可用内存: %3 MB / %4 MB\n"
                            "创建更多模块可能导致系统卡顿")
                        .arg(metrics.memoryUsagePercent, 0, 'f', 1)
                        .arg(m_memoryThreshold, 0, 'f', 1)
                        .arg(metrics.memoryTotalMB - metrics.memoryUsedMB)
                        .arg(metrics.memoryTotalMB);
        }
        return false;
    }

    // 检查进程内存使用
    if (metrics.processMemoryMB > m_processMemoryThreshold) {
        if (reason) {
            *reason = QString("应用程序内存使用过多 (%1 MB > %2 MB)\n"
                            "建议关闭一些模块后再创建新模块")
                        .arg(metrics.processMemoryMB)
                        .arg(m_processMemoryThreshold);
        }
        return false;
    }

    return true;
}

double PerformanceMonitor::getCPUUsage() {
#ifdef Q_OS_MACOS
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
        quint64 totalTicks = 0;
        for (int i = 0; i < CPU_STATE_MAX; i++) {
            totalTicks += cpuinfo.cpu_ticks[i];
        }
        quint64 idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];

        if (m_lastSystemTime > 0) {
            quint64 totalTicksSinceLastTime = totalTicks - m_lastSystemTime;
            quint64 idleTicksSinceLastTime = idleTicks - m_lastCPUTime;

            if (totalTicksSinceLastTime > 0) {
                double usage = 100.0 * (1.0 - ((double)idleTicksSinceLastTime / totalTicksSinceLastTime));
                m_lastCPUTime = idleTicks;
                m_lastSystemTime = totalTicks;
                return qMax(0.0, qMin(100.0, usage));
            }
        }

        m_lastCPUTime = idleTicks;
        m_lastSystemTime = totalTicks;
    }
#endif

#ifdef Q_OS_WIN
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        auto FileTimeToUInt64 = [](const FILETIME& ft) -> quint64 {
            return ((quint64)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        };

        quint64 idle = FileTimeToUInt64(idleTime);
        quint64 kernel = FileTimeToUInt64(kernelTime);
        quint64 user = FileTimeToUInt64(userTime);
        quint64 total = kernel + user;

        if (m_lastSystemTime > 0) {
            quint64 totalDiff = total - m_lastSystemTime;
            quint64 idleDiff = idle - m_lastCPUTime;

            if (totalDiff > 0) {
                double usage = 100.0 * (1.0 - ((double)idleDiff / totalDiff));
                m_lastCPUTime = idle;
                m_lastSystemTime = total;
                return qMax(0.0, qMin(100.0, usage));
            }
        }

        m_lastCPUTime = idle;
        m_lastSystemTime = total;
    }
#endif

    return 0.0; // 返回0表示无法获取
}

quint64 PerformanceMonitor::getSystemMemoryUsed() {
#ifdef Q_OS_MACOS
    vm_size_t page_size;
    mach_port_t mach_port = mach_host_self();
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);

    if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
        host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) == KERN_SUCCESS) {

        quint64 used = ((quint64)vm_stats.active_count +
                       (quint64)vm_stats.inactive_count +
                       (quint64)vm_stats.wire_count) * page_size;
        return used / (1024 * 1024); // 转换为MB
    }
#endif

#ifdef Q_OS_WIN
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024);
    }
#endif

    return 0;
}

quint64 PerformanceMonitor::getSystemMemoryTotal() {
#ifdef Q_OS_MACOS
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    int64_t physical_memory;
    size_t length = sizeof(physical_memory);

    if (sysctl(mib, 2, &physical_memory, &length, nullptr, 0) == 0) {
        return physical_memory / (1024 * 1024); // 转换为MB
    }
#endif

#ifdef Q_OS_WIN
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullTotalPhys / (1024 * 1024);
    }
#endif

    return 0;
}

quint64 PerformanceMonitor::getProcessMemoryUsage() {
#ifdef Q_OS_MACOS
    struct task_basic_info info;
    mach_msg_type_number_t size = sizeof(info);

    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size) == KERN_SUCCESS) {
        return info.resident_size / (1024 * 1024); // 转换为MB
    }
#endif

#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024 * 1024);
    }
#endif

    return 0;
}
