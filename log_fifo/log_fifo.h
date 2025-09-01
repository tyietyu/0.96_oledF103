#ifndef __LOG_FIFO__H
#define __LOG_FIFO__H

#include <stdint.h>

#define LOG_FIFO_DIRECT 0 // 直接打印
#define LOG_FIFO_CACHE  1 // 缓存打印
#define LOG_FIFO_INTACT 2 // 完整打印

struct _Log_device;
typedef struct _Log_device
{
    int32_t mode;            // 打印方式
    uint8_t *fifo;           // 缓冲区指针
    uint32_t fifo_len;       // 缓冲区大小
    uint32_t fifo_in;        // 当前写入位置
    uint32_t residue_size;   // 剩余空间
    int32_t (*write)(struct _Log_device *self, uint8_t *data, uint32_t len); // 写函数
    int32_t ret;             // 操作结果
    void *userData;          // 用户数据
} Log_dev_t;

int32_t Log_Init(Log_dev_t *self, int32_t mode, uint8_t *fifo, uint32_t fifo_len,
                 int32_t (*write)(struct _Log_device *self, uint8_t *data, uint32_t len),
                 void *userData);
int32_t Log_Start(Log_dev_t *self);
int32_t Log_Printf(Log_dev_t *self, const char *fmt, ...);
int32_t Log_Advanced_Printf(Log_dev_t *self, uint32_t send_len);
int32_t Log_End(Log_dev_t *self);
int32_t Log_Result(Log_dev_t *self);

#endif

