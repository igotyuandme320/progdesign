#include "mainwindow.h"
#include "surveydialog.h"
#include <QComboBox>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QDir>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QSpinBox>
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

/** @brief 创建统一样式的只读表格。 @param headers 列标题。 @param name 控件名称。 @return 表格。 */
static QTableWidget *makeTable(const QStringList &headers, const QString &name) {
    auto *table = new QTableWidget(0, int(headers.size())); ///< 新的只读数据表。
    table->setObjectName(name); table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true); table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(38); table->setAlternatingRowColors(true);
    table->setMinimumHeight(150); return table;
}

/** @brief 转换评价数值。 @param rating -1、0、1、2。 @return 中文评价名称。 */
static QString ratingText(int rating) {
    if (rating == 1) return "满意";
    if (rating == 2) return "不满意";
    return rating == 0 ? "不表态" : "未评价";
}

/** @brief 显示比例并处理零分母。 @param numerator 分子。 @param denominator 分母。 @return 百分比或暂无数据。 */
static QString percentage(int numerator, int denominator) {
    return denominator == 0 ? QString("暂无数据") : QString::number(100.0 * numerator / denominator, 'f', 1) + "%";
}

/** @brief 获取设备状态文字。 @param passenger 一条乘客记录。 @return 待检查、无遗留、待领取或已领取。 */
static QString deviceStatus(const Passenger &passenger) {
    if (!passenger.returned) return "待检查";
    if (!passenger.deviceLeft) return "无遗留";
    return passenger.deviceClaimed ? "已领取" : "待领取";
}

