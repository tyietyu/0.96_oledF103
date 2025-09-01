#ifndef __LOG_FIFO__H
#define __LOG_FIFO__H

#include <stdint.h>

#define LOG_FIFO_DIRECT 0 // ֱ�Ӵ�ӡ
#define LOG_FIFO_CACHE  1 // �����ӡ
#define LOG_FIFO_INTACT 2 // ������ӡ

struct _Log_device;
typedef struct _Log_device
{
    int32_t mode;            // ��ӡ��ʽ
    uint8_t *fifo;           // ������ָ��
    uint32_t fifo_len;       // ��������С
    uint32_t fifo_in;        // ��ǰд��λ��
    uint32_t residue_size;   // ʣ��ռ�
    int32_t (*write)(struct _Log_device *self, uint8_t *data, uint32_t len); // д����
    int32_t ret;             // �������
    void *userData;          // �û�����
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

