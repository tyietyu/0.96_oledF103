
#include "Log_FIFO.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

// 内部辅助函数：安全写入缓冲区
static int32_t safe_vsnprintf(Log_dev_t *self, const char *fmt, va_list arg_ptr)
{
    // 确保有空间存放终止符
    uint32_t max_write = (self->residue_size > 0) ? self->residue_size - 1 : 0;
    int32_t len = vsnprintf((char *)self->fifo + self->fifo_in, max_write + 1, fmt, arg_ptr);
    // 处理缓冲区不足的情况
    if (len < 0)
    {
        return -1; // 格式化失败
    }
    // 如果写入长度超过可用空间
    if ((uint32_t)len > max_write)
    {
        // 仅写入可用空间部分
        len = max_write;
        // 确保字符串终止
        self->fifo[self->fifo_in + max_write] = '\0';
    }
    return len;
}

int32_t Log_Init(Log_dev_t *self, int32_t mode, uint8_t *fifo, uint32_t fifo_len,
                 int32_t (*write)(struct _Log_device *self, uint8_t *data, uint32_t len),
                 void *userData)
{
    // 参数检查
    if (!self || !fifo || fifo_len < 16 || !write)
    {
        return -1; // 无效参数
    }
    self->mode = mode;
    self->fifo = fifo;
    self->fifo_len = fifo_len;
    self->fifo_in = 0;
    self->residue_size = fifo_len;
    self->write = write;
    self->ret = 0;
    self->userData = userData;
    return 0;
}

int32_t Log_Start(Log_dev_t *self)
{
    if (!self)
        return -1;
    self->fifo_in = 0;
    self->residue_size = self->fifo_len;
    self->ret = 0;
    // 只需清除第一个字节，避免全缓冲区清零的性能开销
    self->fifo[0] = '\0';
    return 0;
}

int32_t Log_Printf(Log_dev_t *self, const char *fmt, ...)
{
    if (!self || !fmt)
        return -1;
    // 如果之前操作失败，直接返回错误
    if ((self->mode != LOG_FIFO_DIRECT) && (self->ret < 0))
    {
        return self->ret;
    }
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    int32_t dataStr_len = safe_vsnprintf(self, fmt, arg_ptr);
    va_end(arg_ptr);
    if (dataStr_len < 0)
    {
        self->ret = -101; // 格式化错误
        return -101;
    }
    // 更新缓冲区状态
    self->fifo_in += dataStr_len;
    self->residue_size -= dataStr_len;
    // 模式处理
    switch (self->mode)
    {
    case LOG_FIFO_DIRECT:
        return Log_End(self);
    case LOG_FIFO_CACHE:
        // 当剩余空间不足总空间的20%时自动刷新
        if (self->residue_size < (self->fifo_len / 5))
        {
            return Log_Advanced_Printf(self, 0);
        }
        break;
    case LOG_FIFO_INTACT:
        // 当剩余空间不足时发送完整日志
        if (self->residue_size <= 1)
        {
            self->fifo[self->fifo_in] = '\0';
            int32_t res = self->write(self, self->fifo, self->fifo_in);
            if (res < 0)
            {
                self->ret = -1;
                return -1;
            }
            // 重置缓冲区
            self->fifo_in = 0;
            self->residue_size = self->fifo_len;
            // 重新写入当前日志
            va_start(arg_ptr, fmt);
            dataStr_len = safe_vsnprintf(self, fmt, arg_ptr);
            va_end(arg_ptr);
            if (dataStr_len < 0)
            {
                self->ret = -101;
                return -101;
            }
            self->fifo_in += dataStr_len;
            self->residue_size -= dataStr_len;
        }
        break;
    }
    return dataStr_len;
}

int32_t Log_Advanced_Printf(Log_dev_t *self, uint32_t send_len)
{
    if (!self || self->fifo_in == 0)
        return -1;
    if (self->mode == LOG_FIFO_INTACT && send_len != 0)
    {
        return -2; // 无效参数
    }
    // 确定发送长度
    uint32_t actual_send = (send_len == 0 || send_len > self->fifo_in) ? self->fifo_in : send_len;
    // 临时保存被覆盖的字符
    char temp_char = self->fifo[actual_send];
    self->fifo[actual_send] = '\0';
    // 发送数据
    int32_t res = self->write(self, self->fifo, actual_send);
    // 恢复字符
    self->fifo[actual_send] = temp_char;
    if (res < 0)
    {
        self->ret = -1;
        return -1;
    }
    // 移动剩余数据
    if (actual_send < self->fifo_in)
    {
        memmove(self->fifo, self->fifo + actual_send, self->fifo_in - actual_send);
    }
    // 更新缓冲区状态
    self->fifo_in -= actual_send;
    self->residue_size += actual_send;
    return actual_send;
}

int32_t Log_End(Log_dev_t *self)
{
    if (!self)
        return -1;
    if (self->fifo_in > 0)
    {
        // 确保字符串终止
        self->fifo[self->fifo_in] = '\0';
        int32_t res = self->write(self, self->fifo, self->fifo_in);
        if (res < 0)
        {
            self->ret = -1;
            return -1;
        }
    }
    // 重置缓冲区
    self->fifo_in = 0;
    self->residue_size = self->fifo_len;
    return 0;
}

int32_t Log_Result(Log_dev_t *self)
{
    return self ? self->ret : -1;
}

/************************system log test********************************/
void system_log_test(void)
{
    #include <stdio.h>
    #include "Log_FIFO.h"
    int32_t log_write(struct _Log_device * self, uint8_t *data, uint32_t len)
    {
        printf(">> Write %d bytes: [%.*s]\n", len, len, (char *)data);
        return len; // 返回实际写入长度
    }
    void run_test(Log_dev_t * log_dev, int mode, const char *mode_name)
    {
        printf("\n===== Testing %s Mode =====\n", mode_name);
        Log_Init(log_dev, mode, log_buff, sizeof(log_buff), log_write, NULL);
        Log_Start(log_dev);
        for (int i = 1; i <= 5; i++)
        {
            Log_Printf(log_dev, "Log_%d ", i);
        }
        if (mode == LOG_FIFO_CACHE)
        {
            Log_Advanced_Printf(log_dev, 10); // 测试部分发送
        }
        Log_Printf(log_dev, "Final_Message");
        Log_End(log_dev);
        printf("Result: %s\n", Log_Result(log_dev) >= 0 ? "SUCCESS" : "FAILURE");
    }
    /*
    int main()
    {
        static uint8_t log_buff[30];
        Log_dev_t log_dev;
        run_test(&log_dev, LOG_FIFO_DIRECT, "Direct");
        run_test(&log_dev, LOG_FIFO_CACHE, "Cache");
        run_test(&log_dev, LOG_FIFO_INTACT, "Intact");
        return 0;
    }
    */
}
