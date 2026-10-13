#pragma once
#include "models.h"

/** 一个航班保存到一份 JSON 文件。 */
class DataStore {
public:
    /** @brief 设置存储目录。 @param directory JSON 所在目录。 @return 无。 */
    explicit DataStore(const QString &directory);
    /** @brief 加载文件；首次运行返回空档案。 @param data 输出档案。 @param error 输出错误。 @return 是否成功。 */
    bool load(FlightData &data, QString &error) const;
    /** @brief 安全提交文件。 @param data 待保存档案。 @param error 输出错误。 @return 是否成功。 */
    bool save(const FlightData &data, QString &error) const;
private:
    QString filePath; ///< records.json 的完整路径。
};
