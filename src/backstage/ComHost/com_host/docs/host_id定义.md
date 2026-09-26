# host_id（com_host_id）定义

`com_host_id` 用于区分网络中的不同设备。实现见头文件 `protocol/com_host_id_def.h`（类型 `com_host_id_e` / `com_host_id_t`）。

## 网络模型与使用规则

- 仅支持**树状网络**，不支持环状网络；树以 `COM_HOST_ID_MAIN_CONTROLLER`（`0x01`）为根。
- `COM_HOST_ID_MAIN_CONTROLLER` 必须为**产品主控**设备 ID。
- 为兼容老产品、支持多配件与热插拔：**配件设备 ID 由配件向主控申请分配**；未分配前使用 `COM_HOST_ID_AUX_DEVICE_NONE`（`0x10`）。
- `COM_HOST_ID_AUX_DEVICE_NONE` **仅用于**配件向主控申请分配 ID 时，**不允许**在网络中长期存在。
- 配件离开主控网络后须**清除已分配 ID**，下次连接时重新申请。
- 客户端正常情况下**不直接访问**子控与配件，只能通过主控以**产品**形式访问。
- 无产品时客户端直连配件，使用 `COM_HOST_ID_AUX_DEVICE_NONE` 访问配件。

## 取值表

| 符号名 | 数值 | 说明 |
|--------|------|------|
| `COM_HOST_ID_NONE` | `0x00` | 未定义 host |
| **主控设备（0x01–0x0F，共 15 个）** | | |
| `COM_HOST_ID_MAIN_CONTROLLER` | `0x01` | 主控 |
| `COM_HOST_ID_SUB_CONTROLLER_1` | `0x02` | 子控 1 |
| `COM_HOST_ID_SUB_CONTROLLER_2` | `0x03` | 子控 2 |
| `COM_HOST_ID_SUB_CONTROLLER_3` | `0x04` | 子控 3 |
| **配件设备（0x10–0x17，由主控分配，共 8 个）** | | |
| `COM_HOST_ID_AUX_DEVICE_NONE` | `0x10` | 未分配 ID 的配件（申请用） |
| `COM_HOST_ID_AUX_DEVICE_1` | `0x11` | 配件 1 |
| `COM_HOST_ID_AUX_DEVICE_2` | `0x12` | 配件 2 |
| `COM_HOST_ID_AUX_DEVICE_3` | `0x13` | 配件 3 |
| `COM_HOST_ID_AUX_DEVICE_4` | `0x14` | 配件 4 |
| `COM_HOST_ID_AUX_DEVICE_5` | `0x15` | 配件 5 |
| **客户端（0xE0–0xEF，共 16 个）** | | |
| `COM_HOST_ID_MOBOILE_APP` | `0xE0` | 手机 APP |
| `COM_HOST_ID_PC_CLIENT` | `0xE1` | PC 客户端 |
| `COM_HOST_ID_NETWORK_BACKEND` | `0xE2` | 网络后台 |
| **开发者相关（0xF0–0xFE，共 15 个）** | | |
| `COM_HOST_ID_ADMIN` | `0xF1` | 管理员端 |
| `COM_HOST_ID_DEVELOPER` | `0xF1` | 开发者端（与 `ADMIN` 同值，见头文件） |
| `COM_HOST_ID_TESTER` | `0xF2` | 测试者端 |
| `COM_HOST_ID_FACTORY` | `0xF3` | 工厂端 |

## 与代码同步

枚举或注释变更时，请同时更新本文件与 `protocol/com_host_id_def.h`，避免文档与实现不一致。
