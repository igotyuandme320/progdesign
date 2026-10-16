#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

int main(int argc, char *argv[]) {
    QApplication application(argc, argv); ///< Qt 应用对象。
    application.setOrganizationName("ProgDesign");
    application.setApplicationName("MagicBox");
    QCommandLineParser parser; ///< 命令行选项解析器。
    parser.addHelpOption();
    parser.addOption({"data-dir", "指定 JSON 数据目录。", "directory"});
    parser.process(application);
    const QString directory = parser.isSet("data-dir")
        ? QDir(parser.value("data-dir")).absolutePath()
        : QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation); ///< 默认或自定义的数据目录。
    BoxManager manager(directory); ///< 业务与数据对象。
    QString error; ///< 数据加载失败的原因。
    // 文件损坏时退出，不进入业务界面，避免覆盖原档案。
    if (!manager.load(error))
    {
        QMessageBox::critical(
            nullptr,
            "数据加载失败",
            error + "\n文件目录：" + directory
        );
        return 1;
    }
    MainWindow window(manager); ///< 登录与登记窗口。
    window.show();
    return application.exec();
}
