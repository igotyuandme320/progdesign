#pragma once
#include "models.h"

/** 业务管理类骨架；后续逐日增加登录、登记和文件操作。 */
class BoxManager {
public:
    /** @brief 初始化空航班数据。 @return 无。 */
    BoxManager();
    /** @brief 提供界面读取的数据接口。 @return 只读航班数据。 */
    const FlightData &data() const;
private:
    FlightData flightData; ///< 统一管理的航班与乘客数据。
};
