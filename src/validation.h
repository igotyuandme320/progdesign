#pragma once
#include "models.h"

/** @brief 清理输入中的空白并统一编号大小写。 @param passenger 输入记录。 @return 清理后的副本。 */
Passenger normalizePassenger(Passenger passenger);
/** @brief 检查必填项、手机与证件。 @param passenger 输入记录。 @return 空字符串表示通过，否则为错误提示。 */
QString validatePassenger(const Passenger &passenger);
/** @brief 检查航班必填信息。 @param flight 航班数据。 @return 空字符串表示通过，否则为错误提示。 */
QString validateFlight(const FlightData &flight);
