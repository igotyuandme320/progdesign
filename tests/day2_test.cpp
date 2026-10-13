#include "boxmanager.h"
#include "mainwindow.h"
#include <QDateEdit>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QtTest>

/** 第二天验收：使用真实业务类与临时文件，不修改正式数据。 */
class Day2Test : public QObject {
    Q_OBJECT
private slots:
    /** @brief 检查登录、换岗及未登录禁止操作。 @return 无。 */
    void loginAndPermission();
    /** @brief 检查必填、手机号和身份证的错误输入。 @return 无。 */
    void inputValidation();
    /** @brief 检查编号清理与四类重复登记。 @return 无。 */
    void duplicateIssue();
    /** @brief 检查换岗后发放记录及重启恢复。 @return 无。 */
    void saveAndReload();
    /** @brief 检查损坏文件禁止覆盖。 @return 无。 */
    void corruptFile();
    /** @brief 检查保存失败保留原有记录。 @return 无。 */
    void failedSave();
    /** @brief 通过窗口验证登录、发放、错误提示与恢复表格。 @return 无。 */
    void windowWorkflow();
};

/** @brief 创建人工核对的演示输入。 @return 一条有效乘客信息。 */
static Passenger samplePassenger() {
    Passenger passenger; ///< 不对应真实乘客的测试数据。
    passenger.name = "演示乘客"; passenger.documentType = "身份证";
    passenger.documentNumber = "11010519491231002X"; passenger.boardingPass = "BP001";
    passenger.seat = "12A"; passenger.phoneModel = "演示手机"; passenger.phone = "13800000001";
    passenger.backupContact = "family@example.com"; passenger.boxNumber = "BOX001";
    return passenger;
}

void Day2Test::loginAndPermission() {
    QTemporaryDir directory; ///< 隔离的测试目录。
    BoxManager manager(directory.path()); ///< 待测试业务对象。
    QString error; ///< 加载错误。
    QVERIFY(!manager.login("1001", "Box@123456"));
    QVERIFY(manager.load(error));
    QVERIFY(!manager.setFlight("MU5101", QDate(2026, 10, 13), "上海", "北京").isEmpty());
    QVERIFY(!manager.login("1001", "wrong"));
    QVERIFY(manager.login("1001", "Box@123456"));
    QVERIFY(!manager.login("1002", "wrong"));
    QCOMPARE(manager.currentCrew(), QString("1001"));
    QVERIFY(manager.login("1002", "Box@123456"));
    QCOMPARE(manager.currentCrew(), QString("1002"));
}

void Day2Test::inputValidation() {
    QTemporaryDir directory; ///< 临时数据目录。
    BoxManager manager(directory.path()); ///< 校验业务对象。
    QString error; ///< 加载结果。
    QVERIFY(manager.load(error)); QVERIFY(manager.login("1001", "Box@123456"));
    QVERIFY(!manager.issue(samplePassenger()).isEmpty()); // 未保存航班不能发放。
    QVERIFY(!manager.setFlight("", QDate(2026, 10, 13), "上海", "北京").isEmpty());
    QVERIFY(manager.setFlight("MU5101", QDate(2026, 10, 13), "上海", "北京").isEmpty());
    Passenger passenger = samplePassenger(); ///< 逐项构造错误输入。
    passenger.backupContact = "   "; QVERIFY(!manager.issue(passenger).isEmpty());
    passenger = samplePassenger(); passenger.phone = "12345678901"; QVERIFY(!manager.issue(passenger).isEmpty());
    passenger = samplePassenger(); passenger.documentNumber = "110105194912310021"; QVERIFY(!manager.issue(passenger).isEmpty());
    passenger = samplePassenger(); passenger.documentNumber = "11010519490230002X"; QVERIFY(!manager.issue(passenger).isEmpty());
    QCOMPARE(manager.data().passengers.size(), 0);
    QVERIFY(manager.issue(samplePassenger()).isEmpty());
}

