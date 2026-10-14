#include "boxmanager.h"
#include "validation.h"
#include <QDateTime>

BoxManager::BoxManager(const QString &directory) : dataDirectory(directory), store(directory) {}

bool BoxManager::load(QString &error) {
    FlightData candidate; ///< 待读取档案。
    loaded = store.load(candidate, error); crewId.clear();
    if (loaded) flightData = candidate;
    return loaded;
}

bool BoxManager::login(const QString &employeeId, const QString &password) {
    const QString id = employeeId.trimmed(); ///< 去掉首尾空格的工号。
    // 简单课程演示使用两个固定账号；失败的换岗不改变当前操作人。
    const QVector<CrewAccount> accounts = { ///< 演示账户表。
        {"1001", "乘务员一", "Box@123456"}, {"1002", "乘务员二", "Box@123456"}
    };
    if (!loaded) return false;
    for (const CrewAccount &account : accounts) // account：当前核对的账户。
        if (account.employeeId == id && account.password == password) { crewId = id; return true; }
    return false;
}

QString BoxManager::commit(const FlightData &candidate) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    QString error; ///< 文件保存结果。
    // 保存失败时不改变 flightData，避免界面显示未落盘的记录。
    if (!store.save(candidate, error)) return error;
    flightData = candidate; return {};
}

QString BoxManager::setFlight(const QString &number, const QDate &date, const QString &origin, const QString &destination) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    if (!flightData.passengers.isEmpty()) return "已有乘客，不能再更改本档案的航班。";
    FlightData candidate = flightData; ///< 待保存的航班。
    candidate.number = number.trimmed().toUpper(); candidate.date = date;
    candidate.origin = origin.trimmed(); candidate.destination = destination.trimmed();
    const QString error = validateFlight(candidate); ///< 航班校验结果。
    if (!error.isEmpty()) return error;
    return commit(candidate);
}

QString BoxManager::issue(Passenger passenger) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    if (flightData.number.isEmpty()) return "请先保存航班信息。";
    passenger = normalizePassenger(passenger);
    const QString error = validatePassenger(passenger); ///< 乘客校验结果。
    if (!error.isEmpty()) return error;
    for (const Passenger &item : flightData.passengers) { // item：已登记记录。
        if (item.boxNumber == passenger.boxNumber) return "该盒子已登记，不能重复发放。";
        if (item.documentNumber == passenger.documentNumber || item.seat == passenger.seat || item.boardingPass == passenger.boardingPass)
            return "该乘客、座位或登机牌已经登记。";
    }
    passenger.issuedBy = crewId; passenger.issuedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    passenger.returned = false; passenger.returnedBy.clear(); passenger.returnedAt.clear();
    passenger.deviceLeft = false; passenger.deviceClaimed = false;
    passenger.contactNote.clear(); passenger.contactedBy.clear(); passenger.contactedAt.clear();
    passenger.claimedBy.clear(); passenger.claimedAt.clear();
    passenger.rating = -1; passenger.opinion.clear();
    FlightData candidate = flightData; ///< 加入新乘客的档案。
    candidate.passengers.append(passenger); return commit(candidate);
}

const FlightData &BoxManager::data() const {
    return flightData;
}

int BoxManager::findPassenger(const QString &query, bool bySeat) const {
    if (!loaded || crewId.isEmpty()) return -1;
    const QString key = query.trimmed().toUpper(); ///< 清理后的查询编号。
    if (key.isEmpty()) return -1;
    for (int index = 0; index < flightData.passengers.size(); ++index) { // index：当前乘客的位置。
        const Passenger &item = flightData.passengers[index]; ///< 当前查找的记录。
        if ((bySeat ? item.seat : item.boxNumber) == key) return index;
    }
    return -1;
}

QString BoxManager::returnBox(const QString &boxNumber, bool deviceLeft) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    const int index = findPassenger(boxNumber); ///< 待回收记录的位置。
    if (index < 0) return "未找到该盒子。";
    if (flightData.passengers[index].returned) return "该盒子已经归还，不能重复回收。";
    FlightData candidate = flightData; ///< 更新回收状态后的候选档案。
    Passenger &item = candidate.passengers[index]; ///< 仅修改候选档案中的记录。
    // 回收与设备领取是两种状态：发现设备时先归还盒子，再处理设备。
    item.returned = true; item.deviceLeft = deviceLeft;
    item.returnedBy = crewId; item.returnedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    return commit(candidate);
}

QString BoxManager::saveContactNote(const QString &boxNumber, const QString &note) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    const int index = findPassenger(boxNumber); ///< 遗留设备所属记录的位置。
    if (index < 0) return "未找到该盒子。";
    const Passenger &saved = flightData.passengers[index]; ///< 已保存的设备状态。
    if (!saved.returned || !saved.deviceLeft) return "该盒子没有待处理的遗留设备。";
    if (saved.deviceClaimed) return "设备已领取，不能再修改联系备注。";
    const QString text = note.trimmed(); ///< 清理后的联系备注。
    if (text.isEmpty() || text.size() > 2000) return "请填写联系备注，最多 2000 个字符。";
    FlightData candidate = flightData; ///< 更新备注后的候选档案。
    Passenger &item = candidate.passengers[index]; ///< 待更新的联系信息。
    item.contactNote = text; item.contactedBy = crewId;
    item.contactedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    return commit(candidate);
}

QString BoxManager::claimDevice(const QString &boxNumber) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    const int index = findPassenger(boxNumber); ///< 待确认领取的记录位置。
    if (index < 0) return "未找到该盒子。";
    const Passenger &saved = flightData.passengers[index]; ///< 已保存的设备状态。
    if (!saved.returned || !saved.deviceLeft) return "该盒子没有待处理的遗留设备。";
    if (saved.deviceClaimed) return "设备已经领取，不能重复确认。";
    FlightData candidate = flightData; ///< 更新领取状态后的候选档案。
    Passenger &item = candidate.passengers[index]; ///< 待更新的领取信息。
    // 保留 deviceLeft，以便以后查看曾经遗留设备的记录。
    item.deviceClaimed = true; item.claimedBy = crewId;
    item.claimedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    return commit(candidate);
}

QString BoxManager::currentCrew() const { return crewId; }

QString BoxManager::directory() const { return dataDirectory; }
