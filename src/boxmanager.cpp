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
    passenger.returned = false; passenger.returnedBy.clear(); passenger.deviceLeft = false;
    passenger.rating = -1; passenger.opinion.clear();
    FlightData candidate = flightData; ///< 加入新乘客的档案。
    candidate.passengers.append(passenger); return commit(candidate);
}

const FlightData &BoxManager::data() const {
    return flightData;
}

QString BoxManager::currentCrew() const { return crewId; }

QString BoxManager::directory() const { return dataDirectory; }
