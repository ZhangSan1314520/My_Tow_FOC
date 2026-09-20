// ============================================================
// Serial 串口驱动层 - 统一管理 STM32 UART 的收发
// 支持 DMA / IT 两种接收模式，支持 FreeRTOS 任务安全调用
// ============================================================

#include "serial.hpp"

#include <string.h>
#include "printf.h"

#include "HAL_System.hpp"

// ============================================================
// 静态成员 - 维护全局串口列表，用于回调函数中快速查找对应 Serial 实例
// sr_list 是一个固定大小的数组，通过 UART 句柄找到对应的 Serial 对象
// ============================================================
Serial *Serial::sr_list[SERIAL_MAX_COUNT] = {
    NULL,  // 预留给串口 0
    // NULL,  // 预留给串口 1...（取决于 SERIAL_MAX_COUNT 大小）
};
uint8_t Serial::sr_count = 0;  // 已注册的串口数量计数器

// ============================================================
// get_serial - 通过 UART 句柄查找对应的 Serial 实例
// 参数：uart - 指向 UART_HandleTypeDef 的指针
// 返回：找到返回 Serial 指针，未找到返回 NULL
// ============================================================
Serial *Serial::get_serial(UART_HandleTypeDef *uart)
{
    // 遍历已注册的串口列表
    for (uint8_t i = 0; i < sr_count; i++)
    {
        // 比较句柄地址是否匹配
        if (uart == sr_list[i]->_uart)
            return sr_list[i];  // 找到对应的 Serial 实例
    }
    return NULL;  // 未找到
}

// ============================================================
// init - 完整初始化串口（需要所有参数）
// 参数：
//   name   - 串口名称（用于任务命名，最大 SERIAL_NAME_SIZE 字符）
//   config - 串口配置结构体指针
//   uart   - CubeMX 生成的 UART 句柄指针
// 返回：true 成功，false 失败
// ============================================================
bool Serial::init(const char *name, Serial_Config *config, UART_HandleTypeDef *uart)
{
    // -------------------- 1. 参数合法性检查 --------------------
    if (name == NULL || config == NULL || uart == NULL)
    {
        _error = Serial_Error::Init_Error;  // 记录错误类型
        return false;
    }

    // -------------------- 2. 配置参数合理性检查 --------------------
    // 名称长度检查（防止溢出）
    // strnlen 返回字符串长度，超过 SERIAL_NAME_SIZE 则报错
    if (strnlen(name, SERIAL_NAME_SIZE * 2) >= SERIAL_NAME_SIZE ||
        // 接收/发送模式不能是 None（否则无法工作）
        config->rx_mode == Serial_Mode::None || config->tx_mode == Serial_Mode::None ||
        // 数据包大小必须 > 0
        config->rx_pack_size == 0 || config->tx_pack_size == 0 ||
        // 队列长度必须 > 0
        config->rx_queue_len == 0 || config->tx_queue_len == 0)
    {
        _error = Serial_Error::Init_Error;
        return false;
    }

    // -------------------- 3. 清零实例成员 --------------------
    memset(_name, 0, sizeof(_name));      // 清零名称缓冲区
    memset(&_config, 0, sizeof(_config)); // 清零配置结构体
    memset(&_info, 0, sizeof(_info));    // 清零统计信息结构体

    // -------------------- 4. 复制配置 --------------------
    strncpy(_name, name, SERIAL_NAME_SIZE);  // 复制串口名称
    _config = *config;                        // 复制配置参数
    _uart = uart;                             // 保存 UART 句柄

    // -------------------- 5. 注册到全局串口列表 --------------------
    // 防止同一个 UART 句柄被注册两次
    if (get_serial(uart) != NULL)
    {
        _error = Serial_Error::Init_Error;
        return false;
    }
    sr_list[sr_count] = this;  // 将当前实例加入全局列表
    sr_count++;                // 计数器加一

    // -------------------- 6. 初始化子模块（任一失败则整体失败） --------------------
    // 缓冲区初始化
    if (!_init_buf())
    {
        _error = Serial_Error::Init_Error;
        return false;
    }
    // 队列初始化（FreeRTOS 消息队列，用于任务间通信）
    if (!_init_queue())
    {
        _error = Serial_Error::Init_Error;
        return false;
    }
    // 信号量初始化（二值信号量，保护发送资源）
    if (!_init_semaphore())
    {
        _error = Serial_Error::Init_Error;
        return false;
    }
    // FreeRTOS 任务初始化（接收/发送任务）
    if (!_init_task())
    {
        _error = Serial_Error::Init_Error;
        return false;
    }
    // 回调函数注册（HAL 库回调）
    if (!_init_callback())
    {
        _error = Serial_Error::Init_Error;
        return false;
    }

    // -------------------- 7. 开启串口接收 --------------------
    // 根据 rx_mode 选择 IT 或 DMA 模式启动接收
    if (!_rx_irq_start())
    {
        _error = Serial_Error::Start_Error;
        return false;
    }

    // -------------------- 8. 标记就绪 --------------------
    _ready = true;
    return true;
}

