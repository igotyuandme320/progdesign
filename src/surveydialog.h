#pragma once
#include "boxmanager.h"
#include <QDialog>

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QKeyEvent;

/** 只显示评价内容的乘客窗口；返回管理界面必须验证乘务员。 */
class SurveyDialog : public QDialog {
public:
    /** @brief 创建一名乘客的评价页。 @param manager 当前业务对象。 @param boxNumber 待评价乘客的盒号，不展示在乘客页。 @param parent 所属管理窗口。 @return 无。 */
    SurveyDialog(BoxManager &manager, const QString &boxNumber, QWidget *parent = nullptr);
protected:
    /** @brief 拦截取消和关闭，要求乘务员验证。 @return 无。 */
    void reject() override;
    /** @brief 在非文本输入区域支持 1、2、0 评价。 @param event 键盘事件。 @return 无。 */
    void keyPressEvent(QKeyEvent *event) override;
private:
    /** @brief 保存评价，成功后禁止重复提交。 @param rating 1 满意、2 不满意、0 不表态。 @return 无。 */
    void submit(int rating);
    BoxManager &manager; ///< 共用的业务对象。
    QString boxNumber; ///< 本次评价对应的盒号。
    QPlainTextEdit *opinionField; ///< 乘客可选的意见输入。
    QLabel *messageLabel; ///< 提交结果或错误提示。
    QVector<QPushButton *> ratingButtons; ///< 三种评价按钮。
    bool submitted = false; ///< 本窗口是否已经成功提交。
};
