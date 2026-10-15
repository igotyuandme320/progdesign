#include "surveydialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

SurveyDialog::SurveyDialog(BoxManager &manager, const QString &boxNumber, QWidget *parent)
    : QDialog(parent), manager(manager), boxNumber(boxNumber) {
    setObjectName("surveyDialog"); setWindowTitle("服务满意度评价"); resize(760, 520); setMinimumSize(600, 430);
    setWindowModality(Qt::ApplicationModal);
    // 乘客页不放姓名、手机号或管理导航，仅保留评价内容。
    setStyleSheet("QDialog { background: #f3f6fb; } QLabel { color: #24364b; font-size: 16px; } "
                  "QLabel#surveyTitle { font-size: 28px; color: #1e4f8e; font-weight: bold; } "
                  "QPlainTextEdit, QLineEdit { background: white; color: #24364b; border: 1px solid #cfdae7; border-radius: 6px; padding: 10px; } "
                  "QPushButton { background: white; color: #245ea7; padding: 12px; border: 1px solid #cfdae7; border-radius: 6px; } "
                  "QPushButton:disabled { color: #78879a; background: #e9eef5; }");
    auto *layout = new QVBoxLayout(this); ///< 乘客页面布局。
    layout->setContentsMargins(30, 26, 30, 26); layout->setSpacing(18);
    auto *title = new QLabel("请评价本次盒子服务"); ///< 不包含身份信息的标题。
    title->setObjectName("surveyTitle"); layout->addWidget(title);
    auto *description = new QLabel("可先填写意见，再点击评价按钮。每位乘客只能提交一次。\n也可在意见框外按数字键 1、2 或 0。"); ///< 操作说明。
    description->setWordWrap(true); layout->addWidget(description);
    opinionField = new QPlainTextEdit; opinionField->setObjectName("opinionField");
    opinionField->setPlaceholderText("意见与建议（选填，最多 2000 字符）"); layout->addWidget(opinionField, 1);
    auto *ratings = new QHBoxLayout; ///< 三种评价选择。
    const QVector<QPair<int, QString>> choices = {{1, "1 满意"}, {2, "2 不满意"}, {0, "0 不表态"}}; ///< 数值与按钮文字。
    for (const auto &choice : choices) { // choice：当前评价选项。
        auto *button = new QPushButton(choice.second); ///< 当前选项按钮。
        button->setObjectName("ratingButton" + QString::number(choice.first)); button->setAutoDefault(false);
        button->setMinimumHeight(52); ratings->addWidget(button); ratingButtons.append(button);
        connect(button, &QPushButton::clicked, this, [this, choice] { submit(choice.first); });
    }
    layout->addLayout(ratings);
    messageLabel = new QLabel("提交后请交由乘务员继续操作。"); messageLabel->setObjectName("surveyMessage");
    messageLabel->setWordWrap(true); layout->addWidget(messageLabel);
    auto *back = new QPushButton("乘务员返回"); ///< 需要工号和密码验证的返回入口。
    back->setObjectName("surveyBackButton"); back->setAutoDefault(false); layout->addWidget(back, 0, Qt::AlignRight);
    connect(back, &QPushButton::clicked, this, &SurveyDialog::reject);
}

void SurveyDialog::submit(int rating) {
    if (submitted) return;
    const QString error = manager.submitRating(boxNumber, rating, opinionField->toPlainText()); ///< 文件保存结果。
    if (!error.isEmpty()) { messageLabel->setText(error); return; }
    submitted = true; opinionField->setReadOnly(true);
    for (QPushButton *button : ratingButtons) // button：保存成功后禁用的评价按钮。
        button->setEnabled(false);
    messageLabel->setText("评价已保存，感谢您的反馈！请交由乘务员继续操作。");
}

void SurveyDialog::reject() {
    QDialog verification(this); ///< 返回前的乘务员验证窗口。
    verification.setObjectName("crewVerificationDialog"); verification.setWindowTitle("乘务员验证");
    auto *layout = new QFormLayout(&verification); ///< 验证表单。
    auto *employee = new QLineEdit; ///< 验证工号输入。
    employee->setObjectName("verifyEmployee"); employee->setMaxLength(200);
    auto *password = new QLineEdit; ///< 验证密码输入。
    password->setObjectName("verifyPassword"); password->setEchoMode(QLineEdit::Password); password->setMaxLength(200);
    auto *errorLabel = new QLabel; ///< 验证错误提示。
    errorLabel->setWordWrap(true); errorLabel->setObjectName("verificationError");
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); ///< 验证或留在乘客页。
    buttons->button(QDialogButtonBox::Ok)->setText("验证并返回"); buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    layout->addRow("工号", employee); layout->addRow("密码", password); layout->addRow(errorLabel); layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &verification, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &verification, [this, &verification, employee, password, errorLabel] {
        if (manager.login(employee->text(), password->text())) verification.accept();
        else { errorLabel->setText("工号或密码错误。"); password->clear(); password->setFocus(); }
    });
    // 点击返回、按 Esc 或点窗口关闭都会经过此验证；取消后继续留在评价页。
    if (verification.exec() == QDialog::Accepted) QDialog::reject();
}

void SurveyDialog::keyPressEvent(QKeyEvent *event) {
    if (!(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        if (event->key() == Qt::Key_1) { submit(1); return; }
        if (event->key() == Qt::Key_2) { submit(2); return; }
        if (event->key() == Qt::Key_0) { submit(0); return; }
    }
    QDialog::keyPressEvent(event);
}
