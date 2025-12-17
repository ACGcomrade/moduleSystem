#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QObject>
#include <QList>
#include <memory>
#include "ModuleBase.h"
#include "ExampleModule.h"
#include "CustomModuleTemplate.h"
#include "CalculatorModule.h"

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

    // 模块创建（返回nullptr表示达到数量限制）
    ExampleModule* createExampleModule();
    CustomModuleTemplate* createCustomModule();
    CalculatorModule* createCalculatorModule();

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

    // 限制常量
    static const int MAX_EXAMPLE_MODULES = 5;
    static const int MAX_CUSTOM_MODULES = 10;
    static const int MAX_CALCULATOR_MODULES = 3;
    static const int MAX_TOTAL_MODULES = 50;

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
    QList<CalculatorModule*> m_calculatorModules;

    // 类型计数器
    QMap<ModuleBase::ModuleType, int> m_typeCounts;
};

#endif // MODULEMANAGER_H