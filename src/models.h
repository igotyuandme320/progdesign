#pragma once
#include <QDate>
#include <QMetaType>
#include <QString>
#include <QVector>

/** 课程演示用的乘务员账户。 */
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
    QString backupContact; ///< 备用联系方式，登记时必填。
    QString boxNumber; ///< 盒子编号。
    QString issuedBy; ///< 发放人工号。
    QString issuedAt; ///< 发放时间，格式为 yyyy-MM-dd HH:mm:ss。
    QString returnedBy; ///< 回收人工号。
    QString returnedAt; ///< 回收时间，格式为 yyyy-MM-dd HH:mm:ss。
    bool returned = false; ///< 是否已归还。
    bool deviceLeft = false; ///< 回收时是否发现遗留设备，领取后仍保留此记录。
    bool deviceClaimed = false; ///< 遗留设备是否已领取。
    QString contactNote; ///< 联系乘客的备注。
    QString contactedBy; ///< 最近保存联系备注的乘务员工号。
    QString contactedAt; ///< 最近保存联系备注的时间。
    QString claimedBy; ///< 确认设备领取的乘务员工号。
    QString claimedAt; ///< 确认设备领取的时间。
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

Q_DECLARE_METATYPE(CrewAccount)
Q_DECLARE_METATYPE(Passenger)
Q_DECLARE_METATYPE(FlightData)
