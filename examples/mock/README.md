# 模拟航班数据

全部为虚构演示信息，JSON 使用当前 version 3 格式。每个子目录均包含程序要求的 records.json。

| 目录 | 场景 |
| --- | --- |
| 01-empty | 已设置航班，无乘客，可演示登记发放及零人数统计 |
| 02-issued | 8 名乘客，全部未回收、未评价，可演示完整操作流程 |
| 03-mixed | 12 名乘客，3 名待回收、2 台设备待领取、1 台设备已领取；三类评价各 3 人、未评价 3 人；包含重复手机号及 1 个已中奖号码，剩余抽奖候选 7 个 |
| 04-completed | 12 名乘客，盒子全部归还、遗留设备全部领取，可演示备份和新建航班 |

建议复制到独立目录运行，保留原始样例。在项目根目录执行：

```sh
mkdir -p .local-data/mock-demo
cp examples/mock/03-mixed/records.json .local-data/mock-demo/records.json
./build/MagicBox.app/Contents/MacOS/MagicBox --data-dir .local-data/mock-demo
```

重复复制会重置演示目录。登录工号为 1001 或 1002，密码均为 Box@123456。
