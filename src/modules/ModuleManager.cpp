#include "modules/ModuleManager.h"
#include <QDebug>

ModuleManager::ModuleManager(QObject *parent)
    : QObject(parent)
    , m_performanceMonitor(new PerformanceMonitor(this))
{
    qDebug() << "[ModuleManager] Initialized with performance monitoring";
}

ModuleManager::~ModuleManager() {
    destroyAllModules();
    qDebug() << "[ModuleManager] Destroyed";
}

ExampleModule* ModuleManager::createExampleModule(QString* performanceReason) {
    // 检查性能限制
    if (!m_performanceMonitor->canCreateNewModule(performanceReason)) {
        qWarning() << "[ModuleManager] Cannot create ExampleModule due to performance constraints";
        return nullptr;
    }

    ExampleModule* module = new ExampleModule();
    registerModule(module);
    return module;
}

CustomModuleTemplate* ModuleManager::createCustomModule(QString* performanceReason) {
    // 检查性能限制
    if (!m_performanceMonitor->canCreateNewModule(performanceReason)) {
        qWarning() << "[ModuleManager] Cannot create CustomModule due to performance constraints";
        return nullptr;
    }

    CustomModuleTemplate* module = new CustomModuleTemplate();
    registerModule(module);
    return module;
}

QList<ModuleBase*> ModuleManager::allModules() const {
    return m_allModules;
}

QList<ModuleBase*> ModuleManager::modulesByType(ModuleBase::ModuleType type) const {
    QList<ModuleBase*> result;
    for (ModuleBase* module : m_allModules) {
        if (module->moduleType() == type) {
            result.append(module);
        }
    }
    return result;
}

ModuleBase* ModuleManager::moduleById(int id) const {
    for (ModuleBase* module : m_allModules) {
        if (module->moduleId() == id) {
            return module;
        }
    }
    return nullptr;
}

void ModuleManager::destroyModule(ModuleBase* module) {
    if (!module) return;

    qDebug() << "[ModuleManager] Destroying module:" << module->moduleId();
    unregisterModule(module);
    cleanupModule(module);
    emit moduleDestroyed(module);
}

void ModuleManager::destroyAllModules() {
    // 创建副本以避免在迭代时修改列表
    QList<ModuleBase*> modulesToDestroy = m_allModules;
    for (ModuleBase* module : modulesToDestroy) {
        destroyModule(module);
    }
}

int ModuleManager::moduleCountByType(ModuleBase::ModuleType type) const {
    return m_typeCounts.value(type, 0);
}

bool ModuleManager::canCreateModule(ModuleBase::ModuleType type) const {
    // 无限制版本 - 总是允许创建模块
    Q_UNUSED(type);
    return true;
}

void ModuleManager::registerModule(ModuleBase* module) {
    if (!module) return;

    m_allModules.append(module);

    // 根据类型添加到特定列表
    switch (module->moduleType()) {
        case ModuleBase::Example:
            m_exampleModules.append(static_cast<ExampleModule*>(module));
            break;
        case ModuleBase::Custom:
            m_customModules.append(static_cast<CustomModuleTemplate*>(module));
            break;
        default:
            // 对于其他类型，不添加到特定列表
            break;
    }

    // 更新类型计数
    m_typeCounts[module->moduleType()]++;

    qDebug() << "[ModuleManager] Created module:" << module->moduleId()
             << "Type:" << module->moduleType()
             << "Title:" << module->moduleTitle();

    emit moduleCreated(module);
    emit moduleTypeCountChanged(module->moduleType(), m_typeCounts[module->moduleType()]);
}

void ModuleManager::unregisterModule(ModuleBase* module) {
    if (!module) return;

    m_allModules.removeAll(module);

    // 从类型特定列表中移除
    switch (module->moduleType()) {
        case ModuleBase::Example:
            m_exampleModules.removeAll(static_cast<ExampleModule*>(module));
            break;
        case ModuleBase::Custom:
            m_customModules.removeAll(static_cast<CustomModuleTemplate*>(module));
            break;
        default:
            break;
    }

    // 更新类型计数
    if (m_typeCounts.contains(module->moduleType())) {
        m_typeCounts[module->moduleType()]--;
        emit moduleTypeCountChanged(module->moduleType(), m_typeCounts[module->moduleType()]);
    }
}

void ModuleManager::cleanupModule(ModuleBase* module) {
    if (module) {
        module->clear();
        module->deleteLater();
    }
}