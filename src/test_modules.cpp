#include <iostream>
#include <QApplication>
#include <memory>
#include "modules/ModuleManager.h"
#include "modules/ExampleModule.h"
#include "modules/CustomModuleTemplate.h"
#include "modules/CalculatorModule.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);  // Qt需要QApplication

    std::cout << "Testing Module System..." << std::endl;

    // 创建模块管理器
    ModuleManager manager;

    // 创建示例模块
    ExampleModule* example = manager.createExampleModule();
    if (example) {
        std::cout << "Example module created successfully!" << std::endl;
        std::cout << "Module ID: " << example->moduleId() << std::endl;
        std::cout << "Module Title: " << example->moduleTitle().toStdString() << std::endl;

        // 清理模块
        manager.destroyModule(example);
        std::cout << "Example module destroyed" << std::endl;
    } else {
        std::cout << "Failed to create example module" << std::endl;
    }

    // 创建自定义模块
    CustomModuleTemplate* custom = manager.createCustomModule();
    if (custom) {
        std::cout << "Custom module created successfully!" << std::endl;
        std::cout << "Module ID: " << custom->moduleId() << std::endl;
        std::cout << "Module Title: " << custom->moduleTitle().toStdString() << std::endl;

        // 清理模块
        manager.destroyModule(custom);
        std::cout << "Custom module destroyed" << std::endl;
    } else {
        std::cout << "Failed to create custom module" << std::endl;
    }

    // 创建计算器模块
    CalculatorModule* calculator = manager.createCalculatorModule();
    if (calculator) {
        std::cout << "Calculator module created successfully!" << std::endl;
        std::cout << "Module ID: " << calculator->moduleId() << std::endl;
        std::cout << "Module Title: " << calculator->moduleTitle().toStdString() << std::endl;

        // 清理模块
        manager.destroyModule(calculator);
        std::cout << "Calculator module destroyed" << std::endl;
    } else {
        std::cout << "Failed to create calculator module" << std::endl;
    }
    return 0;  // 不运行app.exec()，直接退出
}