#include <stdio.h>
#include <esp_log.h>

#include "serial_port_helper.h"

//extern serial_port_num serial_num;

uint8_t buf[8];

serial_port_num serial_num = SERIAL_2;

//static const char* TAG = "main Module";

void hex_to_log(const serial_data_t* serial_data)
{
	for (int i = 0; i < serial_data->length; ++i)
	{
		buf[i % 8] = serial_data->buffer[i];

		if (i % 8 == 7) ESP_LOGI("", "%02X : %02X : %02X : %02X : %02X : %02X : %02X : %02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);	
	}
}

void app_main()
{

	ESP_LOGI("", "Starting app_main");
	serial_port_init(serial_num);

	serial_port_handler_add(serial_num, &hex_to_log);

	ESP_LOGI("", "serial_port_stream_start");
	serial_port_stream_start();
	ESP_LOGI("", "End of app_main");
}