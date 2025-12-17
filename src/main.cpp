#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Module System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Your Organization");

    // 设置样式
    app.setStyle(QStyleFactory::create("Fusion"));

    // 创建主窗口
    MainWindow window;
    window.show();
    window.raise();
    window.activateWindow();

    qDebug() << "Application started";

    return app.exec();
}