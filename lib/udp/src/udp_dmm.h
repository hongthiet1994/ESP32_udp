#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_event_loop.h"
#include "esp_log.h"

#define  DEBUG_UDP                         "UDP_DEBUG"
#define LEN_RX_BUFFER_UDP                  2048
#define LEN_BUFFER_IP_ADDRESS              128


#define TIME_SEND_UDP_DEBUG        10000

#define UDP_PORT                   50001
#define CONFIG_EXAMPLE_IPV4

 esp_err_t event_handler(void *ctx, system_event_t *event);

 void udp_server_task(void *pvParameters);
 