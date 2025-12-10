/*--------------------------------------------------------------------------
INTERRUPT.H

Header file for simplifying operations of C51 interrupt 
Created by Everett Rain (NUAA) 2025-11-22
Updated: Added Mode 2 (8-bit Auto-Reload) support 2025-12-10
Part of C51 Dev Tools Library - rac51

可使用的函数列表
    FOSC                    晶振频率（默认为 11.0592MHz，可使用 define 修改）

    [Timer 0 - 方式 1 (16位)]
    T0_INIT_TIMER           初始化 T0 为定时器 (16位)，传入计时间隔(ms)
    T0_INIT_COUNTER         初始化 T0 为计数器 (16位)，传入起始值
    T0_RELOAD_TIMER         重装载 T0 值 (16位)，用于中断内手动重装

    [Timer 0 - 方式 2 (8位自动重装)]
    T0_INIT_TIMER_MODE2     初始化 T0 为定时器 (8位自动重装)，传入计数值(0-255)
    T0_INIT_COUNTER_MODE2   初始化 T0 为计数器 (8位自动重装)，传入溢出计数值(N)

    [Timer 0 - 通用]
    T0_GET_VALUE            获取 T0 当前存储值
    T0_SET_VALUE            设定 T0 当前存储值

    ---

    [Timer 1 - 方式 1 (16位)]
    T1_INIT_TIMER           初始化 T1 为定时器 (16位)，传入计时间隔(ms)
    T1_INIT_COUNTER         初始化 T1 为计数器 (16位)，传入起始值
    T1_RELOAD_TIMER         重装载 T1 值 (16位)，用于中断内手动重装

    [Timer 1 - 方式 2 (8位自动重装)]
    T1_INIT_TIMER_MODE2     初始化 T1 为定时器 (8位自动重装)，传入计数值(0-255)
    T1_INIT_COUNTER_MODE2   初始化 T1 为计数器 (8位自动重装)，传入溢出计数值(N)

    [Timer 1 - 通用]
    T1_GET_VALUE            获取 T1 当前存储值
    T1_SET_VALUE            设定 T1 当前存储值
--------------------------------------------------------------------------*/

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <reg52.h>

// =================================================================
// 1. 基础参数配置 (可被外部 define 覆盖)
// =================================================================
#ifndef FOSC
    #define FOSC 11059200L  
#endif

// 计算每毫秒的机器周期数
// 公式: (FOSC / 12) / 1000
#define TICKS_PER_MS  (FOSC / 12000)

// 宏函数：根据毫秒数自动计算 16 位定时器的重装载值 (65536 - count)
#define CALC_RELOAD_16BIT(ms) (65536 - ((unsigned long)(ms) * TICKS_PER_MS))

// 宏函数：根据计数值自动计算 8 位自动重装载值 (256 - count)
// 适用于方式 2，最大计数值 256
#define CALC_RELOAD_8BIT(counts) (256 - (counts))

// =================================================================
// 2. TMOD 模式位掩码 (用于位运算)
// =================================================================
// T0 在低 4 位，T1 在高 4 位
// C/T: 0=定时器, 1=计数器

// --- Timer 0 掩码 ---
#define T0_MASK             0xF0    // 用于清除 T0 的配置 (保留 T1)
// 方式 1 (16位)
#define T0_MODE_TIMER_16    0x01    // M1=0, M0=1
#define T0_MODE_COUNTER_16  0x05    // C/T=1, M1=0, M0=1
// 方式 2 (8位自动重装)
#define T0_MODE_TIMER_8_AUTO   0x02 // M1=1, M0=0
#define T0_MODE_COUNTER_8_AUTO 0x06 // C/T=1, M1=1, M0=0

