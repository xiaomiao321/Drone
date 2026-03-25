# 无人机飞控系统状态说明

## 1. 系统状态总览

飞控系统共有 **4 种飞行状态**：

| 状态 | 英文 | 说明 |
|------|------|------|
| **IDLE** | 怠速/未解锁 | 系统上电默认状态，电机未启动 |
| **NORMAL** | 正常飞行 | 解锁成功，姿态自稳模式 |
| **FIX_HEIGHT** | 定高模式 | 气压计辅助定高飞行 |
| **FAIL** | 失控保护 | 遥控器失联，等待恢复 |

---

## 2. 状态切换流程图

```
                    ┌──────────────┐
                    │    上电      │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
              ┌─────│    IDLE      │◄────┐
              │     │  (怠速状态)  │      │
              │     └──────┬───────┘      │
              │            │              │
              │            │ 解锁成功      │ 失控恢复
              │            │ (长蜂鸣)      │
              │            ▼              │
              │     ┌──────────────┐      │
              │     │   NORMAL     │      │
              │     │ (正常飞行)   │      │
              │     └──────┬───────┘      │
              │            │              │
         失控 │            │ AUX2=0       │
              │            │ 进入定高      │
              │            │ (双蜂鸣)      │
              │            ▼              │
              │     ┌──────────────┐      │
              │     │ FIX_HEIGHT   │      │
              │     │  (定高模式)  │      │
              │     └──────┬───────┘      │
              │            │              │
              │            │ AUX2≠0       │
              │            │ 退出定高      │
              │            │ (单短蜂鸣)    │
              │            │              │
              │            ▼              │
              │         NORMAL            │
              │                           │
              └───────────────────────────┘
```

---

## 3. 状态切换条件详解

### 3.1 IDLE → NORMAL（解锁）

**解锁流程**（日本手 Mode 1）：

1. 油门推到最大 (≥1900) 保持 **1 秒**
2. 油门拉到最小 (≤1100) 保持 **1 秒**
3. 解锁完成，电机怠速旋转

**蜂鸣器反馈**：长蜂鸣 (500ms)

**代码位置**：`App_receive_data.c::App_process_unlock()`

---

### 3.2 NORMAL → FIX_HEIGHT（进入定高）

**触发条件**：
- AUX2 通道 = 0（遥控器未使用 AUX2，固定输出 0）
- 记录当前气压计高度作为目标高度

**蜂鸣器反馈**：双蜂鸣 (2×100ms)

**代码位置**：`App_receive_data.c::App_process_flight_state()`

---

### 3.3 FIX_HEIGHT → NORMAL（退出定高）

**触发条件**：
- AUX2 通道 ≠ 0

**蜂鸣器反馈**：单短蜂鸣 (100ms)

**代码位置**：`App_receive_data.c::App_process_flight_state()`

---

### 3.4 ANY → FAIL（失控保护）

**触发条件**：
- 遥控器连续失联超过阈值（默认 50 次，约 300ms）

**蜂鸣器反馈**：连续报警 (每 500ms 一次：100ms 开 +100ms 关)

**代码位置**：`App_receive_data.c::App_process_flight_state()`

---

### 3.5 FAIL → IDLE（失控恢复）

**触发条件**：
- 系统自动重置

**蜂鸣器反馈**：无

**代码位置**：`App_receive_data.c::App_process_flight_state()`

---

## 4. 各状态 LED 与蜂鸣器表现

### 4.1 LED 布局

```
       前 (Front)
        ↑
   LED1    LED2   ← 前两个灯 (连接状态)
     \    /
      \  /
      /  \
     /    \
   LED3    LED4   ← 后两个灯 (飞行状态)
```

- **LED1/LED2**（前）：遥控器连接状态指示
- **LED3/LED4**（后）：飞行状态指示

---

### 4.2 状态详细表现

#### IDLE（怠速状态）

| 组件 | 状态 | 说明 |
|------|------|------|
| **LED1/LED2** | 熄灭 | 遥控器未连接 |
| **LED1/LED2** | 常亮 | 遥控器连接成功 |
| **LED3/LED4** | 慢闪 (500ms 周期) | IDLE 状态指示 |
| **蜂鸣器** | 无声 | - |

**进入条件**：
- 系统上电
- 失控恢复

**退出条件**：
- 解锁成功 → NORMAL

---

#### NORMAL（正常飞行）

| 组件 | 状态 | 说明 |
|------|------|------|
| **LED1/LED2** | 常亮 | 遥控器连接成功 |
| **LED1/LED2** | 熄灭 | 遥控器失联（即将进入 FAIL） |
| **LED3/LED4** | 快闪 (200ms 周期) | NORMAL 状态指示 |
| **蜂鸣器** | 长鸣 (解锁时) | 解锁成功提示 |

