#include "datastore.h"
#include "validation.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QUuid>

DataStore::DataStore(const QString &directory) : filePath(QDir(directory).filePath("records.json")) {}

bool DataStore::load(FlightData &data, QString &error) const {
    error.clear();
    QFile file(filePath); ///< 待读取的数据文件。
    if (!file.exists()) { data = FlightData{}; return true; }
    if (!file.open(QIODevice::ReadOnly)) { error = "无法读取数据文件：" + file.errorString(); return false; }
    QJsonParseError parseError; ///< JSON 格式错误信息。
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError); ///< 解码后的文件。
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = "数据文件损坏，请检查 records.json。"; return false;
    }
    const QJsonObject root = document.object(); ///< 文件根对象。
    const bool legacy = root["version"] == QJsonValue(1); ///< 是否为第二天的旧格式。
    const bool day4 = root["version"] == QJsonValue(3); ///< 是否包含评价及中奖记录的新格式。
    if ((!legacy && !day4 && root["version"] != QJsonValue(2)) || !root["passengers"].isArray()) {
        error = "数据文件版本或结构不正确。"; return false;
    }
    FlightData candidate; ///< 全部校验成功后才交给业务层的档案。
    candidate.number = root["number"].toString().trimmed().toUpper();
    candidate.date = QDate::fromString(root["date"].toString(), Qt::ISODate);
    candidate.origin = root["origin"].toString().trimmed(); candidate.destination = root["destination"].toString().trimmed();
    if (!validateFlight(candidate).isEmpty()) { error = "文件中的航班信息不完整。"; return false; }
    QSet<QString> boxes; ///< 已加载的盒号。
    QSet<QString> documents; ///< 已加载的证件号。
    QSet<QString> seats; ///< 已加载的座位号。
    QSet<QString> passes; ///< 已加载的登机牌编号。
    for (const QJsonValue &value : root["passengers"].toArray()) { // value：文件中的一条乘客记录。
        const QJsonObject object = value.toObject(); ///< 当前记录。
        Passenger item; ///< 当前乘客。
        item.name = object["name"].toString(); item.documentType = object["documentType"].toString();
        item.documentNumber = object["documentNumber"].toString(); item.boardingPass = object["boardingPass"].toString();
        item.seat = object["seat"].toString(); item.phoneModel = object["phoneModel"].toString();
        item.phone = object["phone"].toString(); item.backupContact = object["backupContact"].toString();
        item.boxNumber = object["boxNumber"].toString(); item.issuedBy = object["issuedBy"].toString();
        item.issuedAt = object["issuedAt"].toString(); item = normalizePassenger(item);
        // 旧档案尚未回收，采用默认值；新档案必须完整保存回收及处理字段。
        if (!legacy) {
            if (!object["returned"].isBool() || !object["deviceLeft"].isBool() || !object["deviceClaimed"].isBool()) {
                error = "文件中的回收状态不正确。"; return false;
            }
            for (const QString &field : {"returnedBy", "returnedAt", "contactNote", "contactedBy", "contactedAt", "claimedBy", "claimedAt"}) { // field：必须为文本的处理字段。
                if (!object[field].isString()) { error = "文件中的处理信息不完整。"; return false; }
            }
            item.returned = object["returned"].toBool(); item.returnedBy = object["returnedBy"].toString();
            item.returnedAt = object["returnedAt"].toString(); item.deviceLeft = object["deviceLeft"].toBool();
            item.deviceClaimed = object["deviceClaimed"].toBool(); item.contactNote = object["contactNote"].toString();
            item.contactedBy = object["contactedBy"].toString(); item.contactedAt = object["contactedAt"].toString();
            item.claimedBy = object["claimedBy"].toString(); item.claimedAt = object["claimedAt"].toString();
        }
        if (day4) {
            if (!object["rating"].isDouble() || object["rating"].toDouble() != object["rating"].toInt() || !object["opinion"].isString()) {
                error = "文件中的评价格式不正确。"; return false;
            }
            item.rating = object["rating"].toInt(); item.opinion = object["opinion"].toString();
            if (item.rating < -1 || item.rating > 2 || item.opinion.size() > 2000 || (item.rating == -1 && !item.opinion.isEmpty())) {
                error = "文件中的评价内容不正确。"; return false;
            }
        }
        // 拒绝不完整及重复记录，避免损坏文件被覆盖成新档案。
        if (!validatePassenger(item).isEmpty() || !validateRecovery(item).isEmpty() || (item.issuedBy != "1001" && item.issuedBy != "1002") ||
            !QDateTime::fromString(item.issuedAt, "yyyy-MM-dd HH:mm:ss").isValid() ||
            boxes.contains(item.boxNumber) || documents.contains(item.documentNumber) || seats.contains(item.seat) || passes.contains(item.boardingPass)) {
            error = "文件中存在无效或重复的乘客记录。"; return false;
        }
        boxes.insert(item.boxNumber); documents.insert(item.documentNumber); seats.insert(item.seat); passes.insert(item.boardingPass);
        candidate.passengers.append(item);
    }
    if (day4) {
        if (!root["winners"].isArray()) { error = "文件中的中奖列表不正确。"; return false; }
        QSet<QString> eligible; ///< 已提交评价的手机号集合。
        QSet<QString> drawn; ///< 防止同一手机号重复中奖。
        for (const Passenger &item : candidate.passengers) // item：已读取的乘客记录。
            if (item.rating >= 0) eligible.insert(item.phone);
        for (const QJsonValue &value : root["winners"].toArray()) { // value：一条中奖记录。
            const QJsonObject object = value.toObject(); ///< 当前中奖 JSON 对象。
            LotteryWinner winner{object["phone"].toString(), object["drawnBy"].toString(), object["drawnAt"].toString()}; ///< 待校验的中奖记录。
            if (!eligible.contains(winner.phone) || drawn.contains(winner.phone) ||
                (winner.drawnBy != "1001" && winner.drawnBy != "1002") ||
                !QDateTime::fromString(winner.drawnAt, "yyyy-MM-dd HH:mm:ss").isValid()) {
                error = "文件中的中奖号码或操作信息不正确。"; return false;
            }
            drawn.insert(winner.phone); candidate.winners.append(winner);
        }
    }
    data = candidate; return true;
}

