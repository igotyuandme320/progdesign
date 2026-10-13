#include "mainwindow.h"
#include <QComboBox>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDir>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

/** @brief 创建表单输入框。 @param form 所属表单。 @param label 中文字段名。 @param objectName 控件标识。 @return 输入框。 */
static QLineEdit *addField(QFormLayout *form, const QString &label, const QString &objectName) {
    auto *input = new QLineEdit; ///< 新建的输入框。
    input->setObjectName(objectName); input->setMaxLength(200); input->setPlaceholderText("请输入" + label);
    form->addRow(label, input); return input;
}

/** @brief 保留后续日期的页面原型。 @param description 进度说明。 @param fields 输入项。 @param buttons 待开发按钮。 @return 原型页面。 */
static QWidget *makePrototype(const QString &description, const QStringList &fields, const QStringList &buttons) {
    auto *page = new QWidget; ///< 页面内容。
    auto *layout = new QVBoxLayout(page); ///< 页面布局。
    auto *label = new QLabel(description); ///< 当前开发进度。
    label->setWordWrap(true); layout->addWidget(label);
    auto *form = new QFormLayout; ///< 原型输入表单。
    for (const QString &name : fields) // name：当前输入项的名称。
        addField(form, name, {});
    layout->addLayout(form);
    auto *actions = new QHBoxLayout; ///< 待开发的按钮区域。
    for (const QString &name : buttons) { // name：当前按钮的文字。
        auto *button = new QPushButton(name); ///< 灰色的原型按钮。
        button->setEnabled(false); actions->addWidget(button);
    }
    layout->addLayout(actions); layout->addStretch();
    auto *scroll = new QScrollArea; ///< 小窗口下的滚动区域。
    scroll->setWidgetResizable(true); scroll->setWidget(page); return scroll;
}

