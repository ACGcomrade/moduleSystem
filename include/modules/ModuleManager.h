#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QObject>
#include <QList>
#include <memory>
#include "ModuleBase.h"
#include "ExampleModule.h"
#include "CustomModuleTemplate.h"
#include "../PerformanceMonitor.h"

/**
 * @brief 模块管理器
 *
 * 负责：
 * 1. 创建和销毁模块
 * 2. 管理模块数量限制
 * 3. 处理模块生命周期
 * 4. 提供模块查询功能
 * 5. 支持模块类型注册
 */
class ModuleManager : public QObject {
    Q_OBJECT

public:
    explicit ModuleManager(QObject *parent = nullptr);
    ~ModuleManager();

    // 模块创建（会检查性能限制）
    ExampleModule* createExampleModule(QString* performanceReason = nullptr);
    CustomModuleTemplate* createCustomModule(QString* performanceReason = nullptr);

    // 获取性能监控器
    PerformanceMonitor* performanceMonitor() { return m_performanceMonitor; }

    // 通用模块创建方法
    template<typename T>
    T* createModule() {
        // 检查是否达到限制
        if (!canCreateModule(T::staticModuleType())) {
            qWarning() << "[ModuleManager] Cannot create module: limit reached for type" << T::staticModuleType();
            return nullptr;
        }

        T* module = new T();
        registerModule(module);
        return module;
    }

    // 获取现有模块
    QList<ModuleBase*> allModules() const;
    QList<ModuleBase*> modulesByType(ModuleBase::ModuleType type) const;
    ModuleBase* moduleById(int id) const;

    // 模块销毁
    void destroyModule(ModuleBase* module);
    void destroyAllModules();

    // 数量查询
    int totalModuleCount() const { return m_allModules.size(); }
    int exampleModuleCount() const { return m_exampleModules.size(); }
    int customModuleCount() const { return m_customModules.size(); }
    int moduleCountByType(ModuleBase::ModuleType type) const;

signals:
    void moduleCreated(ModuleBase* module);
    void moduleDestroyed(ModuleBase* module);
    void moduleTypeCountChanged(ModuleBase::ModuleType type, int count);

private:
    bool canCreateModule(ModuleBase::ModuleType type) const;
    void registerModule(ModuleBase* module);
    void unregisterModule(ModuleBase* module);
    void cleanupModule(ModuleBase* module);

    // 模块存储
    QList<ModuleBase*> m_allModules;
    QList<ExampleModule*> m_exampleModules;
    QList<CustomModuleTemplate*> m_customModules;

    // 类型计数器
    QMap<ModuleBase::ModuleType, int> m_typeCounts;

    // 性能监控
    PerformanceMonitor* m_performanceMonitor;
};

#endif // MODULEMANAGER_H