**进入条件**：
- 解锁成功（从 IDLE）
- 退出定高模式（从 FIX_HEIGHT）

**退出条件**：
- 进入定高 → FIX_HEIGHT
- 遥控器失联 → FAIL

---

#### FIX_HEIGHT（定高模式）

| 组件 | 状态 | 说明 |
|------|------|------|
| **LED1/LED2** | 常亮 | 遥控器连接成功 |
| **LED3/LED4** | 常亮 | FIX_HEIGHT 状态指示 |
| **蜂鸣器** | 双蜂鸣 (进入时) | 进入定高提示 |
| **蜂鸣器** | 单短蜂鸣 (退出时) | 退出定高提示 |

**进入条件**：
- AUX2 = 0（从 NORMAL）

**退出条件**：
- AUX2 ≠ 0 → NORMAL
- 遥控器失联 → FAIL

**定高逻辑**：
- 进入时记录当前气压计高度 `fix_height`
- 气压计任务每 24ms 更新一次高度 PID

---

#### FAIL（失控保护）

| 组件 | 状态 | 说明 |
|------|------|------|
| **LED1/LED2** | 熄灭 | 遥控器失联 |
| **LED3/LED4** | 熄灭 | FAIL 状态指示 |
| **蜂鸣器** | 连续报警 | 每 500ms 一次 (100ms 开 +100ms 关) |

**进入条件**：
- 遥控器连续失联（从 NORMAL 或 FIX_HEIGHT）

**退出条件**：
- 系统重置 → IDLE

**失控处理**：
- 当前实现：状态重置为 IDLE
- 建议改进：添加自动降落逻辑

---

## 5. 状态指示汇总表

| 状态 | LED1/2 (前) | LED3/4 (后) | 蜂鸣器 |
|------|-------------|-------------|--------|
| **IDLE** | 连接时常亮 | 慢闪 (500ms) | 无声 |
| **NORMAL** | 常亮 | 快闪 (200ms) | 解锁时长鸣 |
| **FIX_HEIGHT** | 常亮 | 常亮 | 进出时提示音 |
| **FAIL** | 熄灭 | 熄灭 | 连续报警 |

---

## 6. 蜂鸣器 API 使用

### 6.1 驱动文件

- `interface/Int_buzzer.h`
- `interface/Int_buzzer.c`

### 6.2 可用函数

```c
// 初始化
void Int_buzzer_init(void);

// 基础控制
void Int_buzzer_on(void);      // 开启 (低电平)
void Int_buzzer_off(void);     // 关闭 (高电平)
void Int_buzzer_toggle(void);  // 翻转

// 预设模式
void Int_buzzer_short_beep(void);   // 短蜂鸣 (100ms)
void Int_buzzer_long_beep(void);    // 长蜂鸣 (500ms)
void Int_buzzer_double_beep(void);  // 双蜂鸣
void Int_buzzer_triple_beep(void);  // 三蜂鸣 (错误)
void Int_buzzer_alarm(void);        // 连续报警
```

### 6.3 使用示例

```c
// 解锁成功提示
Int_buzzer_long_beep();

// 进入定高模式
Int_buzzer_double_beep();

// 退出定高模式
Int_buzzer_short_beep();

// 失控报警 (在循环中调用)
if (flight_state == FAIL) {
    Int_buzzer_alarm();
    vTaskDelay(pdMS_TO_TICKS(500));
}
```

---

## 7. 相关代码文件

| 文件 | 功能 |
|------|------|
| `Application/App_receive_data.c` | 状态机逻辑、解锁逻辑 |
| `Application/App_freeRTOS_Task.c` | LED 任务、状态显示 |
| `interface/Int_buzzer.c/h` | 蜂鸣器驱动 |
| `interface/Int_led.c/h` | LED 驱动 |

---

## 8. 调试建议

### 8.1 查看状态变化日志

启用串口调试输出（UART2，115200 波特率）：

```
Flight: UNLOCKED
Flight: FIX_HEIGHT, alt=12 m
Flight: NORMAL
Flight: FAIL (RC lost)
Flight: IDLE (reset)
```

### 8.2 堆栈监控

在 `flight_task` 中已添加堆栈水位线监控：

```c
UBaseType_t stack_watermark = uxTaskGetStackHighWaterMark(NULL);
LOG_DEBUG("Flight stack watermark: %lu words", stack_watermark);
```

---

## 9. 版本信息

| 项目 | 值 |
|------|-----|
| 固件版本 | V1.0 |
| 更新日期 | 2026-02-21 |
| 适用硬件 | STM32F103C8T6 + MPU6050 + SPL06-001 + nRF24L01 |