// ============================================================
// init_with_default - 使用默认配置快速初始化
// 参数：
//   name - 串口名称
//   uart - UART 句柄
// 返回：true 成功，false 失败
// 说明：自动生成默认配置（DMA 模式，1KB 缓冲区等）
// ============================================================
bool Serial::init_with_default(const char *name, UART_HandleTypeDef *uart)
{
    Serial_Config config;
    generate_default_config(&config);  // 生成默认配置
    return init(name, &config, uart);  // 调用完整初始化
}

// ============================================================
// generate_default_config - 生成默认串口配置
// 参数：config - 输出参数，填充默认配置
// 返回：true 成功
// ============================================================
bool Serial::generate_default_config(Serial_Config *config)
{
    if (config == NULL)
        return false;

    // 接收模式：DMA（Direct Memory Access，直接内存访问，不占用 CPU）
    // DMA 模式适合高速通信，减少 CPU 中断次数
    config->rx_mode = Serial_Mode::DMA;

    // 发送模式：DMA
    config->tx_mode = Serial_Mode::DMA;

    // 接收/发送缓冲区大小：1KB
    // 缓冲区用于临时存储数据，DMA 接收的数据先存入这里
    config->rx_buf_size = 1024;  // 接收缓冲区 1KB
    config->tx_buf_size = 1024;  // 发送缓冲区 1KB

    // 单次数据包最大长度：256 字节
    // DMA/IT 每次接收的最大数据量
    config->rx_pack_size = 256;  // 接收数据包大小
    config->tx_pack_size = 256;  // 发送数据包大小

    // FreeRTOS 队列长度：100
    // 队列用于在中断和任务之间传递数据包
    // 队列满时会阻塞发送任务，直到队列有空间
    config->rx_queue_len = 100;  // 接收队列可存储 100 个数据包
    config->tx_queue_len = 100;  // 发送队列可存储 100 个数据包

    return true;
}

// ============================================================
// _init_buf - 初始化收发缓冲区
// 返回：true 成功，false 失败（内存分配失败）
// 说明：使用 FreeRTOS 的 pvPortMalloc 分配内存（从堆中分配）
// ============================================================
bool Serial::_init_buf(void)
{
    // -------------------- 接收缓冲区 --------------------
    // 为接收缓冲区分配内存，大小由配置决定
    _rx_buf.pBuffer = (uint8_t *)pvPortMalloc(_config.rx_buf_size);
    _rx_buf.pData = _rx_buf.pBuffer;  // 数据指针初始指向缓冲区开头
    _rx_buf.size = _config.rx_buf_size;  // 保存缓冲区大小
    _rx_buf.status = 0;                // 状态清零
    _rx_buf.index = 0;                  // 读写索引清零

    // -------------------- 发送缓冲区 --------------------
    _tx_buf.pBuffer = (uint8_t *)pvPortMalloc(_config.tx_buf_size);
    _tx_buf.pData = _tx_buf.pBuffer;
    _tx_buf.size = _config.tx_buf_size;
    _tx_buf.status = 0;
    _tx_buf.index = 0;

    // -------------------- 内存分配检查 --------------------
    // 如果任一分配失败，返回 false
    if (_rx_buf.pBuffer == NULL || _tx_buf.pBuffer == NULL)
        return false;

    return true;
}

// ============================================================
// _init_queue - 初始化 FreeRTOS 消息队列
// 返回：true 成功，false 失败
// 说明：队列用于在中断回调和任务之间传递 DataPack_t 数据包
// ============================================================
bool Serial::_init_queue(void)
{
    // 创建接收队列：长度为 rx_queue_len，每个元素大小为 DataPack_t
    // xQueueCreate(队列长度, 每个元素字节数)
    _rx_queue = xQueueCreate(_config.rx_queue_len, sizeof(DataPack_t));

    // 创建发送队列
    _tx_queue = xQueueCreate(_config.tx_queue_len, sizeof(DataPack_t));

    // 如果任一队列创建失败，返回 false
    if (_rx_queue == NULL || _tx_queue == NULL)
        return false;

    return true;
}

// ============================================================
// _init_semaphore - 初始化二值信号量
// 返回：true 成功，false 失败
// 说明：
//   - _tx_src_sem：控制 UART 发送硬件资源（防止多任务同时发送）
//   - _tx_buf_sem：控制发送缓冲区的读写权限
// ============================================================
bool Serial::_init_semaphore(void)
{
    // 创建二值信号量（初始值为 1，表示资源可用）
    // 二值信号量 = 互斥锁的一种简化形式
    _tx_src_sem = xSemaphoreCreateBinary();

    // 创建发送缓冲区信号量
    _tx_buf_sem = xSemaphoreCreateBinary();

    // 检查创建是否成功
    if (_tx_src_sem == NULL || _tx_buf_sem == NULL)
        return false;

    // 初始化时释放信号量（资源可用状态）
    xSemaphoreGive(_tx_src_sem);
    xSemaphoreGive(_tx_buf_sem);

    return true;
}

