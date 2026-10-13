#include "mainwindow.h"
#include <QApplication>

/** @brief 运行第一天界面原型。 @param argc 参数数量。 @param argv 参数内容。 @return 退出码。 */
int main(int argc, char *argv[]) {
    QApplication application(argc, argv); ///< Qt 应用对象。
    BoxManager manager; ///< 第一天业务管理骨架。
    MainWindow window(manager); ///< 可切换标签页的原型窗口。
    window.show(); return application.exec();
}
