#pragma once
#include <QDate>
#include <QString>
#include <QVector>

/** 乘务员账户初稿；登录功能在第二天接入。 */
struct CrewAccount {
    QString employeeId; ///< 工号。
    QString name; ///< 乘务员姓名。
    QString password; ///< 课程演示密码。
};

/** 乘客记录初稿，供后续发放、回收和评价使用。 */
struct Passenger {
    QString name; ///< 乘客姓名。
    QString documentType; ///< 证件类型。
    QString documentNumber; ///< 证件号码。
    QString boardingPass; ///< 登机牌编号。
    QString seat; ///< 座位号。
    QString phoneModel; ///< 手机机型。
    QString phone; ///< 手机号码。
    QString backupContact; ///< 备用联系方式，后续登记时必填。
    QString boxNumber; ///< 盒子编号。
    QString issuedBy; ///< 发放人工号。
    QString returnedBy; ///< 回收人工号。
    bool returned = false; ///< 是否已归还。
    bool deviceLeft = false; ///< 是否遗留设备。
    int rating = -1; ///< -1 未评价，1 满意，2 不满意，0 不表态。
    QString opinion; ///< 意见与建议。
};

/** 一个航班的基础数据。 */
struct FlightData {
    QString number; ///< 航班号。
    QDate date = QDate::currentDate(); ///< 航班日期。
    QString origin; ///< 出发地。
    QString destination; ///< 目的地。
    QVector<Passenger> passengers; ///< 该航班的乘客列表。
};