// ============================================================
// _init_task - 初始化 FreeRTOS 接收/发送任务
// 返回：true 成功，false 失败
// 说明：
//   - _rx_task：接收任务，从队列取数据，调用用户注册的回调
//   - _tx_task：发送任务，从队列取数据，触发 DMA/IT 发送
// ============================================================
bool Serial::_init_task(void)
{
    BaseType_t status = pdPASS;  // 任务创建状态，pdPASS 表示成功

    // -------------------- 接收任务 --------------------
    // 使用 lambda 封装 C++ 成员函数调用
    // FreeRTOS 任务函数必须是静态函数或普通函数，不能是成员函数
    TaskFunction_t rx_task_lambda = [](void *args)
    {
        // args 传入 this 指针，强转回 Serial* 后调用成员函数
        Serial *sr = (Serial *)args;
        sr->_rx_task();  // 进入接收任务主循环
    };

    // 生成任务名称：rx_<串口名>
    snprintf(_rx_task_name, sizeof(_rx_task_name), "rx_%s", _name);

    // 创建 FreeRTOS 任务
    // xTaskCreate(任务函数, 任务名, 栈大小(Words), 参数, 优先级, 句柄)
    // 256 Words = 1KB 栈空间（STM32 是 32 位，1 Word = 4 字节）
    // 优先级 osPriorityNormal = 普通优先级
    status &= xTaskCreate(rx_task_lambda, _rx_task_name, 256, this, osPriorityNormal, &_rx_task_handle);

    // -------------------- 发送任务 --------------------
    TaskFunction_t tx_task_lambda = [](void *args)
    {
        Serial *sr = (Serial *)args;
        sr->_tx_task();  // 进入发送任务主循环
    };

    snprintf(_tx_task_name, sizeof(_tx_task_name), "tx_%s", _name);
    status &= xTaskCreate(tx_task_lambda, _tx_task_name, 256, this, osPriorityNormal, &_tx_task_handle);

    // 如果任一任务创建失败，返回 false
    if (status != pdPASS)
        return false;

    return true;
}

// ============================================================
// _init_callback - 注册 UART 的 HAL 回调函数
// 返回：true 成功，false 失败
// 说明：需要 CubeMX 开启 USE_HAL_UART_REGISTER_CALLBACKS
// ============================================================
bool Serial::_init_callback(void)
{
    uint8_t status = HAL_OK;  // HAL 状态，HAL_OK = 成功

    // -------------------- 接收完成回调 --------------------
    // RxEventCallback：在 DMA/IT 接收到数据或空闲时触发
    // 接收模式为 DMA 时，使用 RxEventCallback
    // 接收模式为 IT 时，可能使用 RxCpltCallback
    static auto rx_callback_lambda = [](UART_HandleTypeDef *huart, uint16_t size) -> void
    {
        // 通过 UART 句柄查找对应的 Serial 实例
        Serial *sr = Serial::get_serial(huart);
        if (sr != NULL)
            sr->_rx_callback(size);  // 调用成员函数处理接收数据
        else
            sr->_error_handler(Serial_Error::Init_Error);  // 未找到，报错
    };
    // 向 HAL 库注册接收回调
    status |= HAL_UART_RegisterRxEventCallback(_uart, rx_callback_lambda);

    // -------------------- 发送完成回调 --------------------
    // TxCompleteCallback：在 DMA/IT 发送完成后触发
    static auto tx_callback_lambda = [](UART_HandleTypeDef *huart) -> void
    {
        Serial *sr = Serial::get_serial(huart);
        if (sr != NULL)
            sr->_tx_callback();  // 通知发送任务数据已发送完毕
        else
            sr->_error_handler(Serial_Error::Init_Error);
    };
    // 注册发送完成回调，HAL_UART_TX_COMPLETE_CB_ID 指定回调类型
    status |= HAL_UART_RegisterCallback(_uart, HAL_UART_TX_COMPLETE_CB_ID, tx_callback_lambda);

    // -------------------- 错误回调 --------------------
    // ErrorCallback：当 UART 发生错误时触发（噪声错误、帧错误等）
    static auto error_callback_lambda = [](UART_HandleTypeDef *huart) -> void
    {
        Serial *sr = Serial::get_serial(huart);
        if (sr != NULL)
            sr->_error_callback();  // 清除错误标志并重启接收
        else
            sr->_error_handler(Serial_Error::Init_Error);
    };
    status |= HAL_UART_RegisterCallback(_uart, HAL_UART_ERROR_CB_ID, error_callback_lambda);

    // 如果任一注册失败，返回 false
    if (status != HAL_OK)
        return false;

    return true;
}

