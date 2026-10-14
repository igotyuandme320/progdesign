#pragma once
#include "boxmanager.h"
#include <QMainWindow>

class QComboBox;
class QCheckBox;
class QCloseEvent;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QTableWidget;
class QTabWidget;

/** 登录、登记、回收与遗留设备处理窗口；评价及抽奖仍保留原型。 */
class MainWindow : public QMainWindow {
public:
    /** @brief 创建界面并连接按钮。 @param manager 已加载的业务对象。 @return 无。 */
    explicit MainWindow(BoxManager &manager);
protected:
    /** @brief 关闭前处理未保存的联系备注。 @param event 关闭事件。 @return 无。 */
    void closeEvent(QCloseEvent *event) override;
private:
    /** @brief 从业务数据刷新界面和登录状态。 @return 无。 */
    void refresh();
    /** @brief 航班输入与已保存数据一致时才允许发放。 @return 无。 */
    void updateIssueState();
    /** @brief 读取表单并登记。 @return 无。 */
    void issuePassenger();
    /** @brief 创建回收页面并连接操作。 @return 可滚动的回收页面。 */
    QWidget *makeRecoveryPage();
    /** @brief 根据查询条件选中乘客。 @return 无。 */
    void searchPassenger();
    /** @brief 刷新回收详情、数量和按钮状态。 @return 无。 */
    void refreshRecovery();
    /** @brief 判断备注是否有未保存修改。 @return 是否存在联系备注草稿。 */
    bool hasContactDraft() const;
    /** @brief 切换记录或关闭前选择保存、放弃或取消。 @return 是否允许继续。 */
    bool resolveContactDraft();
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
    QComboBox *queryType; ///< 按盒号或座位查询的选择。
    QLineEdit *queryField; ///< 查询编号输入。
    QLabel *recoverySummary; ///< 已发放、已回收、待回收和待领取数量。
    QLabel *recoveryResult; ///< 查询到的乘客及处理详情。
    QCheckBox *deviceLeftCheck; ///< 回收时标记是否发现遗留设备。
    QPushButton *returnButton; ///< 确认归还按钮。
    QPlainTextEdit *contactField; ///< 联系乘客的备注输入。
    QPushButton *saveContactButton; ///< 保存联系备注按钮。
    QPushButton *claimButton; ///< 确认设备领取按钮。
    QString selectedBox; ///< 当前查询到的盒号，查询条件改变时清空。
    QString contactBox; ///< 当前备注所属盒号，查询条件变化时仍用于保留草稿。
};
