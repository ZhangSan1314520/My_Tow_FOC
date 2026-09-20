/**
 * ============================================================
 * uORB (micro Object Request Broker) — 极简发布/订阅中间件
 * ============================================================
 *
 * 设计目标: 在 FreeRTOS 上实现类似 PX4 uORB 的核心语义——
 *   多个发布者可向同一 Topic 写数据, 多个订阅者可独立读取,
 *   发布者与订阅者互相不知道对方的存在, 只通过 Topic 名字匹配。
 *
 * 与 FreeRTOS 消息队列的本质区别:
 *   - 消息队列 = 点对点 FIFO 管道 (N:1), 每条消息都被消费掉
 *   - uORB     = 状态快照 + 多订阅者共享 (N:M), 保留最新值, 不消费
 *
 * 典型用法:
 *   ── 发布端 ──
 *   uorb_handle_t h = uorb_advertise("motor_state", sizeof(struct motor_telemetry));
 *   uorb_publish(h, &my_data);   // 每次更新数据调用
 *
 *   ── 订阅端 ──
 *   uorb_handle_t h = uorb_subscribe("motor_state");
 *   struct motor_telemetry buf;
 *   uorb_copy(h, &buf);          // 阻塞等待新数据并拷贝
 *   // 或 uorb_check(h)          // 非阻塞检查是否有新数据
 * ============================================================
 */

#pragma once
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdbool.h>
#include <string.h>

/* ── 配置常量 ── */
#define UORB_TOPIC_NAME_LEN  16   // Topic 名称最大长度 (15字符 + '\0')
#define UORB_MAX_TOPICS      10   // 最多同时存在的 Topic 数量
#define UORB_MAX_SUBSCRIBERS 5    // 每个 Topic 最多订阅者数

/* ── Topic 控制块 ──
 * 每个 Topic 在全局数组中占一个槽位, 包含:
 *   - 名字 (用于发布/订阅匹配)
 *   - 数据区指针 (发布者分配, 订阅者读取)
 *   - 代际号 (generation): 每次 publish 自增, 订阅者据此判断有没有新数据
 *   - 订阅者列表: 记录每个订阅任务看到了第几代数据
 */
typedef struct {
    char     name[UORB_TOPIC_NAME_LEN];    // Topic 名字, 如 "motor_state"
    void    *data;                          // 指向数据副本的指针 (大小为 data_size)
    uint32_t data_size;                     // 数据块大小 (发布者 advertise 时指定)
    uint32_t generation;                    // 数据更新代际号: 每次 publish 自增 1
    SemaphoreHandle_t mutex;                // 保护 data 的互斥锁 (发布者写 vs 订阅者读)
    struct {
        TaskHandle_t task;                  // 订阅者任务句柄 (预留)
        uint32_t    last_seen_gen;          // 该订阅者上一次读到的代际号
    } subscribers[UORB_MAX_SUBSCRIBERS];
    uint8_t sub_count;                      // 当前订阅者数量
    bool    valid;                          // 该槽位是否在使用中
} uorb_topic_t;

/* ── 句柄类型 ──
 * 发布者拿到的 handle = topics 数组下标
 * 订阅者拿到的 handle = 低 16 位 topics 下标 | 高 16 位 subscriber 槽位号
 * 负数 (-1) 表示无效句柄
 */
typedef int uorb_handle_t;

/* ── extern "C" 保护 ──
 * 本文件会被 C++ 源文件 (#include "uorb.h") 引用,
 * 必须告诉 C++ 编译器这些函数是 C 链接约定的,
 * 否则链接器会因 C++ name mangling 找不到符号
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  发布端: 公告一个 Topic (注册为发布者)
 * @param  name       Topic 名字, 如 "motor_state"
 * @param  data_size  每次发布的数据块字节数, 如 sizeof(struct motor_telemetry)
 * @return >=0 有效句柄, -1 失败 (Topic 已满或已有人公告)
 *
 * 注意: 同一个 Topic 名字只能有一个发布者调用 advertise,
 *       其他任务想发布同一个 Topic 会返回 -1。
 */
uorb_handle_t uorb_advertise(const char *name, uint32_t data_size);

/**
 * @brief  发布端: 向 Topic 写入新数据
 * @param  handle  advertise 返回的句柄
 * @param  data    指向数据块的指针, 大小与 advertise 时一致
 *
 * 内部操作: 拷贝数据 → generation 自增 1 → 所有订阅者下次 check/copy 能感知到
 * 注意: 不能在 ISR 中调用 (内部用了信号量)
 */
void          uorb_publish(uorb_handle_t handle, const void *data);

/**
 * @brief  订阅端: 订阅一个 Topic
 * @param  name  Topic 名字 (与 publish 端的名字一致)
 * @return >=0 有效句柄, -1 失败 (Topic 不存在或订阅者已满)
 *
 * 订阅时记录当前 generation, 之后 check/copy 只在 generation 变化时返回 true。
 * 同一个任务可以订阅多个不同的 Topic, 同一个 Topic 也可以被多个任务订阅。
 */
uorb_handle_t uorb_subscribe(const char *name);

/**
 * @brief  订阅端: 非阻塞检查是否有新数据
 * @param  handle  subscribe 返回的句柄
 * @return true 有新数据 (generation 变了), false 没有
 *
 * 与 uorb_copy 的区别: check 立刻返回不等待, copy 会阻塞到有数据为止
 */
bool          uorb_check(uorb_handle_t handle);

/**
 * @brief  订阅端: 阻塞读取最新数据
 * @param  handle  subscribe 返回的句柄
 * @param  buf     接收数据的缓冲区指针
 * @return true 成功, 目前总是 true
 *
 * 内部会死等直到 generation 变化, 然后把数据 memcpy 到 buf 里。
 * 注意: buf 大小必须和 advertise 时 data_size 一致, 否则内存越界。
 */
bool          uorb_copy(uorb_handle_t handle, void *buf);

/**
 * @brief  全局初始化, 必须在任何 uorb API 调用之前执行
 *
 * 建议在 SystemInit 任务最开头调用一次。
 */
void          uorb_init(void);

#ifdef __cplusplus
}
#endif