// ============================================================
// _is_tx_busy - 判断发送是否忙碌（不能接收新数据）
// 参数：from_isr - 是否在中断上下文中调用
// 返回：true 忙碌（不能发送），false 空闲（可以发送）
// 说明：检查两个条件：
//   1. 待发送数据量超过缓冲区的 80%（发送来不及）
//   2. 发送队列剩余空间小于 2（队列快满了）
// ============================================================
bool Serial::_is_tx_busy(bool from_isr)
{
    // -------------------- 条件 1：发送缓冲区积压过多 --------------------
    // _info.tx_to_process_size 表示待发送的字节数
    // 如果超过缓冲区的 80%，说明发送速度跟不上，返回忙碌
    if (_info.tx_to_process_size > _config.tx_buf_size * 0.8)
        return true;

    // -------------------- 条件 2：发送队列空间不足 --------------------
    UBaseType_t tx_queue_wait;  // 队列中等待的消息数

    if (from_isr)
        // 中断上下文：从 ISR 安全版本查询队列
        tx_queue_wait = uxQueueMessagesWaitingFromISR(_tx_queue);
    else
        // 任务上下文：普通版本查询队列
        tx_queue_wait = uxQueueMessagesWaiting(_tx_queue);

    // 计算队列剩余空间
    if (_config.tx_queue_len - tx_queue_wait < 2)
        return true;  // 剩余空间 < 2，返回忙碌

    return false;  // 两个条件都不满足，表示可以发送
}

// ============================================================
// _update_info - 更新串口统计信息
// 参数：
//   rx_size - 本次接收的字节数（负数表示已发送）
//   tx_size - 本次发送的字节数（负数表示已发送）
// ============================================================
void Serial::_update_info(int16_t rx_size, int16_t tx_size)
{
    // 更新总收发字节数（累计）
    if (rx_size > 0)
        _info.rx_total_size += rx_size;  // 累计接收字节数
    if (tx_size > 0)
        _info.tx_total_size += tx_size;  // 累计发送字节数

    // 更新待处理字节数（正数=新增，负数=已处理）
    _info.rx_to_process_size += rx_size;  // 待处理接收数据
    _info.tx_to_process_size += tx_size;  // 待处理发送数据
}

// ============================================================
// _rx_irq_start - 开启 UART 接收中断/DMA
// 返回：true 成功，false 失败
// 说明：根据 rx_mode 选择 IT 或 DMA 模式启动接收
// ============================================================
bool Serial::_rx_irq_start(void)
{
    uint8_t status;  // HAL 状态

    // -------------------- IT 模式（中断模式） --------------------
    if (_config.rx_mode == Serial_Mode::IT)
    {
        // HAL_UARTEx_ReceiveToIdle_IT：
        // 接收数据直到：
        //   1. 收到 rx_pack_size 个字节，或
        //   2. 检测到 UART 空闲线（数据发送完毕）
        // 收到 rx_pack_size 字节会触发 RxCpltCallback
        // 空闲检测会触发 RxEventCallback
        status = HAL_UARTEx_ReceiveToIdle_IT(_uart, _rx_buf.pData, _config.rx_pack_size);

        // 如果 UART 正在忙碌（上次接收未完成），重试
        if (status == HAL_BUSY)
        {
            _uart->Lock = HAL_UNLOCKED;  // 解锁 UART 句柄
            status = HAL_UARTEx_ReceiveToIdle_IT(_uart, _rx_buf.pData, _config.rx_pack_size);
            _info.error_cnt++;  // 错误计数加一
        }
    }
    // -------------------- DMA 模式（直接内存访问） --------------------
    else if (_config.rx_mode == Serial_Mode::DMA)
    {
        // HAL_UARTEx_ReceiveToIdle_DMA：
        // 与 IT 模式相同，但数据直接通过 DMA 传到内存，不占用 CPU 中断
        // DMA 优点：CPU 不参与数据传输，效率高，适合高速通信
        status = HAL_UARTEx_ReceiveToIdle_DMA(_uart, _rx_buf.pData, _config.rx_pack_size);

        if (status == HAL_BUSY)
        {
            _uart->Lock = HAL_UNLOCKED;
            status = HAL_UARTEx_ReceiveToIdle_DMA(_uart, _rx_buf.pData, _config.rx_pack_size);
            _info.error_cnt++;
        }
    }
    // -------------------- 模式无效 --------------------
    else
    {
        status = HAL_ERROR;
    }

    if (status != HAL_OK)
        return false;

    return true;
}