void Day2Test::duplicateIssue() {
    QTemporaryDir directory; ///< 临时数据目录。
    BoxManager manager(directory.path()); ///< 发放业务对象。
    QString error; ///< 加载结果。
    QVERIFY(manager.load(error)); QVERIFY(manager.login("1001", "Box@123456"));
    QVERIFY(manager.setFlight(" mu5101 ", QDate(2026, 10, 13), " 上海 ", "北京").isEmpty());
    Passenger first = samplePassenger(); ///< 带空白和小写的第一条信息。
    first.documentNumber = "11010519491231002x"; first.boxNumber = " box001 ";
    QVERIFY(manager.issue(first).isEmpty());
    QCOMPARE(manager.data().number, QString("MU5101"));
    QCOMPARE(manager.data().passengers.first().boxNumber, QString("BOX001"));
    Passenger next = samplePassenger(); ///< 合法的另一名乘客。
    next.documentType = "护照"; next.documentNumber = "E12345678"; next.boardingPass = "BP002";
    next.seat = "12B"; next.boxNumber = "BOX002";
    Passenger duplicate = next; ///< 一次只构造一个重复字段。
    duplicate.boxNumber = "box001"; QVERIFY(!manager.issue(duplicate).isEmpty());
    duplicate = next; duplicate.documentNumber = "11010519491231002X"; QVERIFY(!manager.issue(duplicate).isEmpty());
    duplicate = next; duplicate.seat = "12a"; QVERIFY(!manager.issue(duplicate).isEmpty());
    duplicate = next; duplicate.boardingPass = "bp001"; QVERIFY(!manager.issue(duplicate).isEmpty());
    QCOMPARE(manager.data().passengers.size(), 1);
    QVERIFY(manager.issue(next).isEmpty());
    QVERIFY(!manager.setFlight("CA1234", QDate(2026, 10, 14), "北京", "广州").isEmpty());
}

void Day2Test::saveAndReload() {
    QTemporaryDir directory; ///< 跨实例共用的临时目录。
    BoxManager manager(directory.path()); ///< 首个实例。
    QString error; ///< 文件错误。
    QVERIFY(manager.load(error)); QVERIFY(manager.login("1001", "Box@123456"));
    QVERIFY(manager.setFlight("MU5101", QDate(2026, 10, 13), "上海", "北京").isEmpty());
    QVERIFY(manager.login("1002", "Box@123456"));
    QVERIFY(manager.issue(samplePassenger()).isEmpty());
    BoxManager restored(directory.path()); ///< 模拟重启后的实例。
    QVERIFY(restored.load(error));
    QCOMPARE(restored.data().passengers.size(), 1);
    QCOMPARE(restored.data().passengers.first().backupContact, QString("family@example.com"));
    QCOMPARE(restored.data().passengers.first().issuedBy, QString("1002"));
    QVERIFY(!restored.data().passengers.first().issuedAt.isEmpty());
    QVERIFY(restored.currentCrew().isEmpty());
}

void Day2Test::corruptFile() {
    QTemporaryDir directory; ///< 隔离的文件目录。
    QFile file(directory.filePath("records.json")); ///< 故意损坏的 JSON。
    QVERIFY(file.open(QIODevice::WriteOnly)); file.write("{broken"); file.close();
    BoxManager manager(directory.path()); ///< 禁止继续写入的实例。
    QString error; ///< 错误提示。
    QVERIFY(!manager.load(error)); QVERIFY(!error.isEmpty());
    QVERIFY(!manager.login("1001", "Box@123456"));
    QVERIFY(!manager.setFlight("MU5101", QDate(2026, 10, 13), "上海", "北京").isEmpty());
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), QByteArray("{broken"));
}

void Day2Test::failedSave() {
    QTemporaryDir directory; ///< 隔离目录。
    BoxManager manager(directory.path()); ///< 正常加载并登记的实例。
    QString error; ///< 操作错误。
    QVERIFY(manager.load(error)); QVERIFY(manager.login("1001", "Box@123456"));
    QVERIFY(manager.setFlight("MU5101", QDate(2026, 10, 13), "上海", "北京").isEmpty());
    QVERIFY(manager.issue(samplePassenger()).isEmpty());
    // 把存储目录移走并在原位置放普通文件，让下一次保存真实失败。
    const QString movedPath = directory.path() + "-saved"; ///< 原有效数据的临时备份。
    QVERIFY(QDir().rename(directory.path(), movedPath));
    QFile blocker(directory.path()); ///< 阻止新建目录的文件。
    QVERIFY(blocker.open(QIODevice::WriteOnly)); blocker.close();
    Passenger next = samplePassenger(); ///< 第二名乘客。
    next.documentType = "护照"; next.documentNumber = "E12345678"; next.boardingPass = "BP002";
    next.seat = "12B"; next.boxNumber = "BOX002";
    QVERIFY(!manager.issue(next).isEmpty());
    QCOMPARE(manager.data().passengers.size(), 1);
    QVERIFY(blocker.remove()); QVERIFY(QDir().rename(movedPath, directory.path()));
    BoxManager restored(directory.path()); ///< 检查原有效文件。
    QVERIFY(restored.load(error)); QCOMPARE(restored.data().passengers.size(), 1);
}