MainWindow::MainWindow(BoxManager &manager) : manager(manager) {
    setWindowTitle("神气的盒子 · 第2天版本"); resize(1000, 820); setMinimumSize(800, 600);
    setStyleSheet("QWidget {font-size: 14px;} QMainWindow {background: #f4f7fb;} QLineEdit, QComboBox {padding: 5px;} QPushButton {padding: 7px;} QTabWidget::pane {background: white; border: 1px solid #cbd5e1;}");
    auto *root = new QWidget; ///< 主窗口内容。
    auto *layout = new QVBoxLayout(root); ///< 总体布局。
    auto *title = new QLabel("神气的盒子 · 第2天：登录与登记发放"); ///< 阶段标题。
    title->setStyleSheet("font-size: 22px; color: #1d4e89; font-weight: bold;"); layout->addWidget(title);
    auto *account = new QHBoxLayout; ///< 登录与换岗区域。
    employeeField = new QLineEdit; employeeField->setObjectName("employeeField"); employeeField->setPlaceholderText("1001 或 1002");
    passwordField = new QLineEdit; passwordField->setObjectName("passwordField"); passwordField->setEchoMode(QLineEdit::Password);
    auto *login = new QPushButton("登录 / 换岗"); ///< 验证账号的按钮。
    crewLabel = new QLabel;
    account->addWidget(new QLabel("工号")); account->addWidget(employeeField); account->addWidget(new QLabel("密码"));
    account->addWidget(passwordField); account->addWidget(login); account->addWidget(crewLabel); layout->addLayout(account);
    // 点击或密码框回车均进行验证；成功后清空密码并开启业务区域。
    connect(login, &QPushButton::clicked, this, [this] {
        if (!this->manager.login(employeeField->text(), passwordField->text())) {
            showResult("工号或密码错误，请重新输入。", {}); return;
        }
        passwordField->clear(); refresh(); showResult({}, "登录成功，当前工号：" + this->manager.currentCrew());
    });
    connect(passwordField, &QLineEdit::returnPressed, login, &QPushButton::click);
    flightSection = new QWidget;
    auto *flight = new QHBoxLayout(flightSection); ///< 航班填写区域。
    flight->setContentsMargins(0, 0, 0, 0);
    numberField = new QLineEdit; numberField->setObjectName("numberField"); numberField->setMaxLength(200);
    dateField = new QDateEdit; dateField->setDisplayFormat("yyyy-MM-dd"); dateField->setCalendarPopup(true);
    originField = new QLineEdit; originField->setObjectName("originField"); originField->setMaxLength(200);
    destinationField = new QLineEdit; destinationField->setObjectName("destinationField"); destinationField->setMaxLength(200);
    saveFlightButton = new QPushButton("保存航班"); saveFlightButton->setObjectName("saveFlightButton");
    flight->addWidget(new QLabel("航班")); flight->addWidget(numberField); flight->addWidget(dateField);
    flight->addWidget(new QLabel("出发地")); flight->addWidget(originField); flight->addWidget(new QLabel("目的地"));
    flight->addWidget(destinationField); flight->addWidget(saveFlightButton); layout->addWidget(flightSection);
    connect(saveFlightButton, &QPushButton::clicked, this, [this] {
        if (showResult(this->manager.setFlight(numberField->text(), dateField->date(), originField->text(), destinationField->text()), "航班已保存。")) refresh();
    });
    tabs = new QTabWidget; tabs->setObjectName("tabs");
    auto *issuePage = new QWidget; ///< 登记页内容。
    auto *issueLayout = new QVBoxLayout(issuePage); ///< 登记页纵向布局。
    auto *description = new QLabel("先登录并保存航班，再填写所有登记项。修改航班后请重新保存；已发放后航班锁定，记录自动保存。"); ///< 操作说明。
    description->setWordWrap(true); issueLayout->addWidget(description);
    auto *form = new QFormLayout; ///< 乘客登记表单。
    nameField = addField(form, "姓名", "nameField");
    documentType = new QComboBox; documentType->addItems({"身份证", "护照", "其他"}); form->addRow("证件类型", documentType);
    documentField = addField(form, "证件号码", "documentField"); boardingField = addField(form, "登机牌", "boardingField");
    seatField = addField(form, "座位", "seatField"); phoneModelField = addField(form, "手机机型", "phoneModelField");
    phoneField = addField(form, "手机号", "phoneField"); backupField = addField(form, "备用联系方式", "backupField");
    backupField->setPlaceholderText("必填，可填写电话、邮箱等"); boxField = addField(form, "盒子编号", "boxField");
    issueLayout->addLayout(form);
    issueButton = new QPushButton("保存登记并发放"); issueButton->setObjectName("issueButton"); issueLayout->addWidget(issueButton);
    connect(issueButton, &QPushButton::clicked, this, &MainWindow::issuePassenger);
    connect(numberField, &QLineEdit::textChanged, this, &MainWindow::updateIssueState);
    connect(originField, &QLineEdit::textChanged, this, &MainWindow::updateIssueState);
    connect(destinationField, &QLineEdit::textChanged, this, &MainWindow::updateIssueState);
    connect(dateField, &QDateEdit::dateChanged, this, &MainWindow::updateIssueState);
    passengerTable = new QTableWidget(0, 7); passengerTable->setObjectName("passengerTable");
    passengerTable->setHorizontalHeaderLabels({"盒子", "座位", "姓名", "手机号", "备用联系方式", "发放人", "发放时间"});
    passengerTable->setEditTriggers(QAbstractItemView::NoEditTriggers); passengerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    passengerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    passengerTable->setMinimumHeight(160); issueLayout->addWidget(passengerTable); issueLayout->addStretch();
    auto *scroll = new QScrollArea; ///< 窗口缩小时可滚动登记页。
    scroll->setWidgetResizable(true); scroll->setWidget(issuePage); tabs->addTab(scroll, "登记发放");
    tabs->addTab(makePrototype("第3天接入盒号查询、回收确认和遗失设备联系。",
        {"盒子编号或座位", "备用联系方式", "联系备注"}, {"查询", "确认已归还", "确认设备领取"}), "回收与遗失查询");
    tabs->addTab(makePrototype("第4天接入三种评价及意见记录。",
        {"乘客姓名或盒号", "意见与建议"}, {"1 满意", "2 不满意", "0 不表态"}), "评价与意见");
    tabs->addTab(makePrototype("第4天接入满意度统计和手机号抽奖。",
        {"抽奖人数"}, {"查看满意度", "抽取中奖手机号"}), "满意度与抽奖");
    layout->addWidget(tabs, 1);
    auto *footer = new QHBoxLayout; ///< 提示与数据目录入口。
    messageLabel = new QLabel("请登录后开始登记。"); messageLabel->setObjectName("messageLabel"); messageLabel->setWordWrap(true);
    auto *openDirectory = new QPushButton("打开数据目录"); ///< 定位 JSON 的按钮。
    footer->addWidget(messageLabel, 1); footer->addWidget(openDirectory); layout->addLayout(footer);
    connect(openDirectory, &QPushButton::clicked, this, [this] {
        if (!QDir().mkpath(this->manager.directory()) || !QDesktopServices::openUrl(QUrl::fromLocalFile(this->manager.directory())))
            showResult("无法打开数据目录：" + this->manager.directory(), {});
    });
    setCentralWidget(root); refresh();
}