// ============================================================
// _tx_irq_start - 开启 UART 发送中断/DMA
// 参数：tx_pack - 要发送的数据包指针
// 返回：true 成功，false 失败
// ============================================================
bool Serial::_tx_irq_start(DataPack_t *tx_pack)
{
    uint8_t status = HAL_ERROR;

    // -------------------- IT 模式 --------------------
    if (_config.tx_mode == Serial_Mode::IT)
    {
        // HAL_UART_Transmit_IT：中断方式发送，不阻塞
        status = HAL_UART_Transmit_IT(_uart, tx_pack->pData, tx_pack->size);
        if (status == HAL_BUSY)
        {
            _uart->Lock = HAL_UNLOCKED;
            status = HAL_UART_Transmit_IT(_uart, tx_pack->pData, tx_pack->size);
            _info.error_cnt++;
        }
    }
    // -------------------- DMA 模式 --------------------
    else if (_config.tx_mode == Serial_Mode::DMA)
    {
        // HAL_UART_Transmit_DMA：DMA 方式发送，不占用 CPU
        status = HAL_UART_Transmit_DMA(_uart, tx_pack->pData, tx_pack->size);
        if (status == HAL_BUSY)
        {
            _uart->Lock = HAL_UNLOCKED;
            status = HAL_UART_Transmit_DMA(_uart, tx_pack->pData, tx_pack->size);
            _info.error_cnt++;
        }
    }
    // -------------------- 模式为 None --------------------
    else if (_config.tx_mode == Serial_Mode::None)
    {
        // 发送模式为 None 是配置错误，进入错误处理
        _error_handler(Serial_Error::Start_Error);
    }

    if (status != HAL_OK)
        return false;

    return true;
}

// ============================================================
// register_rx_msg_handler - 注册用户的消息回调函数
// 参数：
//   rx_msg_handler - 用户回调函数指针
//   args - 用户自定义参数（透传到回调中）
// 返回：true 成功，false 失败
// 说明：用户注册后，每收到一包数据会调用此回调
// ============================================================
bool Serial::register_rx_msg_handler(Serial_RX_Msg_Handler rx_msg_handler, void *args)
{
    if (rx_msg_handler == NULL)
        return false;

    _rx_msg_handler = rx_msg_handler;      // 保存回调函数指针
    _rx_msg_handler_args = args;           // 保存用户参数

    return true;
}

// ============================================================
// _rx_callback - UART 接收完成回调（由 HAL 库调用）
// 参数：size - 本次接收到的字节数
// 说明：
//   - 在 DMA 半传输完成时只跳过，不处理
//   - 将数据封装成 DataPack_t 放入接收队列
//   - 由 _rx_task 从队列取走处理
// ============================================================
void Serial::_rx_callback(uint16_t size)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;  // 是否需要任务切换
    DataPack_t rx_pack;                               // 接收数据包

    // -------------------- DMA 半传输完成跳过 --------------------
    // DMA 模式下，如果 rx_pack_size 设置较大（如 256 字节），
    // DMA 会分两次传输：前半部 → RxEventCallback → 后半部 → RxEventCallback
    // 这里跳过前半部，只在后半部（完整包）时处理
    if (_config.rx_mode == Serial_Mode::DMA)
    {
        // _uart->RxXferSize 是配置的 DMA 传输总大小
        // 如果 size = 总大小/2 且 _half_cplt = false，说明是前半部
        if (size == _uart->RxXferSize / 2 && _half_cplt == false)
        {
            _half_cplt = true;   // 标记已收到前半部
            return;              // 跳过，等待后半部
        }
        _half_cplt = false;      // 收到后半部，重置标志
    }

    // -------------------- 更新统计信息 --------------------
    _update_info(size, 0);  // 本次收到 size 字节

    // -------------------- 获取时间戳 --------------------
    // 使用 HAL_System 的毫秒计时器记录收到数据的时间
    rx_pack.tick = HAL_System::get_tick_ms();

    // -------------------- 更新缓冲区状态 --------------------
    // 数据已复制到 _rx_buf.pData 指向的位置
    _rx_buf.index += size;       // 缓冲区已用长度增加
    rx_pack.pData = _rx_buf.pData;  // 数据包指向缓冲区
    rx_pack.size = size;           // 记录数据长度

    // -------------------- 环形缓冲区指针回绕 --------------------
    // 如果剩余空间不足一个最大数据包，将指针回到缓冲区开头
    // 实现环形缓冲区的效果
    if (_rx_buf.size - _rx_buf.index < _config.rx_pack_size)
    {
        _rx_buf.index = 0;  // 回绕到开头
    }
    _rx_buf.pData = _rx_buf.pBuffer + _rx_buf.index;  // 更新下次写入位置

    // -------------------- 重新开启接收 --------------------
    // 接收完一包后，重新启动 DMA/IT，准备下一包
    _rx_irq_start();

    // -------------------- 放入接收队列 --------------------
    // xQueueSendFromISR：将数据包放入队列，通知 _rx_task 处理
    // 第二个参数是要发送的数据，第三个参数用于返回是否需要任务切换
    xQueueSendFromISR(_rx_queue, &rx_pack, &pxHigherPriorityTaskWoken);

    // -------------------- 触发任务切换（如有必要） --------------------
    // 如果放入队列后，有更高优先级的任务等待执行，触发调度
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

