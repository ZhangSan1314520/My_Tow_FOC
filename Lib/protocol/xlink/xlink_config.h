#pragma once

#define XLINK_SYSTEM_ID_NONE 0             // 无效系统
#define XLINK_SYSTEM_ID_AVIONICS_BAY 1     // 电子舱系统
#define XLINK_SYSTEM_ID_EO_POD 2           // 光电吊舱系统
#define XLINK_SYSTEM_ID_GROUND_STATION 100 // 地面站系统
#define XLINK_SYSTEM_ID_BROADCAST 255      // 广播系统

#define XLINK_COMPONENT_ID_NONE 0             // 无效组件
#define XLINK_COMPONENT_ID_MAIN_CONTROLLER 1  // 主控单元
#define XLINK_COMPONENT_ID_YAW_CONTROLLER 1   // 方位轴控制单元
#define XLINK_COMPONENT_ID_PITCH_CONTROLLER 2 // 俯仰轴控制单元
#define XLINK_COMPONENT_ID_GCS_APP 88         // 地面站软件
#define XLINK_COMPONENT_ID_BROADCAST 255      // 广播组件

// 本系统ID
#define XLINK_SELF_SYSTEM_ID XLINK_SYSTEM_ID_EO_POD
// 本组件ID
#define XLINK_SELF_COMPONENT_ID XLINK_COMPONENT_ID_YAW_CONTROLLER

#ifndef XLINK_SELF_SYSTEM_ID
#error "Please define XLINK_SELF_SYSTEM_ID in xlink_config.h"
#endif
#ifndef XLINK_SELF_COMPONENT_ID
#error "Please define XLINK_SELF_COMPONENT_ID in xlink_config.h"
#endif

// ------------------------------------------------------------------
// | ID      | System ID | Component ID | Description
// ------------------------------------------------------------------
// | 0-0     | 0         | 0            | 无效系统
// | 1-1     | 1         | 1            | 电子舱系统 - 主控单元
// | 2-1     | 2         | 1            | 光电吊舱系统 - 方位轴控制单元
// | 2-2     | 2         | 2            | 光电吊舱系统 - 俯仰轴控制单元
// | 100-88  | 100       | 88           | 地面站系统 - 地面站软件
// | 255-255 | 255       | 255          | 广播系统 - 广播组件
// ------------------------------------------------------------------

#define XLINK_CHANNEL_USER XLINK_CHANNEL_0
#define XLINK_CHANNEL_PITCH XLINK_CHANNEL_1