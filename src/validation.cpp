#include "validation.h"
#include <QDateTime>
#include <QRegularExpression>

Passenger normalizePassenger(Passenger passenger) {
    passenger.name = passenger.name.trimmed(); passenger.documentType = passenger.documentType.trimmed();
    passenger.documentNumber = passenger.documentNumber.trimmed().toUpper();
    passenger.boardingPass = passenger.boardingPass.trimmed().toUpper(); passenger.seat = passenger.seat.trimmed().toUpper();
    passenger.phoneModel = passenger.phoneModel.trimmed(); passenger.phone = passenger.phone.trimmed();
    passenger.backupContact = passenger.backupContact.trimmed(); passenger.boxNumber = passenger.boxNumber.trimmed().toUpper();
    return passenger;
}

QString validatePassenger(const Passenger &passenger) {
    const Passenger item = normalizePassenger(passenger); ///< 清理后的信息。
    const QVector<QPair<QString, QString>> fields = { ///< 字段值及中文名称。
        {item.name, "姓名"}, {item.documentType, "证件类型"}, {item.documentNumber, "证件号码"},
        {item.boardingPass, "登机牌"}, {item.seat, "座位"}, {item.phoneModel, "手机机型"},
        {item.phone, "手机号"}, {item.backupContact, "备用联系方式"}, {item.boxNumber, "盒子编号"}
    };
    for (const auto &field : fields) { // field：当前检查的必填项。
        if (field.first.isEmpty()) return field.second + "必须填写。";
        if (field.first.size() > 200) return field.second + "不能超过 200 个字符。";
    }
    if (!QRegularExpression("^1[3-9][0-9]{9}$").match(item.phone).hasMatch()) return "手机号应为大陆 11 位号码。";
    if (item.documentType == "身份证") {
        if (!QRegularExpression("^[1-9][0-9]{16}[0-9X]$").match(item.documentNumber).hasMatch()) return "身份证应为 18 位，末位可为 X。";
        const QDate birthday = QDate::fromString(item.documentNumber.mid(6, 8), "yyyyMMdd"); ///< 身份证出生日期。
        if (!birthday.isValid() || birthday > QDate::currentDate()) return "身份证出生日期不正确。";
        const int weights[] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2}; ///< 前 17 位的校验权重。
        const QString checks = "10X98765432"; ///< 加权和余数对应的末位字符。
        int sum = 0; ///< 前 17 位数字的加权和。
        // 用校验位检查明显的身份证输入错误。
        for (int index = 0; index < 17; ++index) // index：当前数字的位置。
            sum += item.documentNumber[index].digitValue() * weights[index];
        if (item.documentNumber[17] != checks[sum % 11]) return "身份证校验位不正确。";
    } else if (item.documentType == "护照" || item.documentType == "其他") {
        if (!QRegularExpression("^[A-Z0-9-]{3,40}$").match(item.documentNumber).hasMatch()) return "其他证件应为 3–40 位字母、数字或连字符。";
    } else return "证件类型不正确。";
    return {};
}

QString validateFlight(const FlightData &flight) {
    if (!flight.date.isValid() || flight.number.trimmed().isEmpty() || flight.origin.trimmed().isEmpty() || flight.destination.trimmed().isEmpty())
        return "请完整填写航班号、日期、出发地和目的地。";
    if (flight.number.size() > 200 || flight.origin.size() > 200 || flight.destination.size() > 200)
        return "航班信息不能超过 200 个字符。";
    return {};
}

/** @brief 检查操作责任信息。 @param crew 工号。 @param time 操作时间。 @return 工号与时间是否有效。 */
static bool validOperation(const QString &crew, const QString &time) {
    return (crew == "1001" || crew == "1002") && QDateTime::fromString(time, "yyyy-MM-dd HH:mm:ss").isValid();
}

QString validateRecovery(const Passenger &passenger) {
    // 未归还时不能有回收信息；没有遗留设备时不能有联系或领取信息。
    if (!passenger.returned && (!passenger.returnedBy.isEmpty() || !passenger.returnedAt.isEmpty() || passenger.deviceLeft))
        return "未归还记录含有回收信息。";
    if (passenger.returned && !validOperation(passenger.returnedBy, passenger.returnedAt))
        return "回收人工号或时间不正确。";
    if (!passenger.deviceLeft && (passenger.deviceClaimed || !passenger.contactNote.isEmpty() ||
        !passenger.contactedBy.isEmpty() || !passenger.contactedAt.isEmpty() || !passenger.claimedBy.isEmpty() || !passenger.claimedAt.isEmpty()))
        return "没有遗留设备的记录含有处理信息。";
    if (passenger.contactNote.size() > 2000) return "联系备注不能超过 2000 个字符。";
    if (passenger.contactNote.isEmpty()) {
        if (!passenger.contactedBy.isEmpty() || !passenger.contactedAt.isEmpty()) return "空备注含有联系人信息。";
    } else if (passenger.contactNote.trimmed().isEmpty() || !validOperation(passenger.contactedBy, passenger.contactedAt))
        return "联系备注或操作信息不正确。";
    if (passenger.deviceClaimed) {
        if (!validOperation(passenger.claimedBy, passenger.claimedAt)) return "领取操作人工号或时间不正确。";
    } else if (!passenger.claimedBy.isEmpty() || !passenger.claimedAt.isEmpty()) return "未领取记录含有领取信息。";
    return {};
}