// ============================================================
// _tx_callback - UART 发送完成回调（由 HAL 库调用）
// 说明：
//   - 通知 _tx_task 可以发送下一包数据
//   - 释放 _tx_src_sem 信号量（发送硬件资源）
// ============================================================
void Serial::_tx_callback(void)
{
    // -------------------- 更新统计信息 --------------------
    // 发送 size 字节（取负表示已完成）
    _update_info(0, -(int16_t)_uart->TxXferSize);

    // -------------------- 释放发送信号量 --------------------
    // 发送完成，UART 硬件资源可用，释放信号量
    // _tx_task 会等待此信号量，收到后才发下一包
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(_tx_src_sem, &pxHigherPriorityTaskWoken);

    // 触发任务切换
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

// ============================================================
// _error_callback - UART 错误回调（由 HAL 库调用）
// 说明：处理噪声错误、帧错误等，清除标志并重启接收
// ============================================================
void Serial::_error_callback(void)
{
    // -------------------- 清除错误标志 --------------------
    if (_uart->ErrorCode != HAL_UART_ERROR_NONE)
    {
        // 噪声错误（Noise Error）：接收到噪声
        if (__HAL_UART_GET_FLAG(_uart, UART_FLAG_NE))
        {
            __HAL_UART_CLEAR_NEFLAG(_uart);  // 清除噪声错误标志
        }

        // 帧错误（Framing Error）：停止位不对
        if (__HAL_UART_GET_FLAG(_uart, UART_FLAG_FE))
        {
            __HAL_UART_CLEAR_FEFLAG(_uart);  // 清除帧错误标志
        }
    }

    // -------------------- 重新开启接收 --------------------
    // 发生错误后，DMA/IT 可能已停止，需要重新启动
    _rx_irq_start();
}

// ============================================================
// send_data - 发送数据（任务安全版本）
// 参数：
//   pack - 数据包指针（包含数据地址、大小、时间戳）
//   auto_gen_tick - 是否自动生成时间戳
// 返回：true 成功，false 失败
// 说明：
//   - 线程安全，可在任意任务中调用
//   - 内部使用信号量保护发送资源
//   - 如果发送忙碌会等待，最多等待 100 次 × 1ms
// ============================================================
bool Serial::send_data(DataPack_t *pack, bool auto_gen_tick)
{
    DataPack_t tx_pack;  // 本地数据包副本

    // -------------------- 参数检查 --------------------
    if (pack == NULL || pack->pData == NULL)
        return false;

    // -------------------- 就绪状态和大小检查 --------------------
    if (pack->size == 0 || !is_ready())
        return false;

    // -------------------- 单次发送大小限制 --------------------
    // 单次最多发送缓冲区 80% 的数据，防止阻塞
    if (pack->size > _config.tx_buf_size * 0.8)
        return false;

    // -------------------- 检查发送是否忙碌 --------------------
    if (_is_tx_busy())
        return false;

    // -------------------- 占用发送缓冲区 --------------------
    // 获取信号量，阻塞直到可用
    // portMAX_DELAY = 无限等待
    xSemaphoreTake(_tx_buf_sem, portMAX_DELAY);

    // -------------------- 生成时间戳 --------------------
    if (auto_gen_tick)
    {
        tx_pack.tick = HAL_System::get_tick_ms();
    }

    // -------------------- 计算发送缓冲区剩余空间 --------------------
    uint16_t rest_buf_size = _tx_buf.size - _tx_buf.index;

    // -------------------- 分情况复制数据 --------------------
    // 情况 1：剩余空间足够，直接复制
    if (pack->size <= rest_buf_size)
    {
        // 将数据复制到发送缓冲区
        memcpy(_tx_buf.pData, pack->pData, pack->size);

        tx_pack.pData = _tx_buf.pData;  // 数据包指向发送缓冲区
        tx_pack.size = pack->size;      // 数据长度

        // 将数据包放入发送队列
        xQueueSend(_tx_queue, &tx_pack, portMAX_DELAY);

        _tx_buf.index += pack->size;  // 更新写入位置

        // 如果剩余空间不足，回绕到开头
        if (_tx_buf.size - _tx_buf.index < _config.tx_pack_size)
        {
            _tx_buf.index = 0;
        }
        _tx_buf.pData = _tx_buf.pBuffer + _tx_buf.index;
    }
    // 情况 2：剩余空间不足，分两部分复制
    else
    {
        // 第一部分：填满剩余空间
        memcpy(_tx_buf.pData, pack->pData, rest_buf_size);

        tx_pack.pData = _tx_buf.pData;
        tx_pack.size = rest_buf_size;

        // 发送第一部分
        xQueueSend(_tx_queue, &tx_pack, portMAX_DELAY);

        // 第二部分长度
        uint16_t rest_pack_size = pack->size - rest_buf_size;

        // 第二部分：从缓冲区开头继续复制
        memcpy(_tx_buf.pBuffer, pack->pData + rest_buf_size, rest_pack_size);

        tx_pack.pData = _tx_buf.pBuffer;
        tx_pack.size = rest_pack_size;

        // 发送第二部分
        xQueueSend(_tx_queue, &tx_pack, portMAX_DELAY);

        _tx_buf.index = rest_pack_size;
        _tx_buf.pData = _tx_buf.pBuffer + _tx_buf.index;
    }

    // -------------------- 更新统计 --------------------
    _update_info(0, pack->size);

    // -------------------- 释放发送缓冲区 --------------------
    xSemaphoreGive(_tx_buf_sem);

    return true;
}

// ============================================================
// send_data - 简化版发送（直接传数据和长度）
// 参数：
//   data - 数据指针
//   size - 数据长度
//   tick - 时间戳（0 = 自动生成）
// 返回：true 成功，false 失败
// ============================================================
bool Serial::send_data(uint8_t *data, uint16_t size, uint32_t tick)
{
    DataPack_t tx_pack = {
        .pData = data,    // 数据指针
        .size = size,     // 数据长度
        .tick = tick      // 时间戳
    };

    // tick = 0 则使用自动生成的时间戳
    return send_data(&tx_pack, (tick == 0));
}

// ============================================================
// send_data_from_isr - 中断安全版本的发送函数
// 参数：同 send_data
// 返回：true 成功，false 失败
// 说明：
//   - 可在中断回调中调用
//   - 使用 FromISR 后缀的 FreeRTOS 函数
// ============================================================
bool Serial::send_data_from_isr(uint8_t *data, uint16_t size, uint32_t tick)
{
    DataPack_t tx_pack;
    BaseType_t pxHigherPriorityTaskWoken;  // 任务切换标记

    // -------------------- 基本检查 --------------------
    if (size == 0 || !is_ready())
        return false;

    if (size > _config.tx_buf_size * 0.8)
        return false;

    if (_is_tx_busy(true))  // true = from_isr 模式
        return false;

    // -------------------- 获取信号量（中断安全版本） --------------------
    if (xSemaphoreTakeFromISR(_tx_buf_sem, &pxHigherPriorityTaskWoken) != pdTRUE)
        return false;

    // -------------------- 生成时间戳 --------------------
    if (tick == 0)
    {
        tx_pack.tick = HAL_System::get_tick_ms();
    }

    // -------------------- 复制数据到发送缓冲区 --------------------
    uint16_t rest_buf_size = _tx_buf.size - _tx_buf.index;

    if (size <= rest_buf_size)
    {
        memcpy(_tx_buf.pData, data, size);
        tx_pack.pData = _tx_buf.pData;
        tx_pack.size = size;

        // 放入队列（FromISR 版本）
        xQueueSendFromISR(_tx_queue, &tx_pack, &pxHigherPriorityTaskWoken);

        _tx_buf.index += size;

        if (_tx_buf.size - _tx_buf.index < _config.tx_pack_size)
        {
            _tx_buf.index = 0;
        }
        _tx_buf.pData = _tx_buf.pBuffer + _tx_buf.index;
    }
    else
    {
        memcpy(_tx_buf.pData, data, rest_buf_size);
        tx_pack.pData = _tx_buf.pData;
        tx_pack.size = rest_buf_size;

        xQueueSendFromISR(_tx_queue, &tx_pack, &pxHigherPriorityTaskWoken);

        uint16_t rest_pack_size = size - rest_buf_size;

        memcpy(_tx_buf.pBuffer, data + rest_buf_size, rest_pack_size);

        tx_pack.pData = _tx_buf.pBuffer;
        tx_pack.size = rest_pack_size;

        xQueueSendFromISR(_tx_queue, &tx_pack, &pxHigherPriorityTaskWoken);

        _tx_buf.index = rest_pack_size;
        _tx_buf.pData = _tx_buf.pBuffer + _tx_buf.index;
    }

    // -------------------- 更新统计 --------------------
    _update_info(0, size);

    // -------------------- 释放信号量 --------------------
    xSemaphoreGiveFromISR(_tx_buf_sem, &pxHigherPriorityTaskWoken);

    // -------------------- 触发任务切换 --------------------
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

    return true;
}

// ============================================================
// send_data_fast - 快速发送（直接调用 DMA/IT，不排队）
// 参数：
//   data - 数据指针
//   size - 数据长度
// 返回：true 成功
// 说明：
//   - 不检查忙碌状态，不排队，直接发送
//   - 如果上一次发送未完成，会覆盖正在发送的数据
//   - 适合实时性要求高的场景
// ============================================================
bool Serial::send_data_fast(uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0 || !is_ready())
        return false;

    if (_config.tx_mode == Serial_Mode::IT)
    {
        // IT 方式发送
        HAL_UART_Transmit_IT(_uart, data, size);
    }
    else if (_config.tx_mode == Serial_Mode::DMA)
    {
        // DMA 方式发送
        HAL_UART_Transmit_DMA(_uart, data, size);
    }

    return true;
}