MainWindow::MainWindow(BoxManager &manager) : manager(manager) {
    setWindowTitle("神气的盒子"); resize(1000, 860); setMinimumSize(800, 600);
    // 样式统一放在这里：浅色背景、白色区块和蓝色主按钮，方便课程项目修改。
    setStyleSheet(R"(
        QWidget { font-size: 14px; color: #24364b; }
        QWidget#mainContent { background: #f3f6fb; }
        QWidget#formPage { background: white; }
        QMessageBox { background: #f3f6fb; }
        QFrame#accountCard, QWidget#flightSection {
            background: white; border: 1px solid #dce5f0; border-radius: 10px;
        }
        QLabel#titleLabel { font-size: 27px; font-weight: bold; color: #1e4f8e; }
        QLabel#subtitleLabel, QLabel#descriptionLabel { color: #718096; }
        QLabel#sectionTitle { font-size: 16px; font-weight: bold; color: #355779; }
        QLabel#crewLabel { background: #edf4ff; color: #35629c; border-radius: 12px; padding: 6px 12px; }
        QLineEdit, QComboBox, QDateEdit, QSpinBox, QPlainTextEdit {
            background: white; border: 1px solid #cfdae7; border-radius: 6px;
            padding: 6px 9px; min-height: 18px; selection-background-color: #dbeafe;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QPlainTextEdit:focus { border-color: #377dd5; }
        QLineEdit:disabled, QComboBox:disabled, QDateEdit:disabled, QPlainTextEdit:disabled { background: #f2f5f9; color: #78879a; }
        QLabel#recoveryResult { background: #f5f9fe; border-radius: 6px; padding: 12px; }
        QComboBox QAbstractItemView { background: white; color: #24364b; selection-background-color: #e2edfc; }
        QCalendarWidget QWidget { background: white; color: #24364b; }
        QPushButton {
            background: white; border: 1px solid #cfdae7; border-radius: 6px; padding: 9px 16px;
        }
        QPushButton:hover { background: #f0f6ff; border-color: #7ba5db; }
        QPushButton:pressed { background: #e1ecfb; }
        QPushButton[primary="true"] { background: #2d6fc2; color: white; border-color: #2d6fc2; }
        QPushButton[primary="true"]:hover { background: #245ea7; border-color: #245ea7; }
        QPushButton[primary="true"]:pressed { background: #1e4f8e; }
        QPushButton:disabled, QPushButton[primary="true"]:disabled {
            background: #e9eef5; color: #8b99ab; border-color: #dce5ef;
        }
        QTabWidget::pane { background: white; border: 1px solid #dce5f0; border-radius: 8px; }
        QTabBar::tab {
            background: #e8eef7; color: #64768d; padding: 11px 20px; margin-right: 5px;
            border-top-left-radius: 7px; border-top-right-radius: 7px;
        }
        QTabBar::tab:selected { background: white; color: #2563a6; font-weight: bold; }
        QTabBar::tab:hover { color: #2563a6; }
        QTableWidget {
            background: white; alternate-background-color: #f5f9fe; border: 1px solid #dce5f0;
            selection-background-color: #e2edfc; selection-color: #214b7a;
        }
        QHeaderView::section { background: #eef4fb; color: #526d8d; padding: 9px; border: none; }
        QTableWidget::item { padding: 6px; }
        QScrollArea { background: white; border: none; }
        QScrollBar:vertical { background: #f3f6fb; width: 10px; }
        QScrollBar:horizontal { background: #f3f6fb; height: 10px; }
        QScrollBar::handle { background: #c4cfde; border-radius: 4px; min-width: 22px; min-height: 22px; }
        QScrollBar::add-line, QScrollBar::sub-line { width: 0px; height: 0px; }
        QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
        QLabel#messageLabel { background: #eaf0f7; color: #58708d; border-radius: 6px; padding: 9px 12px; }
    )");
    auto *root = new QWidget; ///< 主窗口内容。
    root->setObjectName("mainContent");
    auto *layout = new QVBoxLayout(root); ///< 总体布局。
    layout->setContentsMargins(22, 20, 22, 20); layout->setSpacing(10);
    auto *title = new QLabel("神气的盒子"); ///< 应用标题。
    title->setObjectName("titleLabel"); layout->addWidget(title);
    auto *subtitle = new QLabel("乘务员盒子管理"); ///< 简短的应用用途说明。
    subtitle->setObjectName("subtitleLabel"); layout->addWidget(subtitle);
    auto *accountCard = new QFrame; ///< 白色的登录区块。
    accountCard->setObjectName("accountCard");
    auto *account = new QHBoxLayout(accountCard); ///< 登录与换岗区域。
    account->setContentsMargins(16, 10, 16, 10); account->setSpacing(10);
    employeeField = new QLineEdit; employeeField->setObjectName("employeeField"); employeeField->setPlaceholderText("1001 或 1002");
    employeeField->setMaximumWidth(180);
    passwordField = new QLineEdit; passwordField->setObjectName("passwordField"); passwordField->setEchoMode(QLineEdit::Password);
    passwordField->setPlaceholderText("请输入密码"); passwordField->setMaximumWidth(240);
    auto *login = new QPushButton("登录 / 换岗"); ///< 验证账号的按钮。
    login->setProperty("primary", true); login->setCursor(Qt::PointingHandCursor);
    crewLabel = new QLabel; crewLabel->setObjectName("crewLabel");
    account->addWidget(new QLabel("工号")); account->addWidget(employeeField); account->addWidget(new QLabel("密码"));
    account->addWidget(passwordField); account->addWidget(login); account->addStretch(); account->addWidget(crewLabel); layout->addWidget(accountCard);
    // 点击或密码框回车均进行验证；成功后清空密码并开启业务区域。
    connect(login, &QPushButton::clicked, this, [this] {
        if (!this->manager.login(employeeField->text(), passwordField->text())) {
            showResult("工号或密码错误，请重新输入。", {}); return;
        }
        passwordField->clear(); refresh(); showResult({}, "登录成功，当前工号：" + this->manager.currentCrew());
    });
    connect(passwordField, &QLineEdit::returnPressed, login, &QPushButton::click);
    flightSection = new QWidget;
    flightSection->setObjectName("flightSection");
    auto *flight = new QHBoxLayout(flightSection); ///< 航班填写区域。
    flight->setContentsMargins(16, 10, 16, 10); flight->setSpacing(10);
    numberField = new QLineEdit; numberField->setObjectName("numberField"); numberField->setMaxLength(200);
    dateField = new QDateEdit; dateField->setDisplayFormat("yyyy-MM-dd"); dateField->setCalendarPopup(true);
    dateField->setMinimumWidth(135);
    originField = new QLineEdit; originField->setObjectName("originField"); originField->setMaxLength(200);
    destinationField = new QLineEdit; destinationField->setObjectName("destinationField"); destinationField->setMaxLength(200);
    saveFlightButton = new QPushButton("保存航班"); saveFlightButton->setObjectName("saveFlightButton");
    saveFlightButton->setProperty("primary", true); saveFlightButton->setCursor(Qt::PointingHandCursor);
    flight->addWidget(new QLabel("航班")); flight->addWidget(numberField); flight->addWidget(dateField);
    flight->addWidget(new QLabel("出发地")); flight->addWidget(originField); flight->addWidget(new QLabel("目的地"));
    flight->addWidget(destinationField); flight->addWidget(saveFlightButton); layout->addWidget(flightSection);
    connect(saveFlightButton, &QPushButton::clicked, this, [this] {
        if (showResult(this->manager.setFlight(numberField->text(), dateField->date(), originField->text(), destinationField->text()), "航班已保存。")) refresh();
    });
    tabs = new QTabWidget; tabs->setObjectName("tabs");
    auto *issuePage = new QWidget; ///< 登记页内容。
    issuePage->setObjectName("formPage");
    auto *issueLayout = new QVBoxLayout(issuePage); ///< 登记页纵向布局。
    issueLayout->setContentsMargins(20, 16, 20, 16); issueLayout->setSpacing(10);
    auto *description = new QLabel("先登录并保存航班，再填写所有登记项。修改航班后请重新保存；已发放后航班锁定，记录自动保存。"); ///< 操作说明。
    description->setObjectName("descriptionLabel");
    description->setWordWrap(true); issueLayout->addWidget(description);
    auto *formTitle = new QLabel("乘客登记"); ///< 登记表单的标题。
    formTitle->setObjectName("sectionTitle"); issueLayout->addWidget(formTitle);
    auto *forms = new QHBoxLayout; ///< 并排放置两列表单。
    forms->setSpacing(28);
    auto *form = new QFormLayout; ///< 左列：乘客身份及座位。
    auto *deviceForm = new QFormLayout; ///< 右列：手机、联系及盒号。
    form->setSpacing(10); deviceForm->setSpacing(10);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter); deviceForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // 两列填满可用宽度，并从顶部对齐，避免 Mac 默认表单把长号码挤窄。
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow); deviceForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop); deviceForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    nameField = addField(form, "姓名", "nameField");
    documentType = new QComboBox; documentType->addItems({"身份证", "护照", "其他"}); form->addRow("证件类型", documentType);
    documentField = addField(form, "证件号码", "documentField"); boardingField = addField(form, "登机牌", "boardingField");
    seatField = addField(form, "座位", "seatField"); phoneModelField = addField(deviceForm, "手机机型", "phoneModelField");
    phoneField = addField(deviceForm, "手机号", "phoneField"); backupField = addField(deviceForm, "备用联系方式", "backupField");
    backupField->setPlaceholderText("必填，可填写电话、邮箱等"); boxField = addField(deviceForm, "盒子编号", "boxField");
    forms->addLayout(form, 1); forms->addLayout(deviceForm, 1); issueLayout->addLayout(forms);
    issueButton = new QPushButton("保存登记并发放"); issueButton->setObjectName("issueButton");
    issueButton->setProperty("primary", true); issueButton->setCursor(Qt::PointingHandCursor);
    issueLayout->addWidget(issueButton, 0, Qt::AlignRight);
    connect(issueButton, &QPushButton::clicked, this, &MainWindow::issuePassenger);
    connect(numberField, &QLineEdit::textChanged, this, &MainWindow::updateIssueState);
    connect(originField, &QLineEdit::textChanged, this, &MainWindow::updateIssueState);
    connect(destinationField, &QLineEdit::textChanged, this, &MainWindow::updateIssueState);
    connect(dateField, &QDateEdit::dateChanged, this, &MainWindow::updateIssueState);
    auto *tableTitle = new QLabel("已发放记录"); ///< 记录表的标题。
    tableTitle->setObjectName("sectionTitle"); issueLayout->addWidget(tableTitle);
    passengerTable = new QTableWidget(0, 9); passengerTable->setObjectName("passengerTable");
    passengerTable->setHorizontalHeaderLabels({"盒子", "座位", "姓名", "手机号", "备用联系方式", "发放人", "发放时间", "盒子状态", "设备状态"});
    passengerTable->setEditTriggers(QAbstractItemView::NoEditTriggers); passengerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    passengerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    passengerTable->horizontalHeader()->setStretchLastSection(true);
    passengerTable->verticalHeader()->setVisible(false); passengerTable->verticalHeader()->setDefaultSectionSize(38);
    passengerTable->setAlternatingRowColors(true); passengerTable->setShowGrid(false);
    passengerTable->setMinimumHeight(150); issueLayout->addWidget(passengerTable, 1);
    auto *scroll = new QScrollArea; ///< 窗口缩小时可滚动登记页。
    scroll->setFrameShape(QFrame::NoFrame); scroll->setWidgetResizable(true); scroll->setWidget(issuePage); tabs->addTab(scroll, "登记发放");
    tabs->addTab(makeRecoveryPage(), "回收与遗失查询");
    // 双击记录直接查询该盒子，乘务员不必重复抄写编号。
    connect(passengerTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { // row：双击的乘客行。
        queryType->setCurrentIndex(0); queryField->setText(this->manager.data().passengers[row].boxNumber);
        tabs->setCurrentIndex(1); searchPassenger();
    });
    tabs->addTab(makeSurveyPage(), "评价与意见");
    tabs->addTab(makeStatisticsPage(), "满意度与抽奖");
    layout->addWidget(tabs, 1);
    auto *footer = new QHBoxLayout; ///< 提示与数据目录入口。
    messageLabel = new QLabel("请登录后开始登记。"); messageLabel->setObjectName("messageLabel"); messageLabel->setWordWrap(true);
    auto *openDirectory = new QPushButton("打开数据目录"); ///< 定位 JSON 的按钮。
    openDirectory->setCursor(Qt::PointingHandCursor);
    newFlightButton = new QPushButton("新建航班"); newFlightButton->setObjectName("newFlightButton");
    connect(newFlightButton, &QPushButton::clicked, this, &MainWindow::newFlight);
    footer->addWidget(messageLabel, 1); footer->addWidget(newFlightButton); footer->addWidget(openDirectory); layout->addLayout(footer);
    connect(openDirectory, &QPushButton::clicked, this, [this] {
        if (!QDir().mkpath(this->manager.directory()) || !QDesktopServices::openUrl(QUrl::fromLocalFile(this->manager.directory())))
            showResult("无法打开数据目录：" + this->manager.directory(), {});
    });
    setCentralWidget(root); refresh();
}

QWidget *MainWindow::makeSurveyPage() {
    auto *page = new QWidget; ///< 乘务员选择评价乘客的页面。
    page->setObjectName("formPage");
    auto *layout = new QVBoxLayout(page); ///< 评价管理页布局。
    layout->setContentsMargins(20, 16, 20, 16); layout->setSpacing(12);
    auto *description = new QLabel("选择尚未评价的乘客，再交给乘客填写。评价页不显示乘客资料，返回需要乘务员工号和密码。"); ///< 操作说明。
    description->setObjectName("descriptionLabel"); description->setWordWrap(true); layout->addWidget(description);
    auto *selection = new QHBoxLayout; ///< 乘客选择与进入按钮。
    surveyPassenger = new QComboBox; surveyPassenger->setObjectName("surveyPassenger");
    startSurveyButton = new QPushButton("进入乘客评价页"); startSurveyButton->setObjectName("startSurveyButton");
    startSurveyButton->setProperty("primary", true);
    selection->addWidget(surveyPassenger, 1); selection->addWidget(startSurveyButton); layout->addLayout(selection);
    connect(startSurveyButton, &QPushButton::clicked, this, &MainWindow::startSurvey);
    surveyTable = makeTable({"盒号", "座位", "姓名", "评价", "意见与建议"}, "surveyTable");
    layout->addWidget(surveyTable, 1); return page;
}

QWidget *MainWindow::makeStatisticsPage() {
    auto *page = new QWidget; ///< 统计与抽奖页面内容。
    page->setObjectName("formPage");
    auto *layout = new QVBoxLayout(page); ///< 统计页布局。
    layout->setContentsMargins(20, 16, 20, 16); layout->setSpacing(12);
    surveySummary = new QLabel; surveySummary->setObjectName("surveySummary"); surveySummary->setWordWrap(true);
    layout->addWidget(surveySummary);
    auto *opinionTitle = new QLabel("乘客意见（双击查看完整内容）"); ///< 意见列表标题。
    opinionTitle->setObjectName("sectionTitle"); layout->addWidget(opinionTitle);
    opinionTable = makeTable({"盒号", "姓名", "评价", "意见与建议"}, "opinionTable"); layout->addWidget(opinionTable);
    connect(opinionTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { // row：要查看意见的行。
        QDialog dialog(this); ///< 只读的完整意见窗口。
        dialog.setWindowTitle("乘客意见"); dialog.resize(600, 360);
        auto *layout = new QVBoxLayout(&dialog); ///< 意见窗口布局。
        auto *text = new QPlainTextEdit; ///< 完整意见文本。
        text->setReadOnly(true); text->setPlainText(opinionTable->item(row, 3)->text()); layout->addWidget(text);
        auto *close = new QPushButton("关闭"); ///< 返回统计页面的按钮。
        layout->addWidget(close); connect(close, &QPushButton::clicked, &dialog, &QDialog::accept); dialog.exec();
    });
    auto *lotteryTitle = new QLabel("手机号抽奖"); ///< 抽奖区标题。
    lotteryTitle->setObjectName("sectionTitle"); layout->addWidget(lotteryTitle);
    lotterySummary = new QLabel; lotterySummary->setObjectName("lotterySummary"); lotterySummary->setWordWrap(true); layout->addWidget(lotterySummary);
    auto *actions = new QHBoxLayout; ///< 抽奖人数与按钮。
    winnerCount = new QSpinBox; winnerCount->setObjectName("winnerCount"); winnerCount->setRange(1, 999); winnerCount->setValue(1);
    drawButton = new QPushButton("抽取并保存中奖号码"); drawButton->setObjectName("drawButton"); drawButton->setProperty("primary", true);
    actions->addWidget(new QLabel("抽奖人数")); actions->addWidget(winnerCount); actions->addStretch(); actions->addWidget(drawButton); layout->addLayout(actions);
    connect(drawButton, &QPushButton::clicked, this, [this] {
        QStringList phones; ///< 保存成功后展示的本轮中奖手机号。
        const QString error = this->manager.drawLottery(winnerCount->value(), phones); ///< 抽奖及保存结果。
        if (showResult(error, "本轮中奖：" + phones.join("、"))) refresh();
    });
    winnerTable = makeTable({"中奖手机号", "操作工号", "抽奖时间"}, "winnerTable");
    winnerTable->setColumnWidth(0, 170); layout->addWidget(winnerTable);
    auto *scroll = new QScrollArea; ///< 小窗口下滚动查看全部统计。
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame); scroll->setWidget(page); return scroll;
}

void MainWindow::startSurvey() {
    const QString box = surveyPassenger->currentData().toString(); ///< 乘务员选中的待评价盒号。
    if (manager.findPassenger(box) < 0 || !resolveContactDraft()) return;
    SurveyDialog dialog(manager, box, this); ///< 一人一次的专用评价窗口。
    // 管理窗口隐藏后再展示乘客页，避免后台露出其他乘客资料。
    hide(); dialog.exec(); show(); refresh();
}

void MainWindow::refreshSurvey() {
    const bool authenticated = !manager.currentCrew().isEmpty(); ///< 是否允许显示乘客资料。
    const QString previous = surveyPassenger->currentData().toString(); ///< 刷新前的待评价乘客。
    surveyPassenger->clear(); surveyTable->setRowCount(0); opinionTable->setRowCount(0); winnerTable->setRowCount(0);
    if (authenticated) {
        for (const Passenger &item : manager.data().passengers) { // item：一名登记乘客。
            if (item.rating == -1) surveyPassenger->addItem(item.boxNumber + "　" + item.seat + "　" + item.name, item.boxNumber);
            const int row = surveyTable->rowCount(); ///< 评价表中新增的行。
            surveyTable->insertRow(row);
            const QStringList values = {item.boxNumber, item.seat, item.name, ratingText(item.rating), item.opinion}; ///< 一行评价内容。
            for (int column = 0; column < values.size(); ++column) // column：评价表列下标。
                surveyTable->setItem(row, column, new QTableWidgetItem(values[column]));
            if (item.rating >= 0 && !item.opinion.isEmpty()) {
                const int opinionRow = opinionTable->rowCount(); ///< 意见表中新增的行。
                opinionTable->insertRow(opinionRow);
                const QStringList opinions = {item.boxNumber, item.name, ratingText(item.rating), item.opinion}; ///< 一行非空意见。
                for (int column = 0; column < opinions.size(); ++column) // column：意见表列下标。
                    opinionTable->setItem(opinionRow, column, new QTableWidgetItem(opinions[column]));
            }
        }
        for (const LotteryWinner &winner : manager.data().winners) { // winner：已保存的中奖记录。
            const int row = winnerTable->rowCount(); ///< 中奖表新增的行。
            winnerTable->insertRow(row);
            winnerTable->setItem(row, 0, new QTableWidgetItem(winner.phone));
            winnerTable->setItem(row, 1, new QTableWidgetItem(winner.drawnBy));
            winnerTable->setItem(row, 2, new QTableWidgetItem(winner.drawnAt));
        }
    }
    const int previousIndex = surveyPassenger->findData(previous); ///< 原来选中的乘客在新列表中的位置。
    if (previousIndex >= 0) surveyPassenger->setCurrentIndex(previousIndex);
    startSurveyButton->setEnabled(authenticated && surveyPassenger->count() > 0);
    newFlightButton->setEnabled(authenticated); drawButton->setEnabled(authenticated);
    const SurveyStats stats = manager.surveyStats(); ///< 从原始数据计算的人数。
    const int submitted = stats.satisfied + stats.dissatisfied + stats.neutral; ///< 已评价人数，包含不表态。
    surveySummary->setText(QString("满意 %1 人　不满意 %2 人　不表态 %3 人\n已评价 %4 人　未评价 %5 人\n"
        "参与率（已评价 ÷ 登记人数）：%6\n满意率（满意 ÷ 全部已评价）：%7\n已表态满意率（满意 ÷ 满意与不满意人数）：%8")
        .arg(stats.satisfied).arg(stats.dissatisfied).arg(stats.neutral).arg(submitted).arg(stats.total - submitted)
        .arg(percentage(submitted, stats.total)).arg(percentage(stats.satisfied, submitted))
        .arg(percentage(stats.satisfied, stats.satisfied + stats.dissatisfied)));
    lotterySummary->setText(QString("已评价乘客按手机号去重，包含选择 0 的乘客。\n本轮可抽 %1 个号码，已有 %2 个号码中奖；中奖号码不再参与后续抽取。")
        .arg(manager.lotteryCandidates().size()).arg(authenticated ? manager.data().winners.size() : 0));
}

void MainWindow::newFlight() {
    if (!resolveContactDraft()) return;
    QDialog dialog(this); ///< 新航班信息表单。
    dialog.setObjectName("newFlightDialog"); dialog.setWindowTitle("备份并新建航班"); dialog.resize(460, 320);
    auto *form = new QFormLayout(&dialog); ///< 新航班表单布局。
    auto *description = new QLabel("需先回收全部盒子并处理完遗留设备。旧航班会备份到数据目录的 history 文件夹。"); ///< 备份说明。
    description->setWordWrap(true); form->addRow(description);
    auto *number = addField(form, "航班号", "newNumber"); ///< 新航班号。
    auto *date = new QDateEdit(QDate::currentDate()); ///< 新航班日期。
    date->setObjectName("newDate"); date->setDisplayFormat("yyyy-MM-dd"); date->setCalendarPopup(true); form->addRow("日期", date);
    auto *origin = addField(form, "出发地", "newOrigin"); ///< 新出发地。
    auto *destination = addField(form, "目的地", "newDestination"); ///< 新目的地。
    auto *errorLabel = new QLabel; ///< 新建失败时在表单内显示的原因。
    errorLabel->setWordWrap(true); errorLabel->setTextFormat(Qt::PlainText); form->addRow(errorLabel);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); ///< 确认和取消按钮。
    buttons->button(QDialogButtonBox::Ok)->setText("备份并新建"); buttons->button(QDialogButtonBox::Cancel)->setText("取消"); form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, &dialog, number, date, origin, destination, errorLabel] {
        const QString error = manager.startNewFlight(number->text(), date->date(), origin->text(), destination->text()); ///< 备份及提交结果。
        if (error.isEmpty()) dialog.accept(); else errorLabel->setText(error);
    });
    if (dialog.exec() != QDialog::Accepted) return;
    selectedBox.clear(); contactBox.clear(); contactField->clear(); queryField->clear();
    for (QLineEdit *field : {nameField, documentField, boardingField, seatField, phoneModelField, phoneField, backupField, boxField}) // field：旧航班未提交的表单内容。
        field->clear();
    tabs->setCurrentIndex(0); refresh(); showResult({}, "新航班已建立，旧航班已备份到 history。");
}

QWidget *MainWindow::makeRecoveryPage() {
    auto *page = new QWidget; ///< 回收页面内容。
    page->setObjectName("formPage");
    auto *layout = new QVBoxLayout(page); ///< 回收页面布局。
    layout->setContentsMargins(20, 16, 20, 16); layout->setSpacing(12);
    auto *description = new QLabel("按盒号或座位查询，确认收到盒子后回收；发现遗留设备时，通过备用联系方式联系乘客。"); ///< 回收操作说明。
    description->setObjectName("descriptionLabel"); description->setWordWrap(true); layout->addWidget(description);
    recoverySummary = new QLabel; recoverySummary->setObjectName("sectionTitle"); recoverySummary->setWordWrap(true);
    layout->addWidget(recoverySummary);
    auto *search = new QHBoxLayout; ///< 查询条件与按钮区域。
    queryType = new QComboBox; queryType->setObjectName("queryType"); queryType->addItems({"盒子编号", "座位号"});
    queryField = new QLineEdit; queryField->setObjectName("queryField"); queryField->setMaxLength(200);
    queryField->setPlaceholderText("例如 BOX001 或 12A");
    auto *searchButton = new QPushButton("查询"); ///< 执行精确查询的按钮。
    searchButton->setObjectName("searchButton"); searchButton->setProperty("primary", true); searchButton->setCursor(Qt::PointingHandCursor);
    search->addWidget(queryType); search->addWidget(queryField, 1); search->addWidget(searchButton); layout->addLayout(search);
    recoveryResult = new QLabel; recoveryResult->setObjectName("recoveryResult"); recoveryResult->setWordWrap(true);
    recoveryResult->setTextFormat(Qt::PlainText); recoveryResult->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(recoveryResult);
    auto *returns = new QHBoxLayout; ///< 回收选项和按钮。
    deviceLeftCheck = new QCheckBox("盒内有遗留设备"); deviceLeftCheck->setObjectName("deviceLeftCheck");
    returnButton = new QPushButton("确认已归还"); returnButton->setObjectName("returnButton");
    returnButton->setProperty("primary", true); returnButton->setCursor(Qt::PointingHandCursor);
    returns->addWidget(deviceLeftCheck); returns->addStretch(); returns->addWidget(returnButton); layout->addLayout(returns);
    auto *noteLabel = new QLabel("联系备注（最多 2000 字符；修改后先保存，再确认领取）"); ///< 联系备注说明。
    noteLabel->setWordWrap(true); layout->addWidget(noteLabel);
    contactField = new QPlainTextEdit; contactField->setObjectName("contactField"); contactField->setMaximumHeight(95);
    contactField->setPlaceholderText("例如：已通过备用电话联系家属，乘客稍后来领取。"); layout->addWidget(contactField);
    auto *actions = new QHBoxLayout; ///< 联系及领取操作区域。
    saveContactButton = new QPushButton("保存联系备注"); saveContactButton->setObjectName("saveContactButton");
    claimButton = new QPushButton("确认设备领取"); claimButton->setObjectName("claimButton");
    saveContactButton->setCursor(Qt::PointingHandCursor); claimButton->setCursor(Qt::PointingHandCursor);
    actions->addStretch(); actions->addWidget(saveContactButton); actions->addWidget(claimButton); layout->addLayout(actions); layout->addStretch();
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchPassenger);
    connect(queryField, &QLineEdit::returnPressed, searchButton, &QPushButton::click);
    // 查询条件改变后立即取消原选中记录，避免误操作上一次查询的盒子。
    connect(queryField, &QLineEdit::textChanged, this, [this] { selectedBox.clear(); refreshRecovery(); });
    connect(queryType, &QComboBox::currentIndexChanged, this, [this] { selectedBox.clear(); refreshRecovery(); });
    connect(returnButton, &QPushButton::clicked, this, [this] {
        if (showResult(this->manager.returnBox(selectedBox, deviceLeftCheck->isChecked()), "盒子已归还。")) refresh();
    });
    connect(saveContactButton, &QPushButton::clicked, this, [this] {
        if (showResult(this->manager.saveContactNote(contactBox, contactField->toPlainText()), "联系备注已保存。")) refresh();
    });
    connect(claimButton, &QPushButton::clicked, this, [this] {
        const int index = this->manager.findPassenger(selectedBox); ///< 当前选中记录的位置。
        if (index >= 0 && contactField->toPlainText().trimmed() != this->manager.data().passengers[index].contactNote) {
            showResult("请先保存修改的联系备注，再确认领取。", {}); return;
        }
        if (showResult(this->manager.claimDevice(selectedBox), "遗留设备已领取。")) refresh();
    });
    auto *scroll = new QScrollArea; ///< 小窗口下的回收滚动区域。
    scroll->setFrameShape(QFrame::NoFrame); scroll->setWidgetResizable(true); scroll->setWidget(page); return scroll;
}

void MainWindow::searchPassenger() {
    const int index = manager.findPassenger(queryField->text(), queryType->currentIndex() == 1); ///< 查询结果的位置。
    const QString nextBox = index >= 0 ? manager.data().passengers[index].boxNumber : QString{}; ///< 本次准备显示的盒号。
    if (nextBox != contactBox && !resolveContactDraft()) return;
    selectedBox = nextBox;
    refreshRecovery();
    showResult(index < 0 ? "未找到记录，请检查查询类型和编号。" : QString{}, "已找到该乘客，可以回收或处理遗留设备。");
}

void MainWindow::refreshRecovery() {
    const bool authenticated = !manager.currentCrew().isEmpty(); ///< 是否已登录。
    int returnedCount = 0; ///< 已归还盒子数量。
    int pendingDevices = 0; ///< 遗留设备待领取数量。
    if (authenticated) {
        for (const Passenger &item : manager.data().passengers) { // item：参与数量统计的一条记录。
            if (item.returned) ++returnedCount;
            if (item.deviceLeft && !item.deviceClaimed) ++pendingDevices;
        }
    }
    const int total = authenticated ? int(manager.data().passengers.size()) : 0; ///< 已发放总数。
    recoverySummary->setText(QString("已发放 %1　已回收 %2　待回收 %3　待领取设备 %4").arg(total).arg(returnedCount).arg(total - returnedCount).arg(pendingDevices));
    const int index = manager.findPassenger(selectedBox); ///< 当前选中盒号对应的位置。
    if (index < 0) {
        recoveryResult->setText("查询后显示乘客、备用联系方式和处理状态。");
        returnButton->setEnabled(false); deviceLeftCheck->setEnabled(false); deviceLeftCheck->setChecked(false);
        const bool draft = hasContactDraft(); ///< 原记录是否仍有未保存备注。
        if (draft) recoveryResult->setText("盒子 " + contactBox + " 有未保存的联系备注，请保存后继续查询。");
        else { contactBox.clear(); contactField->clear(); }
        contactField->setEnabled(draft); saveContactButton->setEnabled(draft); claimButton->setEnabled(false); return;
    }
    const Passenger &item = manager.data().passengers[index]; ///< 最近保存成功的查询记录。
    QString details = QString("盒子：%1　座位：%2　姓名：%3\n手机：%4　机型：%5\n备用联系方式：%6\n盒子状态：%7　设备状态：%8\n发放：%9　%10")
        .arg(item.boxNumber).arg(item.seat).arg(item.name).arg(item.phone).arg(item.phoneModel).arg(item.backupContact)
        .arg(item.returned ? "已归还" : "待回收").arg(deviceStatus(item)).arg(item.issuedBy).arg(item.issuedAt); ///< 查询详情文字。
    if (item.returned) details += "\n回收：" + item.returnedBy + "　" + item.returnedAt;
    if (!item.contactNote.isEmpty()) details += "\n联系：" + item.contactedBy + "　" + item.contactedAt;
    if (item.deviceClaimed) details += "\n领取：" + item.claimedBy + "　" + item.claimedAt;
    recoveryResult->setText(details);
    returnButton->setEnabled(!item.returned); deviceLeftCheck->setEnabled(!item.returned); deviceLeftCheck->setChecked(item.deviceLeft);
    const bool pending = item.returned && item.deviceLeft && !item.deviceClaimed; ///< 设备是否仍待处理。
    // 普通刷新和换岗保留同一盒子的草稿，只有切换记录或保存后才回填。
    if (contactBox != item.boxNumber || !hasContactDraft()) contactField->setPlainText(item.contactNote);
    contactBox = item.boxNumber; contactField->setEnabled(pending);
    saveContactButton->setEnabled(pending); claimButton->setEnabled(pending);
}

bool MainWindow::hasContactDraft() const {
    const int index = manager.findPassenger(contactBox); ///< 当前备注对应的记录位置。
    return index >= 0 && contactField->toPlainText().trimmed() != manager.data().passengers[index].contactNote;
}

bool MainWindow::resolveContactDraft() {
    if (!hasContactDraft()) return true;
    QMessageBox dialog(QMessageBox::Question, "联系备注未保存", "盒子 " + contactBox + " 的联系备注尚未保存。请选择如何处理。",
                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this); ///< 草稿处理对话框。
    dialog.button(QMessageBox::Save)->setText("保存"); dialog.button(QMessageBox::Discard)->setText("放弃修改");
    dialog.button(QMessageBox::Cancel)->setText("取消"); dialog.setDefaultButton(QMessageBox::Save);
    const int choice = dialog.exec(); ///< 用户选择的草稿处理方式。
    if (choice != QMessageBox::Save && choice != QMessageBox::Discard) return false;
    if (choice == QMessageBox::Save && !showResult(manager.saveContactNote(contactBox, contactField->toPlainText()), "联系备注已保存。"))
        return false;
    const int index = manager.findPassenger(contactBox); ///< 保存或放弃后恢复正式备注的位置。
    if (index >= 0) contactField->setPlainText(manager.data().passengers[index].contactNote);
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (resolveContactDraft()) QMainWindow::closeEvent(event);
    else event->ignore();
}

bool MainWindow::showResult(const QString &error, const QString &success) {
    messageLabel->setText(error.isEmpty() ? success : error);
    messageLabel->setStyleSheet(error.isEmpty() ? "color: #166534; background: #eaf5ef;" : "color: #b91c1c; background: #fff0f0;");
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
        const QStringList values = {item.boxNumber, item.seat, item.name, item.phone, item.backupContact, item.issuedBy, item.issuedAt,
            item.returned ? "已归还" : "待回收", deviceStatus(item)}; ///< 各列文字。
        for (int column = 0; column < values.size(); ++column) // column：当前列。
            passengerTable->setItem(row, column, new QTableWidgetItem(values[column]));
    }
    refreshRecovery();
    refreshSurvey();
}
