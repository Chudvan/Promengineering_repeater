
#include "serial_port_helper.h"

#include <sys/fcntl.h>
#include <driver/uart.h>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>

#include <esp_log.h>
#include <esp_vfs_dev.h>

// #include "../mavlink_helper.h"
// #include "../main.h"

#define TAG "DSS_serial"

serial_port_config_t serial_port_configs[3] =
{
	{.init = false,	.tx = GPIO_NUM_1,	.rx = GPIO_NUM_3},
	{.init = false,	.tx = GPIO_NUM_5,	.rx = GPIO_NUM_4},
	{.init = false,	.tx = GPIO_NUM_17,	.rx = GPIO_NUM_16},
};

serial_port_handler_t* serial_port_handlers;
int serial_port_handlers_size = 0;


void serial_port_init(const serial_port_num serial_num)
{
	int serial_socket;
	/*
	uart_config_t uart_config = // настройки Павла
	{ 
		.baud_rate = 420000,//serial_num == OSD ? SERIAL_PORT_RATE_OSD : SERIAL_PORT_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT
	}; */

	uart_config_t uart_config = { // настройки с сайта
        .baud_rate = 115200, // Скорость передачи
        .data_bits = UART_DATA_8_BITS, // Биты данных
        .parity = UART_PARITY_DISABLE, // проверка четности
        .stop_bits = UART_STOP_BITS_1, // Стоповые биты
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS, // управление потоком
		.source_clk = UART_SCLK_DEFAULT,
        .rx_flow_ctrl_thresh = 122, // Аппаратный порог RTS
	};
	ESP_ERROR_CHECK(uart_param_config(serial_num, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(serial_num, serial_port_configs[serial_num].tx, serial_port_configs[serial_num].rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	//ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 18, 19));

	const int uart_buffer_size = (1024 * 2);
	QueueHandle_t uart_queue;
	// ESP_ERROR_CHECK(uart_driver_install(serial_num, 1024, 0, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

	char serial_path[12];
    snprintf(serial_path, 12, "/dev/uart/%d", serial_num);

	if ((serial_socket = open(serial_path, O_RDWR)) == -1)
	{
		ESP_LOGE(TAG, "Cannot open UART %i", serial_num);
		close(serial_socket);
		uart_driver_delete(serial_num);
		return;
	}

	serial_port_configs[serial_num].init = true;
}

// Input - Output //

// int serial_port_write(const serial_port_num serial_num, const uint8_t* buffer, const size_t* length)
// {
// 	int written = uart_write_bytes(serial_num, buffer, *length);

// 	if (serial_num == PIXHAWK) mvl_handler_read_raw(MAVLINK_COMM_1, buffer, length);

// 	if (written > 0)
// 	{
// 		ESP_LOGD(TAG, "Wrote %i bytes to serial %i", written, serial_num);
// 	}
// 	else
// 	{
// 		ESP_LOGE(TAG, "Error writing to serial %i: %s", serial_num, ""); //esp_err_to_name(errno));
// 	}

// 	return written;
// }

void serial_port_read()
{
	while (1)
	{
		// for (int serial_num = 0; serial_num < SERIAL_PORT_COUNT; ++serial_num)
		{
			/* int serial_num = 2; */
			if (serial_port_configs[serial_num].init)
			{
				serial_data_t sd;
				/*
				uint8_t data[128];
				int length = 0;
				ESP_ERROR_CHECK(uart_get_buffered_data_len(serial_num, (size_t*)&sd.length));
				length = uart_read_bytes(uart_num, data, length, 100);
				*/
				ESP_ERROR_CHECK(uart_get_buffered_data_len(serial_num, (size_t*)&sd.length));
				if (sd.length > 0)
				//if ((sd.length = uart_read_bytes(serial_num, sd.buffer, SERIAL_PORT_RD_BYTES_NUM, 200 / portTICK_PERIOD_MS)) > 0)
				{
					uart_read_bytes(serial_num, sd.buffer, SERIAL_PORT_RD_BYTES_NUM, 200 / portTICK_PERIOD_MS);
					for (int task_num = 0; task_num < serial_port_handlers_size; ++task_num)
					{
						if (serial_port_handlers[task_num].serial_num == serial_num) (* serial_port_handlers[task_num].task)(&sd);
					}
				}
			}
			else vTaskDelay(200 / portTICK_PERIOD_MS);
		}
		// vTaskDelay(1);
	}
}

// Handlers //

void serial_port_handler_add(const serial_port_num serial_num, const SPFunction_t task)
{
	if (++serial_port_handlers_size == 1)
		serial_port_handlers = malloc(sizeof(serial_port_handler_t));
	else
	{
		serial_port_handler_t* temp = realloc(serial_port_handlers, sizeof(serial_port_handler_t) * serial_port_handlers_size);
		if (temp == NULL)
		{
			ESP_LOGE(TAG, "Error add new handler - cant realloc!");
			return;
		}
		serial_port_handlers = temp;
	}

	serial_port_handlers[serial_port_handlers_size - 1].serial_num = serial_num;
	serial_port_handlers[serial_port_handlers_size - 1].task = task;

	ESP_LOGI(TAG, "Add new handler to serial %i", serial_num);
}

// void serial_port_handler_del(TaskFunction_t handler)
// {
// 	for (int i = 0; i < handlers_count; ++i)
// 	{
// 		if (handlers[i] == handler)
// 		{
// 			TaskFunction_t* temp = malloc(sizeof(TaskFunction_t) * (handlers_count - 1));
// 			uint8_t temp_count = 0;
// 			for (int i = 0; i < handlers_count - 1; ++i)
// 			{
// 				if (handlers[i] != handler) temp[temp_count++] = handlers[i];
// 			}

// 			free(handlers);
// 			handlers = temp;
// 			--handlers_count;
// 			return;
// 		}
// 	}

// 	ESP_LOGD(TAG1, "Handler to delete not found");
// }


void serial_port_stream_start()
{
	xTaskCreate(&serial_port_read, "my_serial_port", 8000, NULL, 5, NULL);
}