// ============================================================
// print - 格式化打印（类似 printf，但通过串口发送）
// 参数：format - 格式化字符串（同 printf）
// 返回：发送的字符数，-1 表示失败
// 说明：
//   - 支持 %d、%f、%s 等格式化输出
//   - 内部使用 vsnprintf 格式化，然后调用 send_data
//   - 发送失败会重试 100 次 × 1ms
// ============================================================
int Serial::print(const char *format, ...)
{
    // 分配临时缓冲区存储格式化后的字符串
    uint8_t *print_buf = (uint8_t *)pvPortMalloc(SERIAL_PRINT_SIZE);

    if (print_buf == NULL)
        return -1;  // 内存分配失败

    // -------------------- 格式化字符串 --------------------
    va_list args;
    va_start(args, format);  // 开始解析可变参数
    // vsnprintf：安全版本的 snprintf，防止缓冲区溢出
    int len = vsnprintf((char *)print_buf, SERIAL_PRINT_SIZE, format, args);
    va_end(args);  // 结束解析

    // -------------------- 发送（失败重试） --------------------
    bool status;
    for (uint8_t i = 0; i < 100; i++)  // 最多重试 100 次
    {
        status = send_data(print_buf, len);  // 调用已有的 send_data
        if (status)
            break;  // 发送成功，退出重试循环
        vTaskDelay(pdMS_TO_TICKS(1));  // 等待 1ms 后重试
    }

    if (!status)
        len = -1;  // 100 次都失败，标记失败

    // -------------------- 释放临时缓冲区 --------------------
    vPortFree(print_buf);

    return len;
}

