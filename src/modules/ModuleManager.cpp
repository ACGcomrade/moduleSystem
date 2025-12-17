#include "modules/ModuleManager.h"
#include <QDebug>

ModuleManager::ModuleManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[ModuleManager] Initialized";
}

ModuleManager::~ModuleManager() {
    destroyAllModules();
    qDebug() << "[ModuleManager] Destroyed";
}

ExampleModule* ModuleManager::createExampleModule() {
    if (!canCreateModule(ModuleBase::Example)) {
        qWarning() << "[ModuleManager] Cannot create ExampleModule: limit reached";
        return nullptr;
    }

    ExampleModule* module = new ExampleModule();
    registerModule(module);
    return module;
}

CustomModuleTemplate* ModuleManager::createCustomModule() {
    if (!canCreateModule(ModuleBase::Custom)) {
        qWarning() << "[ModuleManager] Cannot create CustomModule: limit reached";
        return nullptr;
    }

    CustomModuleTemplate* module = new CustomModuleTemplate();
    registerModule(module);
    return module;
}

CalculatorModule* ModuleManager::createCalculatorModule() {
    if (!canCreateModule(ModuleBase::Calculator)) {
        qWarning() << "[ModuleManager] Cannot create CalculatorModule: limit reached";
        return nullptr;
    }

    CalculatorModule* module = new CalculatorModule();
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
    // 检查总模块数限制
    if (m_allModules.size() >= MAX_TOTAL_MODULES) {
        return false;
    }

    // 检查类型特定限制
    switch (type) {
        case ModuleBase::Example:
            return m_exampleModules.size() < MAX_EXAMPLE_MODULES;
        case ModuleBase::Custom:
            return m_customModules.size() < MAX_CUSTOM_MODULES;
        case ModuleBase::Calculator:
            return m_calculatorModules.size() < MAX_CALCULATOR_MODULES;
        default:
            // 对于其他类型，默认允许创建，但可以在这里添加更多限制
            return true;
    }
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
        case ModuleBase::Calculator:
            m_calculatorModules.append(static_cast<CalculatorModule*>(module));
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
        case ModuleBase::Calculator:
            m_calculatorModules.removeAll(static_cast<CalculatorModule*>(module));
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