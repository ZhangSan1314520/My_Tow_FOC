/**
 * ============================================================
 * uORB 核心实现
 * ============================================================
 *
 * 数据流:
 *
 *   发布者                     全局 Topic 表              订阅者
 *   ──────                    ────────────               ──────
 *   advertise("accel",16)     topics[0] {                subscribe("accel")
 *          │                    name="accel"                     │
 *          │                    data=[16 bytes]                  │
 *          │                    generation=0                     │
 *          │                    subs[0]={last_seen=0}            │
 *          │                    }                                │
 *          │                                                    │
 *   publish(h, &data)  ──→  memcpy data                        │
 *          │                generation++ (→1)                    │
 *          │                                                    │
 *          │                                     check/copy ──→ generation 从 0 变 1,
 *          │                                                    感知到新数据, 拷贝
 *          │                                                    更新 last_seen=1
 *
 *   关键机制:
 *   - 匹配: publish 和 subscribe 靠同一个 Topic 名字字符串关联
 *   - 通知: 靠 generation 整数, 不用信号量唤醒 (轮询 + 1ms 延时)
 *   - 数据: 共享内存方案, 每次 publish 是 memcpy 一份副本
 *   - 安全: global_lock 保护 topics 数组, topic 级 mutex 保护数据读写
 * ============================================================
 */

#include "uorb.hpp"

/* ── 全局 Topic 表 ──
 * 所有 Topic 状态集中存于此, 全局锁保护增删操作
 */
static uorb_topic_t topics[UORB_MAX_TOPICS];
static SemaphoreHandle_t global_lock;   // 保护 topics 数组结构的互斥锁

/**
 * @brief  全局初始化: 清零 Topic 表 + 创建全局锁
 *
 * 必须在调度器启动后、任何 uorb API 调用前执行。
 * 重复调用会导致锁泄漏, 不建议。
 */
void uorb_init(void) {
    memset(topics, 0, sizeof(topics));               // 所有槽位标记为 invalid
    global_lock = xSemaphoreCreateMutex();           // 创建保护 topics 数组的锁
}

/**
 * @brief  在全局表中按名字查找 Topic, 找不到则分配新槽位
 * @param  name  Topic 名字
 * @return 找到或新分配的槽位指针, 表满则返回 NULL
 *
 * 两步:
 *   1. 遍历 topics[], 找 name 相同且 valid 的 → 已有 Topic (回收)
 *   2. 找不到 → 找第一个 invalid 槽位 → 初始化新 Topic
 *
 * 注意: 调用者必须先持有 global_lock
 */
static uorb_topic_t *find_or_create(const char *name) {
    /* Step 1: 查找已存在的同名 Topic */
    for (int i = 0; i < UORB_MAX_TOPICS; i++) {
        if (topics[i].valid && strcmp(topics[i].name, name) == 0)
            return &topics[i];                       // 找到了, 直接返回
    }
    /* Step 2: 分配新槽位 */
    for (int i = 0; i < UORB_MAX_TOPICS; i++) {
        if (!topics[i].valid) {                      // 找到空闲槽
            strncpy(topics[i].name, name, UORB_TOPIC_NAME_LEN - 1);
            topics[i].mutex = xSemaphoreCreateMutex(); // 为这个 Topic 创建专属锁
            topics[i].valid = true;
            topics[i].generation = 0;
            topics[i].sub_count = 0;
            return &topics[i];
        }
    }
    return NULL;                                     // 表满了
}

/**
 * @brief  发布端: 公告一个 Topic
 *
 * 内部逻辑:
 *   1. 获取全局锁
 *   2. 查找/创建 Topic 槽位
 *   3. 如果该 Topic 已有 data 指针 → 说明已被 advertise 过 → 失败
 *   4. 分配数据内存区, 记录 data_size
 *   5. 返回 handle = topics 数组下标
 *
 * handle 是数组下标, publish 时直接 topics[handle] 访问, O(1) 无需查名字。
 */
uorb_handle_t uorb_advertise(const char *name, uint32_t data_size) {
    xSemaphoreTake(global_lock, portMAX_DELAY);      // 🔒 锁全局表
    uorb_topic_t *t = find_or_create(name);
    if (!t || t->data) {                             // 表满 或 已被别人 advertise
        xSemaphoreGive(global_lock);                 // 🔓 放锁
        return -1;
    }
    t->data = pvPortMalloc(data_size);               // 从 FreeRTOS 堆分配数据区
    t->data_size = data_size;
    int idx = t - topics;                            // 指针减法 = 数组下标
    xSemaphoreGive(global_lock);                     // 🔓 放锁
    return idx;                                      // 返回的 handle 就是槽号
}

/**
 * @brief  发布端: 向 Topic 写入新数据
 *
 * 内部逻辑:
 *   1. 校验 handle 合法性
 *   2. 获取该 Topic 的专属 mutex (只锁这一个 Topic, 不影响其他)
 *   3. memcpy 数据到 Topic 的数据区
 *   4. generation++ → 这一步"通知"了所有订阅者
 *   5. 释放 mutex
 *
 * publish 不直接发信号给订阅者, 而是靠 generation 这个整数。
 * 订阅者自己轮询发现 generation 变了, 这就是"被动通知"机制。
 */