// --- Timer 1 掩码 ---
#define T1_MASK             0x0F    // 用于清除 T1 的配置 (保留 T0)
// 方式 1 (16位)
#define T1_MODE_TIMER_16    0x10    // M1=0, M0=1
#define T1_MODE_COUNTER_16  0x50    // C/T=1, M1=0, M0=1
// 方式 2 (8位自动重装)
#define T1_MODE_TIMER_8_AUTO   0x20 // M1=1, M0=0
#define T1_MODE_COUNTER_8_AUTO 0x60 // C/T=1, M1=1, M0=0

// =================================================================
// 3. Timer 0 (T0) 通用配置宏
// =================================================================

// --- 方式 1: 16位模式 ---

/// @brief 初始化 T0 为 [定时器模式 1] (16位)
/// @param ms: 定时时长(毫秒)
#define T0_INIT_TIMER(ms) \
    do { \
        TMOD &= T0_MASK;             /* 清除 T0旧设置 */ \
        TMOD |= T0_MODE_TIMER_16;    /* 设置为 16 位定时器 */ \
        TH0 = CALC_RELOAD_16BIT(ms) >> 8; \
        TL0 = CALC_RELOAD_16BIT(ms) & 0xFF; \
        ET0 = 1;                     /* 开启 T0 中断 */ \
        TR0 = 1;                     /* 启动 T0 */ \
        EA  = 1;                     /* 确保总中断开启 */ \
    } while(0)

/// @brief 初始化 T0 为 [计数器模式 1] (16位, 引脚 P3.4)
/// @param init_val: 初始计数值 (通常为0)
#define T0_INIT_COUNTER(init_val) \
    do { \
        TMOD &= T0_MASK;             /* 清除 T0旧设置 */ \
        TMOD |= T0_MODE_COUNTER_16;  /* 设置为 16 位计数器 */ \
        TH0 = (init_val) >> 8; \
        TL0 = (init_val) & 0xFF; \
        TR0 = 1;                     /* 启动监测 */ \
    } while(0)

// [T0 重装载]: 仅用于方式 1，需在中断内调用
#define T0_RELOAD_TIMER(ms) \
    do { \
        TH0 = CALC_RELOAD_16BIT(ms) >> 8; \
        TL0 = CALC_RELOAD_16BIT(ms) & 0xFF; \
    } while(0)

// --- 方式 2: 8位自动重装模式 ---

/// @brief 初始化 T0 为 [计数器模式 2] (8位自动重装, 引脚 P3.4)
/// @param counts: 每累计多少个脉冲产生一次中断 (1-256)
#define T0_INIT_COUNTER_MODE2(counts) \
    do { \
        TMOD &= T0_MASK;               /* 清除 T0 设置 */ \
        TMOD |= T0_MODE_COUNTER_8_AUTO;/* 设置为计数器+方式2 */ \
        TH0 = CALC_RELOAD_8BIT(counts);/* 自动重装载值 */ \
        TL0 = CALC_RELOAD_8BIT(counts);/* 当前计数值 */ \
        ET0 = 1;                       /* 开启 T0 中断 */ \
        TR0 = 1;                       /* 启动 T0 */ \
        EA  = 1;                       /* 开启总中断 */ \
    } while(0)

/// @brief 初始化 T0 为 [定时器模式 2] (8位自动重装)
/// @note 8位定时很短(通常us级)，常用于波特率生成
/// @param counts: 定时周期 (机器周期数, 1-256)
#define T0_INIT_TIMER_MODE2(counts) \
    do { \
        TMOD &= T0_MASK;               /* 清除 T0 设置 */ \
        TMOD |= T0_MODE_TIMER_8_AUTO;  /* 设置为定时器+方式2 */ \
        TH0 = CALC_RELOAD_8BIT(counts); \
        TL0 = CALC_RELOAD_8BIT(counts); \
        ET0 = 1;                       /* 开启 T0 中断 */ \
        TR0 = 1;                       /* 启动 T0 */ \
        EA  = 1;                       /* 开启总中断 */ \
    } while(0)