bool MainWindow::showResult(const QString &error, const QString &success) {
    messageLabel->setText(error.isEmpty() ? success : error);
    messageLabel->setStyleSheet(error.isEmpty() ? "color: #166534;" : "color: #b91c1c;");
    return error.isEmpty();
}

void MainWindow::issuePassenger() {
    Passenger passenger; ///< 从登记表单读取的信息。
    passenger.name = nameField->text(); passenger.documentType = documentType->currentText(); passenger.documentNumber = documentField->text();
    passenger.boardingPass = boardingField->text(); passenger.seat = seatField->text(); passenger.phoneModel = phoneModelField->text();
    passenger.phone = phoneField->text(); passenger.backupContact = backupField->text(); passenger.boxNumber = boxField->text();
    if (!showResult(manager.issue(passenger), "登记成功，盒子 " + passenger.boxNumber.trimmed().toUpper() + " 已发放。")) return;
    // 只有保存成功才清空表单；失败时保留输入，方便修改。
    for (QLineEdit *field : {nameField, documentField, boardingField, seatField, phoneModelField, phoneField, backupField, boxField}) // field：待清空的登记输入框。
        field->clear();
    refresh(); nameField->setFocus();
}

void MainWindow::updateIssueState() {
    const FlightData &data = manager.data(); ///< 已成功保存的航班。
    const bool matchesSaved = !data.number.isEmpty() && numberField->text().trimmed().toUpper() == data.number &&
        dateField->date() == data.date && originField->text().trimmed() == data.origin &&
        destinationField->text().trimmed() == data.destination; ///< 界面航班与文件中的航班是否一致。
    // 防止用户改了航班却没有保存，导致乘客记到旧航班。
    issueButton->setEnabled(!manager.currentCrew().isEmpty() && matchesSaved);
}

void MainWindow::refresh() {
    const FlightData &data = manager.data(); ///< 最近保存成功的档案。
    const bool authenticated = !manager.currentCrew().isEmpty(); ///< 是否已通过登录。
    crewLabel->setText(authenticated ? "当前：" + manager.currentCrew() : "未登录");
    tabs->setEnabled(authenticated); flightSection->setEnabled(authenticated && data.passengers.isEmpty());
    numberField->setText(data.number); dateField->setDate(data.date); originField->setText(data.origin); destinationField->setText(data.destination);
    updateIssueState();
    // 未登录时只显示空表格，验证账号后再展示已保存的乘客资料。
    passengerTable->setRowCount(authenticated ? int(data.passengers.size()) : 0);
    for (int row = 0; row < passengerTable->rowCount(); ++row) { // row：表格中的乘客行。
        const Passenger &item = data.passengers[row]; ///< 当前乘客记录。
        const QStringList values = {item.boxNumber, item.seat, item.name, item.phone, item.backupContact, item.issuedBy, item.issuedAt}; ///< 各列文字。
        for (int column = 0; column < values.size(); ++column) // column：当前列。
            passengerTable->setItem(row, column, new QTableWidgetItem(values[column]));
    }
}
