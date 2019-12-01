#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "driver/uart.h"
#include "lwip/sockets.h"
#include "udp_dmm.h"
#include "wifi_dmm.h"
#include "cJSON.h"
#include "ssd1306_oled.h"
#include "eeprom_dmm.h"
#include "debug.h"



struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
int sock;

//uint32_t ui32_receive_cmd_stop = false;

//extern EventGroupHandle_t wifi_event_group;

char rx_udp_buffer_backup[LEN_RX_BUFFER_UDP] = {0};
size_t len_buffer_udp_backup = 0;

void free_udp_buffer_backup()
{
    memset(rx_udp_buffer_backup, 0x00, LEN_RX_BUFFER_UDP);
}

void udp_server_task(void *pvParameters)
{
    char rx_buffer[LEN_RX_BUFFER_UDP];
    char addr_str[LEN_BUFFER_IP_ADDRESS];
    int addr_family;
    int ip_protocol;
    while (1)
    {
        #ifdef CONFIG_EXAMPLE_IPV4
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(UDP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
        #else // IPV6
        struct sockaddr_in6 destAddr;
        bzero(&destAddr.sin6_addr.un, sizeof(destAddr.sin6_addr.un));
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(PORT);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        #endif
        sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(DEBUG_UDP, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(DEBUG_UDP, "Socket created");
        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0)
        {
            ESP_LOGE(DEBUG_UDP, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(DEBUG_UDP, "Socket binded");
        while (1)
        {
            //struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);
            // Error occured during receiving
            if (len < 0)
            {
                ESP_LOGI(DEBUG_UDP, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else
            {
                // Get the sender's ip address as string
                if (sourceAddr.sin6_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                }
                else if (sourceAddr.sin6_family == PF_INET6)
                {
                    inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                len_buffer_udp_backup = len;
                memcpy(rx_udp_buffer_backup, rx_buffer, len_buffer_udp_backup);
                ESP_LOGI(DEBUG_UDP, "Received %d bytes from %s:", len, addr_str);
                sendto(sock,rx_buffer,len_buffer_udp_backup, 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                //ESP_LOGI(DEBUG_UDP, "%s", rx_buffer);
            }
        }
        if (sock != -1)
        {
            ESP_LOGE(DEBUG_UDP, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

