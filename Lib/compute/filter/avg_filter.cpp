#include "ex_math.hpp"   // 包含自定义数学工具头文件（可能包含一些辅助宏或函数）
#include "FreeRTOS.h"    // 包含 FreeRTOS 内核头文件，用于使用 pvPortMalloc 动态内存分配
#include "string.h"
// ============================================================
// 类名: AvgFilter
// 功能: 实现滑动窗口平均滤波，窗口满后自动去掉一个最大值和一个最小值，
//       再对剩余数据求平均，以消除异常值干扰。
//       内部使用 FreeRTOS 的 pvPortMalloc 动态申请环形缓冲区。
// ============================================================

// 初始化滤波器
// 参数 win_size : 滑动窗口大小（至少为2，因为需要去掉最大最小值后至少有1个数据）
// 返回值: true 表示初始化成功，false 表示失败（窗口太小或内存分配失败）
bool AvgFilter::init(uint8_t win_size)
{
    // 窗口太小（<2）无法进行去极值平均，返回失败
    if (win_size < 2)
        return false;

    _sum = 0.f;          // 清空窗口内数据总和
    _size = win_size;    // 记录窗口大小
    _index = 0;          // 环形缓冲区当前写入位置索引
    _count = 0;          // 当前已存储的数据个数（未满时 < _size）

    // 使用 FreeRTOS 的动态内存分配函数申请 float 类型的环形缓冲区
    pData = (float *)pvPortMalloc(sizeof(float) * _size);
    if (pData == NULL)   // 内存分配失败
        return false;
    memset(pData, 0, sizeof(float) * _size);// 清空缓冲区内容 防止随机值
    return true;         // 初始化成功
}

// 滤波核心函数：向滤波器输入一个新值，返回滤波后的结果
// 参数 value : 新采样的原始数据
// 返回值     : 滤波后的平均值
float AvgFilter::filter(float value)
{
    // 1. 更新数据计数（未满时增加，满了后保持为 _size）
    _count = _count < _size ? _count + 1 : _size;

    // 2. 更新总和：减去即将被覆盖的旧值，加上新值
    _sum = _sum - pData[_index] + value; //将被覆盖的最老数据从总和中剔除。

    // 3. 将新值存入当前槽位
    pData[_index] = value; 

    // 4. 环形缓冲区指针向前移动，并取模实现循环
    _index = (_index + 1) % _size;

    // 5. 初始化极值（用于后续查找最大最小值）
    _max = _min = value;

    // 6. 判断是否已填满窗口
    if (_count == _size)
    {
        // 窗口已满：遍历所有数据，找出最大值和最小值
        for (int i = 0; i < _size; i++)
        {
            if (pData[i] > _max)
                _max = pData[i];
            if (pData[i] < _min)
                _min = pData[i];
        }
        // 计算去极值平均：总和减去最大最小值，再除以 (窗口大小 - 2)
        _avg = (_sum - _max - _min) / (_size - 2);
    }
    else
    {
        // 窗口未满：直接计算普通平均值（总和 / 当前数据个数）
        _avg = _sum / _count;
    }

    // 返回滤波结果
    return _avg;
}

// 清空滤波器状态，重置所有内部变量，但不释放内存（保留已申请的缓冲区）
void AvgFilter::clear(void)
{
    _index = 0;          // 重置写入位置
    _count = 0;          // 重置数据计数
    _sum = 0.f;          // 重置总和
    _max = 0.f;          // 重置最大值（仅用于临时计算）
    _min = 0.f;          // 重置最小值（仅用于临时计算）
    memset(pData,0,sizeof(float)*_size); // 清空缓冲区内容
}


