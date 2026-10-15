#include "boxmanager.h"
#include "validation.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QSet>

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

QString BoxManager::submitRating(const QString &boxNumber, int rating, const QString &opinion) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    const int index = findPassenger(boxNumber); ///< 本次评价的乘客位置。
    if (index < 0) return "未找到该乘客。";
    if (flightData.passengers[index].rating != -1) return "该乘客已经评价，不能重复提交。";
    if (rating < 0 || rating > 2) return "评价只能选择 1、2 或 0。";
    const QString text = opinion.trimmed(); ///< 去掉首尾空白后的意见。
    if (text.size() > 2000) return "意见不能超过 2000 个字符。";
    FlightData candidate = flightData; ///< 只在保存成功后正式采用的评价。
    candidate.passengers[index].rating = rating; candidate.passengers[index].opinion = text;
    return commit(candidate);
}

SurveyStats BoxManager::surveyStats() const {
    SurveyStats stats; ///< 实时计算的人数。
    if (!loaded || crewId.isEmpty()) return stats;
    stats.total = int(flightData.passengers.size());
    for (const Passenger &item : flightData.passengers) { // item：当前统计的乘客。
        if (item.rating == 1) ++stats.satisfied;
        else if (item.rating == 2) ++stats.dissatisfied;
        else if (item.rating == 0) ++stats.neutral;
    }
    return stats;
}

QStringList BoxManager::lotteryCandidates() const {
    QStringList phones; ///< 按登记顺序整理的可抽奖手机号。
    if (!loaded || crewId.isEmpty()) return phones;
    QSet<QString> excluded; ///< 已中奖或本轮已经加入的手机号。
    for (const LotteryWinner &winner : flightData.winners) // winner：已经中奖的记录。
        excluded.insert(winner.phone);
    // 0 也是已提交评价；同一手机号即使登记多次也只获得一次机会。
    for (const Passenger &item : flightData.passengers) { // item：候选乘客记录。
        if (item.rating >= 0 && !excluded.contains(item.phone)) {
            phones.append(item.phone); excluded.insert(item.phone);
        }
    }
    return phones;
}

QString BoxManager::drawLottery(int count, QStringList &phones) {
    phones.clear();
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    QStringList candidates = lotteryCandidates(); ///< 本轮候选号码，随后进行局部洗牌。
    if (count < 1) return "抽奖人数至少为 1。";
    if (count > candidates.size()) return QString("候选不足：当前只有 %1 个未中奖的已评价手机号。").arg(candidates.size());
    FlightData candidate = flightData; ///< 加入新中奖记录后的航班副本。
    const QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"); ///< 同一轮的抽奖时间。
    // Fisher-Yates 洗牌：每一步从剩余号码中等概率选一个，天然不重复。
    for (int index = 0; index < count; ++index) { // index：本轮第几个中奖名额。
        const int chosen = QRandomGenerator::global()->bounded(index, int(candidates.size())); ///< 随机选中的位置。
        candidates.swapItemsAt(index, chosen);
        candidate.winners.append({candidates[index], crewId, time});
    }
    const QString error = commit(candidate); ///< 写入失败时不对外展示中奖号码。
    if (!error.isEmpty()) return error;
    phones = candidates.mid(0, count); return {};
}

QString BoxManager::startNewFlight(const QString &number, const QDate &date, const QString &origin, const QString &destination) {
    if (!loaded || crewId.isEmpty()) return "请先登录。";
    for (const Passenger &item : flightData.passengers) { // item：旧航班尚未完成的记录。
        if (!item.returned) return "还有盒子未归还，请先完成回收。";
        if (item.deviceLeft && !item.deviceClaimed) return "还有遗留设备未领取，请先完成处理。";
    }
    FlightData candidate; ///< 不携带旧乘客和中奖号码的新航班。
    candidate.number = number.trimmed().toUpper(); candidate.date = date;
    candidate.origin = origin.trimmed(); candidate.destination = destination.trimmed();
    const QString validation = validateFlight(candidate); ///< 新航班必填项校验结果。
    if (!validation.isEmpty()) return validation;
    if (candidate.number == flightData.number && candidate.date == flightData.date) return "新航班的航班号和日期不能都与当前相同。";
    QString error; ///< 旧航班备份失败原因。
    // 先备份再替换；任一步失败都不清空原来的业务数据。
    if (!flightData.number.isEmpty() && !store.archive(flightData, error)) return error;
    return commit(candidate);
}