bool DataStore::save(const FlightData &data, QString &error) const {
    error.clear();
    QJsonArray passengers; ///< 待序列化的乘客数组。
    for (const Passenger &item : data.passengers) { // item：一条发放记录。
        passengers.append(QJsonObject{
            {"name", item.name}, {"documentType", item.documentType}, {"documentNumber", item.documentNumber},
            {"boardingPass", item.boardingPass}, {"seat", item.seat}, {"phoneModel", item.phoneModel},
            {"phone", item.phone}, {"backupContact", item.backupContact}, {"boxNumber", item.boxNumber},
            {"issuedBy", item.issuedBy}, {"issuedAt", item.issuedAt},
            {"returned", item.returned}, {"returnedBy", item.returnedBy}, {"returnedAt", item.returnedAt},
            {"deviceLeft", item.deviceLeft}, {"deviceClaimed", item.deviceClaimed},
            {"contactNote", item.contactNote}, {"contactedBy", item.contactedBy}, {"contactedAt", item.contactedAt},
            {"claimedBy", item.claimedBy}, {"claimedAt", item.claimedAt},
            {"rating", item.rating}, {"opinion", item.opinion}
        });
    }
    QJsonArray winners; ///< 待保存的中奖记录数组。
    for (const LotteryWinner &winner : data.winners) // winner：一个已中奖手机号。
        winners.append(QJsonObject{{"phone", winner.phone}, {"drawnBy", winner.drawnBy}, {"drawnAt", winner.drawnAt}});
    const QJsonObject root = { ///< 带版本号的完整档案。
        {"version", 3}, {"number", data.number}, {"date", data.date.toString(Qt::ISODate)},
        {"origin", data.origin}, {"destination", data.destination}, {"passengers", passengers}, {"winners", winners}
    };
    if (!QDir().mkpath(QFileInfo(filePath).absolutePath())) { error = "无法创建数据目录。"; return false; }
    QSaveFile file(filePath); ///< 先写临时文件，再替换原文件。
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented); ///< UTF-8 JSON 内容。
    // 任一步失败均返回错误；原文件不会被部分写入覆盖。
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        error = "保存失败：" + file.errorString(); return false;
    }
    return true;
}

bool DataStore::archive(const FlightData &data, QString &error) const {
    const QString folder = QFileInfo(filePath).absolutePath() + "/history/" +
        data.date.toString("yyyyMMdd") + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces); ///< 不覆盖旧备份的唯一目录。
    DataStore backup(folder); ///< 复用同一个 JSON 保存过程。
    if (!backup.save(data, error)) { error = "旧航班备份失败：" + error; return false; }
    return true;
}
