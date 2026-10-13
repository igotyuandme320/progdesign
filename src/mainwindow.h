#pragma once
#include "boxmanager.h"
#include <QMainWindow>

class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTabWidget;

/** 第二天窗口：登录、保存航班及登记发放，其他页面保留原型。 */
class MainWindow : public QMainWindow {
public:
    /** @brief 创建界面并连接按钮。 @param manager 已加载的业务对象。 @return 无。 */
    explicit MainWindow(BoxManager &manager);
private:
    /** @brief 从业务数据刷新界面和登录状态。 @return 无。 */
    void refresh();
    /** @brief 航班输入与已保存数据一致时才允许发放。 @return 无。 */
    void updateIssueState();
    /** @brief 读取表单并登记。 @return 无。 */
    void issuePassenger();
    /** @brief 显示操作结果。 @param error 错误，为空表示成功。 @param success 成功提示。 @return 是否成功。 */
    bool showResult(const QString &error, const QString &success);
    BoxManager &manager; ///< 窗口共享的业务对象。
    QLineEdit *employeeField; ///< 工号输入。
    QLineEdit *passwordField; ///< 密码输入。
    QLabel *crewLabel; ///< 当前登录工号。
    QLabel *messageLabel; ///< 窗口底部操作反馈。
    QWidget *flightSection; ///< 航班信息区域。
    QLineEdit *numberField; ///< 航班号输入。
    QDateEdit *dateField; ///< 航班日期输入。
    QLineEdit *originField; ///< 出发地输入。
    QLineEdit *destinationField; ///< 目的地输入。
    QPushButton *saveFlightButton; ///< 航班保存按钮。
    QTabWidget *tabs; ///< 四个模块的标签页。
    QComboBox *documentType; ///< 证件类型选择。
    QLineEdit *nameField; ///< 乘客姓名。
    QLineEdit *documentField; ///< 证件号码。
    QLineEdit *boardingField; ///< 登机牌编号。
    QLineEdit *seatField; ///< 座位号。
    QLineEdit *phoneModelField; ///< 手机机型。
    QLineEdit *phoneField; ///< 手机号码。
    QLineEdit *backupField; ///< 必填的备用联系方式。
    QLineEdit *boxField; ///< 盒子编号。
    QPushButton *issueButton; ///< 登记发放按钮。
    QTableWidget *passengerTable; ///< 已保存的发放记录表。
};
