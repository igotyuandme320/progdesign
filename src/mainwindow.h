#pragma once
#include "boxmanager.h"
#include <QMainWindow>

/** 第一天的可操作页面原型；业务按钮暂不启用。 */
class MainWindow : public QMainWindow {
public:
    /** @brief 创建四个标签页及表单。 @param manager 业务接口骨架。 @return 无。 */
    explicit MainWindow(const BoxManager &manager);
};
