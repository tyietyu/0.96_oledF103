#include "esp8266.h"
#include "usart.h"
#include "OLED.h"

/*********************ESP8266_UART2********************************* */
static UART_HandleTypeDef g_uart_handle;
static uint8_t g_uart_tx_buf[ESP8266_UART_TX_BUF_SIZE];
static struct
{
    uint8_t buf[ESP8266_UART_RX_BUF_SIZE];
    struct
    {
        uint16_t len    : 15;
        uint16_t finsh  : 1;
    } sta;
} g_uart_rx_frame = {0};

void ESP8266_uart_init(uint32_t baudrate)
{
    g_uart_handle.Instance          = ESP8266_UART_INTERFACE;
    g_uart_handle.Init.BaudRate     = baudrate;
    g_uart_handle.Init.WordLength   = UART_WORDLENGTH_8B;
    g_uart_handle.Init.StopBits     = UART_STOPBITS_1;
    g_uart_handle.Init.Parity       = UART_PARITY_NONE;
    g_uart_handle.Init.Mode         = UART_MODE_TX_RX;
    g_uart_handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    g_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;
}

void ESP8266_uart_printf(char *fmt, ...)
{
    va_list ap;
    uint16_t len;

    va_start(ap, fmt);
    vsprintf((char *)g_uart_tx_buf, fmt, ap);
    va_end(ap);

    len = strlen((const char *)g_uart_tx_buf);
    HAL_UART_Transmit(&g_uart_handle, g_uart_tx_buf, len, HAL_MAX_DELAY);
}

void ESP8266_uart_rx_restart(void)
{
    g_uart_rx_frame.sta.len     = 0;
    g_uart_rx_frame.sta.finsh   = 0;
}

uint8_t *ESP8266_uart_rx_get_frame(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = '\0';
        return g_uart_rx_frame.buf;
    }
    else
    {
        return NULL;
    }
}

uint16_t ESP8266_uart_rx_get_frame_len(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        return g_uart_rx_frame.sta.len;
    }
    else
    {
        return 0;
    }
}

/*********************ESP8266********************************* */
uint8_t ESP8266_send_at_cmd(char *cmd, char *ack, uint32_t timeout)
{
    uint8_t *ret = NULL;

    ESP8266_uart_rx_restart();
    ESP8266_uart_printf("%s\r\n", cmd);

    if ((ack == NULL) || (timeout == 0))
    {
        return ESP8266_EOK;
    }
    else
    {
        while (timeout > 0)
        {
            ret = ESP8266_uart_rx_get_frame();

            if (ret != NULL)
            {
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return ESP8266_EOK;
                }
                else
                {
                    ESP8266_uart_rx_restart();
                }
            }

            timeout--;
            HAL_Delay(1);
        }

        return ESP8266_ETIMEOUT;
    }
}

