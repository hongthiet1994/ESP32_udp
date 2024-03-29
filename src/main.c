#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ssd1306_oled.h"
#include "i2c_lib.h"
#include "main.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi_dmm.h"
#include "http_server_dmm.h"
#include "eeprom_dmm.h"
#include "device_infor_dmm.h"
#include "debug.h"
#include "esp_wifi.h"
#include "udp_dmm.h"

/* Handles for the tasks create by main(). */
TaskHandle_t xTask_UDP_server = NULL;

void app_main()
{
	//Initialize NVS
	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {        
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);   
	count_restart();    
	ESP_LOGI(DEBUG_DATE_TIME,"COMPILED AT : %s",__TIME__);
	init_start_system();
    config_WiFi_infor_default();   
    initialise_WiFi(WIFI_MODE_STA);   
    start_web();    
	i2c_master_init();
	ssd1306_init();
	task_ssd1306_display_clear();	    
    display_string_debug("_EMBEDDEDSYSTEM_",16,1);
    xTaskCreatePinnedToCore(udp_server_task, "udp_server_task", 4096, NULL, 3, &xTask_UDP_server,1); 
}
