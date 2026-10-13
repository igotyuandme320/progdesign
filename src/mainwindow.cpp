#include "mainwindow.h"
#include <QDateEdit>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

/** @brief 创建页面原型。 @param description 页面说明。 @param fields 输入项。 @param buttons 未接入的按钮。 @return 可滚动页面。 */
static QWidget *makePage(const QString &description, const QStringList &fields, const QStringList &buttons) {
    auto *page = new QWidget; ///< 页面内容容器。
    auto *layout = new QVBoxLayout(page); ///< 说明、表单和按钮布局。
    auto *descriptionLabel = new QLabel(description); ///< 当前页面用途。
    descriptionLabel->setWordWrap(true); layout->addWidget(descriptionLabel);
    auto *form = new QFormLayout; ///< 基础输入表单。
    for (const QString &name : fields) { // name：当前输入项的中文名称。
        auto *input = new QLineEdit; ///< 原型输入框，暂不保存内容。
        input->setPlaceholderText("请输入" + name); form->addRow(name, input);
    }
    layout->addLayout(form);
    auto *actions = new QHBoxLayout; ///< 功能按钮一行排列。
    for (const QString &name : buttons) { // name：待开发功能的按钮文字。
        auto *button = new QPushButton(name); ///< 尚未接入业务的按钮。
        button->setEnabled(false); actions->addWidget(button);
    }
    layout->addLayout(actions); layout->addStretch();
    auto *scroll = new QScrollArea; ///< 小窗口下允许滚动表单。
    scroll->setWidgetResizable(true); scroll->setWidget(page); return scroll;
}

MainWindow::MainWindow(const BoxManager &manager) {
    setWindowTitle("神气的盒子 · 第1天版本"); resize(1000, 760); setMinimumSize(800, 600);
    setStyleSheet("QWidget {font-size: 14px;} QMainWindow {background: #f4f7fb;} QLineEdit {padding: 6px;} QPushButton {padding: 8px;} QTabWidget::pane {background: white; border: 1px solid #cbd5e1;}");
    auto *root = new QWidget; ///< 主窗口容器。
    auto *layout = new QVBoxLayout(root); ///< 总体纵向布局。
    auto *title = new QLabel("神气的盒子 · 第1天：方案与界面骨架"); ///< 顶部标题。
    title->setStyleSheet("font-size: 22px; color: #1d4e89; font-weight: bold;"); layout->addWidget(title);
    auto *account = new QHBoxLayout; ///< 登录区域原型。
    auto *employee = new QLineEdit; ///< 工号输入框。
    auto *password = new QLineEdit; ///< 密码输入框。
    password->setEchoMode(QLineEdit::Password);
    auto *login = new QPushButton("登录（第2天接入）"); ///< 尚未接入验证的按钮。
    login->setEnabled(false); account->addWidget(new QLabel("工号")); account->addWidget(employee);
    account->addWidget(new QLabel("密码")); account->addWidget(password); account->addWidget(login); layout->addLayout(account);
    auto *flight = new QHBoxLayout; ///< 航班输入原型。
    auto *number = new QLineEdit(manager.data().number); ///< 航班号输入。
    auto *date = new QDateEdit(manager.data().date); ///< 航班日期输入。
    date->setDisplayFormat("yyyy-MM-dd"); date->setCalendarPopup(true);
    auto *origin = new QLineEdit; ///< 出发地输入。
    auto *destination = new QLineEdit; ///< 目的地输入。
    flight->addWidget(new QLabel("航班")); flight->addWidget(number); flight->addWidget(date);
    flight->addWidget(new QLabel("出发地")); flight->addWidget(origin); flight->addWidget(new QLabel("目的地")); flight->addWidget(destination); layout->addLayout(flight);
    auto *tabs = new QTabWidget; ///< 四个功能入口。
    tabs->setObjectName("tabs");
    tabs->addTab(makePage("登记页面原型：字段已确定，校验和文件保存将在第2天接入。",
        {"姓名", "证件类型", "证件号码", "登机牌", "座位", "手机机型", "手机号", "备用联系方式", "盒子编号"}, {"保存登记并发放"}), "登记发放");
    tabs->addTab(makePage("回收页面原型：第3天接入盒号查询、回收确认和遗失设备联系。",
        {"盒子编号或座位", "备用联系方式", "联系备注"}, {"查询", "确认已归还", "确认设备领取"}), "回收与遗失查询");
    tabs->addTab(makePage("评价页面原型：第4天接入三种评价及意见记录。",
        {"乘客姓名或盒号", "意见与建议"}, {"1 满意", "2 不满意", "0 不表态"}), "评价与意见");
    tabs->addTab(makePage("统计与抽奖页面原型：第4天接入满意度统计和手机号抽奖。",
        {"抽奖人数"}, {"查看满意度", "抽取中奖手机号"}), "满意度与抽奖");
    layout->addWidget(tabs, 1);
    auto *status = new QLabel("本版本已完成数据结构、业务接口和页面骨架；业务按钮未接入，不会写入文件。"); ///< 第一阶段进度说明。
    status->setWordWrap(true); status->setStyleSheet("color: #475569;"); layout->addWidget(status); setCentralWidget(root);
}