void uorb_publish(uorb_handle_t handle, const void *data) {
    if (handle < 0 || handle >= UORB_MAX_TOPICS) return;  // 无效句柄, 静默丢弃
    uorb_topic_t *t = &topics[handle];
    if (!t->valid) return;                                 // 无效槽, 静默丢弃

    xSemaphoreTake(t->mutex, portMAX_DELAY);               // 🔒 锁数据区
    memcpy(t->data, data, t->data_size);                   // 写入最新值
    t->generation++;                                       // ★ 代际号递增, 通知订阅者
    xSemaphoreGive(t->mutex);                              // 🔓 放锁
}

/**
 * @brief  订阅端: 订阅一个 Topic
 *
 * 内部逻辑:
 *   1. 获取全局锁
 *   2. 查找 Topic (不存在则创建空壳)
 *   3. 在当前 Topic 的 subscribers[] 中占一个槽位
 *   4. 记录当前 generation 作为 last_seen_gen (避免立即收到旧数据)
 *   5. 返回编码后的 handle
 *
 * handle 编码:
 *   - 低 16 位 = topics 数组下标 (定位 Topic)
 *   - 高 16 位 = subscribers 槽位号 (定位该订阅者的 last_seen_gen)
 *
 * 一个 Topic 可以有多个订阅者, 每个拿到的 handle 高 16 位不同,
 * 但低 16 位相同——都指向同一个 Topic。
 */
uorb_handle_t uorb_subscribe(const char *name) {
    xSemaphoreTake(global_lock, portMAX_DELAY);            // 🔒 锁全局表
    uorb_topic_t *t = find_or_create(name);
    if (!t || t->sub_count >= UORB_MAX_SUBSCRIBERS) {      // 表满或订阅者满
        xSemaphoreGive(global_lock);                       // 🔓 放锁
        return -1;
    }
    t->subscribers[t->sub_count].task = xTaskGetCurrentTaskHandle();
    t->subscribers[t->sub_count].last_seen_gen = t->generation;  // 初始值: 当前代际
    int slot = t->sub_count;                                     // 记下槽位号
    t->sub_count++;
    xSemaphoreGive(global_lock);                           // 🔓 放锁

    // 编码 handle: 低16位=Topic号, 高16位=订阅者槽号
    return (t - topics) | (slot << 16);
}

/* ── handle 编解码 ── */
static inline int handle_topic(uorb_handle_t h) { return h & 0xFFFF; }
static inline int handle_slot(uorb_handle_t h)  { return (h >> 16) & 0xFFFF; }

/**
 * @brief  订阅端: 非阻塞检查是否有新数据
 *
 * 对比订阅者的 last_seen_gen 与 Topic 的 generation。
 * 不等 → 发布者写过新数据了 → 返回 true。
 * 相等 → 没有新数据 → 返回 false。
 *
 * 不拷贝数据, 不阻塞, 适合轮询场景。
 */
bool uorb_check(uorb_handle_t handle) {
    int ti = handle_topic(handle), si = handle_slot(handle);
    if (ti < 0 || ti >= UORB_MAX_TOPICS) return false;
    uorb_topic_t *t = &topics[ti];
    if (si >= t->sub_count) return false;
    return t->subscribers[si].last_seen_gen < t->generation;  // 只比大小, 不等就有新
}

/**
 * @brief  订阅端: 阻塞读取最新数据
 *
 * 内部逻辑:
 *   1. while 循环等 generation 变化 (每 1ms 检查一次, 不自旋)
 *   2. 拿到 Topic 的数据锁
 *   3. memcpy 数据到用户 buf
 *   4. 更新 last_seen_gen = 当前 generation
 *   5. 放锁返回
 *
 * 阻塞机制是轮询 vTaskDelay(1), 不是信号量通知。
 * 这意味着最差响应延迟 ≈ 1ms (对于电机控制 1kHz 以下够用)。
 */
bool uorb_copy(uorb_handle_t handle, void *buf) {
    int ti = handle_topic(handle), si = handle_slot(handle);
    uorb_topic_t *t = &topics[ti];

    /* 轮询等待: 每 1ms 看一眼 generation 变了没有 */
    while (t->subscribers[si].last_seen_gen == t->generation)
        vTaskDelay(1);                                   // 出让 CPU, 不自旋

    /* 数据到达, 拷贝 */
    xSemaphoreTake(t->mutex, portMAX_DELAY);             // 🔒 锁数据区 (防止读半截)
    memcpy(buf, t->data, t->data_size);
    uint32_t gen = t->generation;                        // 记录当前代际
    xSemaphoreGive(t->mutex);                            // 🔓 放锁

    t->subscribers[si].last_seen_gen = gen;              // 更新"已看到"的代际
    return true;
}