// [T0 读写操作]
#define T0_GET_VALUE()  ((unsigned int)(TH0 << 8) | TL0)
#define T0_SET_VALUE(v) do{ TH0=(v)>>8; TL0=(v)&0xFF; }while(0)


// =================================================================
// 4. Timer 1 (T1) 通用配置宏
// =================================================================

// --- 方式 1: 16位模式 ---

/// @brief 初始化 T1 为 [定时器模式 1] (16位)
/// @param ms: 定时时长(毫秒)
#define T1_INIT_TIMER(ms) \
    do { \
        TMOD &= T1_MASK;             /* 清除 T1旧设置 */ \
        TMOD |= T1_MODE_TIMER_16;    /* 设置为 16位定时器 */ \
        TH1 = CALC_RELOAD_16BIT(ms) >> 8; \
        TL1 = CALC_RELOAD_16BIT(ms) & 0xFF; \
        ET1 = 1;                     /* 开启 T1 中断 */ \
        TR1 = 1;                     /* 启动 T1 */ \
        EA  = 1;                     /* 确保总中断开启 */ \
    } while(0)

/// @brief 初始化 T1 为 [计数器模式 1] (16位, 引脚 P3.5)
/// @param init_val: 初始计数值 (通常为0)
#define T1_INIT_COUNTER(init_val) \
    do { \
        TMOD &= T1_MASK;             /* 清除 T1旧设置 */ \
        TMOD |= T1_MODE_COUNTER_16;  /* 设置为 16位计数器 */ \
        TH1 = (init_val) >> 8; \
        TL1 = (init_val) & 0xFF; \
        TR1 = 1;                     /* 启动监测 */ \
    } while(0)

// [T1 重装载]: 仅用于方式 1，需在中断内调用
#define T1_RELOAD_TIMER(ms) \
    do { \
        TH1 = CALC_RELOAD_16BIT(ms) >> 8; \
        TL1 = CALC_RELOAD_16BIT(ms) & 0xFF; \
    } while(0)

// --- 方式 2: 8位自动重装模式 ---

/// @brief 初始化 T1 为 [计数器模式 2] (8位自动重装, 引脚 P3.5)
/// @param counts: 每累计多少个脉冲产生一次中断 (1-256)
#define T1_INIT_COUNTER_MODE2(counts) \
    do { \
        TMOD &= T1_MASK;               /* 清除 T1 设置 */ \
        TMOD |= T1_MODE_COUNTER_8_AUTO;/* 设置为计数器+方式2 */ \
        TH1 = CALC_RELOAD_8BIT(counts);/* 自动重装载值 */ \
        TL1 = CALC_RELOAD_8BIT(counts);/* 当前计数值 */ \
        ET1 = 1;                       /* 开启 T1 中断 */ \
        TR1 = 1;                       /* 启动 T1 */ \
        EA  = 1;                       /* 开启总中断 */ \
    } while(0)

/// @brief 初始化 T1 为 [定时器模式 2] (8位自动重装)
/// @note 8位定时很短(通常us级)，常用于波特率生成
/// @param counts: 定时周期 (机器周期数, 1-256)
#define T1_INIT_TIMER_MODE2(counts) \
    do { \
        TMOD &= T1_MASK;               /* 清除 T1 设置 */ \
        TMOD |= T1_MODE_TIMER_8_AUTO;  /* 设置为定时器+方式2 */ \
        TH1 = CALC_RELOAD_8BIT(counts); \
        TL1 = CALC_RELOAD_8BIT(counts); \
        ET1 = 1;                       /* 开启 T1 中断 */ \
        TR1 = 1;                       /* 启动 T1 */ \
        EA  = 1;                       /* 开启总中断 */ \
    } while(0)

// [T1 读写操作]
#define T1_GET_VALUE()  ((unsigned int)(TH1 << 8) | TL1)
#define T1_SET_VALUE(v) do{ TH1=(v)>>8; TL1=(v)&0xFF; }while(0)

#endif