uint8_t ESP8266_sw_reset(void)
{
    uint8_t ret;
    printf("1\r\n");
    ret = ESP8266_send_at_cmd("AT+RST", "OK", 500);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_at_test(void)
{
    uint8_t ret;
    uint8_t i;

    for (i = 0; i < 10; i++)
    {

        ret = ESP8266_send_at_cmd("AT", "OK", 500);

        if (ret == ESP8266_EOK)
        {
            return ESP8266_EOK;
        }
    }

    return ESP8266_ERROR;
}

uint8_t ESP8266_init(uint32_t baudrate)
{
    ESP8266_uart_init(baudrate);
    ESP8266_sw_reset();
    HAL_Delay(3000);

    if (ESP8266_at_test() != ESP8266_EOK)
    {
        return ESP8266_ERROR;
    }

    return ESP8266_EOK;
}

uint8_t ESP8266_restore(void)
{
    uint8_t ret;

    ret = ESP8266_send_at_cmd("AT+RESTORE", "ready", 3000);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}



uint8_t ESP8266_set_mode(uint8_t mode)
{
    uint8_t ret;

    switch (mode)
    {
        case 1:
        {
            ret = ESP8266_send_at_cmd("AT+CWMODE=1", "OK", 500);    /* Station模式 */
            break;
        }

        case 2:
        {
            ret = ESP8266_send_at_cmd("AT+CWMODE=2", "OK", 500);    /* AP模式 */
            break;
        }

        case 3:
        {
            ret = ESP8266_send_at_cmd("AT+CWMODE=3", "OK", 500);    /* AP+Station模式 */
            break;
        }

        default:
        {
            return ESP8266_EINVAL;
        }
    }

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_ate_config(uint8_t cfg)
{
    uint8_t ret;

    switch (cfg)
    {
        case 0:
        {
            ret = ESP8266_send_at_cmd("ATE0", "OK", 500);   /* 关闭回显 */
            break;
        }

        case 1:
        {
            ret = ESP8266_send_at_cmd("ATE1", "OK", 500);   /* 打开回显 */
            break;
        }

        default:
        {
            return ESP8266_EINVAL;
        }
    }

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_join_ap(char *ssid, char *pwd)
{
    uint8_t ret;
    char cmd[64];

    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    ret = ESP8266_send_at_cmd(cmd, "WIFI GOT IP", 10000);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_get_ip(char *buf)
{
    uint8_t ret;
    char *p_start;
    char *p_end;

    ret = ESP8266_send_at_cmd("AT+CIFSR", "OK", 500);

    if (ret != ESP8266_EOK)
    {
        return ESP8266_ERROR;
    }

    p_start = strstr((const char *)ESP8266_uart_rx_get_frame(), "\"");
    p_end = strstr(p_start + 1, "\"");
    *p_end = '\0';
    sprintf(buf, "%s", p_start + 1);
    return ESP8266_EOK;
}

uint8_t ESP8266_connect_tcp_server(char *server_ip, char *server_port)
{
    uint8_t ret;
    char cmd[64];

    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s", server_ip, server_port);
    ret = ESP8266_send_at_cmd(cmd, "CONNECT", 5000);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_enter_unvarnished(void)
{
    uint8_t ret;

    ret  = ESP8266_send_at_cmd("AT+CIPMODE=1", "OK", 500);
    ret += ESP8266_send_at_cmd("AT+CIPSEND", ">", 500);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

void ESP8266_exit_unvarnished(void)
{
    ESP8266_uart_printf("+++");
}

uint8_t ESP8266_connect_Ali_Clode(char *id, char *pwd)
{
    uint8_t ret;
    char cmd[64];

    sprintf(cmd, "AT+ATKCLDSTA=\"%s\",\"%s\"", id, pwd);
    ret = ESP8266_send_at_cmd(cmd, "CLOUD CONNECTED", 10000);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

uint8_t ESP8266_disconnect_Ali_Clode(void)
{
    uint8_t ret;

    ret = ESP8266_send_at_cmd("AT+ATKCLDCLS", "CLOUD DISCONNECT", 500);

    if (ret == ESP8266_EOK)
    {
        return ESP8266_EOK;
    }
    else
    {
        return ESP8266_ERROR;
    }
}

/*********************ESP8266_UARTCALLBACK********************************* */
void ESP8266_usart2Hander(void)
{
    uint8_t tmp;

    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_ORE) != RESET)        /* UART接收过载错误中断 */
    {
        __HAL_UART_CLEAR_OREFLAG(&g_uart_handle);                           /* 清除接收过载错误中断标志 */
        (void)g_uart_handle.Instance->SR;                                   /* 先读SR寄存器，再读DR寄存器 */
        (void)g_uart_handle.Instance->DR;
    }

    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_RXNE) != RESET)       /* UART接收中断 */
    {
        HAL_UART_Receive(&g_uart_handle, &tmp, 1, HAL_MAX_DELAY);           /* UART接收数据 */

        if (g_uart_rx_frame.sta.len < (ESP8266_UART_RX_BUF_SIZE - 1))   /* 判断UART接收缓冲是否溢出
                                                                             * 留出一位给结束符'\0'
                                                                             */
        {
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;             /* 将接收到的数据写入缓冲 */
            g_uart_rx_frame.sta.len++;                                      /* 更新接收到的数据长度 */
        }
        else                                                                /* UART接收缓冲溢出 */
        {
            g_uart_rx_frame.sta.len = 0;                                    /* 覆盖之前收到的数据 */
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;             /* 将接收到的数据写入缓冲 */
            g_uart_rx_frame.sta.len++;                                      /* 更新接收到的数据长度 */
        }
    }

    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_IDLE) != RESET)       /* UART总线空闲中断 */
    {
        g_uart_rx_frame.sta.finsh = 1;                                      /* 标记帧接收完成 */

        __HAL_UART_CLEAR_IDLEFLAG(&g_uart_handle);                          /* 清除UART总线空闲中断 */
    }
}
