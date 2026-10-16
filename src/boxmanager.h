#pragma once
#include "models.h"
#include "datastore.h"
#include <QStringList>

/** 业务管理：登录、发放、回收、评价、抽奖和新建航班。 */
class BoxManager {

public:
    /** @brief 设置数据目录。 @param directory 保存 JSON 的目录。 @return 无。 */
    explicit BoxManager(const QString &directory);
    /** @brief 加载已有档案。 @param error 输出失败原因。 @return 是否成功。 */
    bool load(QString &error);
    /** @brief 验证登录或换岗。 @param employeeId 工号。 @param password 密码。 @return 是否验证成功。 */
    bool login(const QString &employeeId, const QString &password);
    /** @brief 保存航班。 @param number 航班号。 @param date 日期。 @param origin 出发地。 @param destination 目的地。 @return 空字符串为成功，否则为错误。 */
    QString setFlight(const QString &number, const QDate &date, const QString &origin, const QString &destination);
    /** @brief 校验并登记发放。 @param passenger 乘客输入信息。 @return 空字符串为成功，否则为错误。 */
    QString issue(Passenger passenger);
    /** @brief 按盒号或座位精确查询。 @param query 查询文字。 @param bySeat 为 true 时按座位查询。 @return 乘客下标，未登录或未找到为 -1。 */
    int findPassenger(const QString &query, bool bySeat = false) const;
    /** @brief 确认盒子归还。 @param boxNumber 盒号。 @param deviceLeft 是否发现遗留设备。 @return 空字符串为成功，否则为错误。 */
    QString returnBox(const QString &boxNumber, bool deviceLeft);
    /** @brief 保存联系备注。 @param boxNumber 盒号。 @param note 联系情况，最多 2000 字符。 @return 空字符串为成功，否则为错误。 */
    QString saveContactNote(const QString &boxNumber, const QString &note);
    /** @brief 确认遗留设备领取。 @param boxNumber 盒号。 @return 空字符串为成功，否则为错误。 */
    QString claimDevice(const QString &boxNumber);
    /** @brief 提交一次评价。 @param boxNumber 盒号。 @param rating 1 满意、2 不满意、0 不表态。 @param opinion 可选意见，最多 2000 字符。 @return 空字符串为成功，否则为错误。 */
    QString submitRating(const QString &boxNumber, int rating, const QString &opinion);
    /** @brief 计算当前航班三类评价人数。 @return 人数统计，未登录时为空。 */
    SurveyStats surveyStats() const;
    /** @brief 筛选已评价且未中奖的去重手机号。 @return 本轮可参加抽奖的号码。 */
    QStringList lotteryCandidates() const;
    /** @brief 随机抽取并先保存中奖记录。 @param count 抽奖人数。 @param phones 成功时输出中奖手机号，失败时为空。 @return 空字符串为成功，否则为错误。 */
    QString drawLottery(int count, QStringList &phones);
    /** @brief 旧航班处理完成后备份并新建航班。 @param number 新航班号。 @param date 日期。 @param origin 出发地。 @param destination 目的地。 @return 空字符串为成功，否则为错误。 */
    QString startNewFlight(const QString &number, const QDate &date, const QString &origin, const QString &destination);
    /** @brief 提供界面读取的数据接口。 @return 只读航班数据。 */
    const FlightData &data() const;
    /** @brief 获取当前登录人员。 @return 工号，未登录时为空。 */
    QString currentCrew() const;
    /** @brief 获取存储位置。 @return 数据目录的路径。 */
    QString directory() const;
private:
    /** @brief 先写文件再更新内存。 @param candidate 待保存数据。 @return 空字符串为成功，否则为错误。 */
    QString commit(const FlightData &candidate);
    FlightData flightData; ///< 统一管理的航班与乘客数据。
    QString dataDirectory; ///< 文件保存目录。
    DataStore store; ///< JSON 文件读写对象。
    QString crewId; ///< 当前登录的乘务员工号。
    bool loaded = false; ///< 是否安全完成数据加载。
};