void Day2Test::windowWorkflow() {
    QTemporaryDir directory; ///< 窗口使用的临时目录。
    BoxManager manager(directory.path()); ///< 界面的真实业务对象。
    QString error; ///< 数据加载结果。
    QVERIFY(manager.load(error));
    MainWindow window(manager); ///< 被验收的主窗口。
    QPushButton *login = nullptr; ///< 登录入口。
    for (QPushButton *button : window.findChildren<QPushButton *>()) // button：窗口中的一个按钮。
        if (button->text().startsWith("登录")) login = button;
    QVERIFY(login); QVERIFY(login->isEnabled());
    auto *tabs = window.findChild<QTabWidget *>("tabs"); ///< 功能标签页。
    QVERIFY(tabs); QVERIFY(!tabs->isEnabled());
    auto *employee = window.findChild<QLineEdit *>("employeeField"); ///< 工号输入框。
    auto *password = window.findChild<QLineEdit *>("passwordField"); ///< 密码输入框。
    auto *message = window.findChild<QLabel *>("messageLabel"); ///< 操作反馈。
    QVERIFY(employee && password && message);
    employee->setText("1001"); password->setText("wrong"); login->click();
    QVERIFY(!tabs->isEnabled()); QVERIFY(!message->text().isEmpty());
    password->setText("Box@123456"); QTest::keyClick(password, Qt::Key_Return);
    QVERIFY(tabs->isEnabled()); QCOMPARE(password->text(), QString());
    window.findChild<QLineEdit *>("numberField")->setText("MU5101");
    window.findChild<QLineEdit *>("originField")->setText("上海");
    window.findChild<QLineEdit *>("destinationField")->setText("北京");
    window.findChild<QPushButton *>("saveFlightButton")->click();
    auto *issue = window.findChild<QPushButton *>("issueButton"); ///< 发放入口。
    auto *table = window.findChild<QTableWidget *>("passengerTable"); ///< 发放记录表。
    QVERIFY(issue && table); QVERIFY(issue->isEnabled());
    // 航班四个字段任一变更，都应先保存，不能把登记写到旧航班。
    window.findChild<QLineEdit *>("numberField")->setText("CA1234");
    QVERIFY(!issue->isEnabled()); issue->click(); QCOMPARE(manager.data().passengers.size(), 0);
    window.findChild<QLineEdit *>("numberField")->setText("MU5101"); QVERIFY(issue->isEnabled());
    window.findChild<QLineEdit *>("originField")->setText("广州"); QVERIFY(!issue->isEnabled());
    window.findChild<QLineEdit *>("originField")->setText("上海");
    window.findChild<QLineEdit *>("destinationField")->setText("深圳"); QVERIFY(!issue->isEnabled());
    window.findChild<QLineEdit *>("destinationField")->setText("北京");
    auto *date = window.findChild<QDateEdit *>(); ///< 航班日期输入框。
    const QDate originalDate = date->date(); ///< 已保存日期。
    date->setDate(originalDate.addDays(1)); QVERIFY(!issue->isEnabled());
    date->setDate(originalDate); QVERIFY(issue->isEnabled());
    window.findChild<QLineEdit *>("nameField")->setText("演示乘客");
    window.findChild<QLineEdit *>("documentField")->setText("11010519491231002X");
    window.findChild<QLineEdit *>("boardingField")->setText("BP001");
    window.findChild<QLineEdit *>("seatField")->setText("12A");
    window.findChild<QLineEdit *>("phoneModelField")->setText("演示手机");
    window.findChild<QLineEdit *>("phoneField")->setText("13800000001");
    window.findChild<QLineEdit *>("boxField")->setText("BOX001");
    issue->click(); // 缺少备用联系时不能发放。
    QCOMPARE(table->rowCount(), 0); QVERIFY(message->text().contains("备用联系方式"));
    window.findChild<QLineEdit *>("backupField")->setText("family@example.com"); issue->click();
    QCOMPARE(table->rowCount(), 1); QCOMPARE(table->item(0, 0)->text(), QString("BOX001"));
    QCOMPARE(table->item(0, 5)->text(), QString("1001"));
    QVERIFY(window.findChild<QLineEdit *>("nameField")->text().isEmpty());
    QVERIFY(!window.findChild<QPushButton *>("saveFlightButton")->isEnabled());
    BoxManager restored(directory.path()); ///< 重启后的业务实例。
    QVERIFY(restored.load(error)); MainWindow reopened(restored); ///< 重启后的窗口。
    QVERIFY(!reopened.findChild<QTabWidget *>("tabs")->isEnabled());
    QCOMPARE(reopened.findChild<QTableWidget *>("passengerTable")->rowCount(), 0); // 未登录不展示乘客资料。
    reopened.findChild<QLineEdit *>("employeeField")->setText("1002");
    reopened.findChild<QLineEdit *>("passwordField")->setText("Box@123456");
    QTest::keyClick(reopened.findChild<QLineEdit *>("passwordField"), Qt::Key_Return);
    QCOMPARE(reopened.findChild<QTableWidget *>("passengerTable")->rowCount(), 1);
}

QTEST_MAIN(Day2Test)
#include "day2_test.moc"