// ============================================================
// print_from_isr - 中断安全版本的格式化打印
// 说明：同 print，但可在中断中使用，不重试
// ============================================================
int Serial::print_from_isr(const char *format, ...)
{
    uint8_t *print_buf = (uint8_t *)pvPortMalloc(SERIAL_PRINT_SIZE);

    if (print_buf == NULL)
        return -1;

    // 格式化
    va_list args;
    va_start(args, format);
    int len = vsnprintf((char *)print_buf, SERIAL_PRINT_SIZE, format, args);
    va_end(args);

    // 发送（中断版本，不重试）
    bool status;
    status = send_data_from_isr(print_buf, len);

    if (!status)
        len = -1;

    vPortFree(print_buf);

    return len;
}

// ============================================================
// _rx_task - 接收任务主循环（FreeRTOS 任务入口）
// 说明：
//   - 阻塞等待接收队列
//   - 收到数据包后，调用用户注册的回调函数
// ============================================================
void Serial::_rx_task(void)
{
    DataPack_t pack;  // 本地数据包

    while (1)  // FreeRTOS 任务必须是无限循环
    {
        // -------------------- 阻塞等待接收队列 --------------------
        // xQueueReceive：从队列中取出数据包
        // portMAX_DELAY = 无限等待，直到队列有数据
        xQueueReceive(_rx_queue, &pack, portMAX_DELAY);

        // -------------------- 调用用户回调 --------------------
        if (_rx_msg_handler != NULL)
        {
            // 将数据包和 Serial 实例传给用户回调
            _rx_msg_handler(&pack, this, _rx_msg_handler_args);
        }
    }
}

// ============================================================
// _tx_task - 发送任务主循环（FreeRTOS 任务入口）
// 说明：
//   - 阻塞等待发送队列
//   - 收到数据包后，等待发送信号量，然后触发 DMA/IT 发送
// ============================================================
void Serial::_tx_task(void)
{
    DataPack_t pack;

    while (1)
    {
        // -------------------- 阻塞等待发送队列 --------------------
        xQueueReceive(_tx_queue, &pack, portMAX_DELAY);

        // -------------------- 等待发送资源 --------------------
        // 获取 _tx_src_sem，确保上一次发送已完成
        // 这样可以保证数据包按顺序发送，不会重叠
        xSemaphoreTake(_tx_src_sem, portMAX_DELAY);

        // -------------------- 触发发送 --------------------
        _tx_irq_start(&pack);
    }
}

// ============================================================
// _error_handler - 错误处理函数（用户可重写）
// 参数：error - 错误类型
// 说明：
//   - 当前为空实现，用户可自定义错误处理逻辑
//   - 例如：点亮错误指示灯、打印错误日志等
// ============================================================
void Serial::_error_handler(Serial_Error error)
{
    // 用户可在此实现：
    // - 记录错误日志
    // - 点亮错误 LED
    // - 进入安全模式等
}
