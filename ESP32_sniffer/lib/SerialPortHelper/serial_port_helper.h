#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERIAL_PORT_RATE 115200
#define SERIAL_PORT_RATE_OSD 57600
#define SERIAL_PORT_RD_BYTES_NUM 8
/* #define SERIAL_PORT_COUNT 3 */

typedef enum
{
	SERIAL_0 = 0,
	SERIAL_1 = 1,
	SERIAL_2 = 2
}
serial_port_num;

extern serial_port_num serial_num;

typedef struct { bool init; int tx, rx; bool havaHandlers; } serial_port_config_t;

typedef struct
{
	uint8_t buffer[SERIAL_PORT_RD_BYTES_NUM];
	size_t length;
}
serial_data_t;

typedef void ( * SPFunction_t ) (const serial_data_t *);

typedef struct
{
	serial_port_num serial_num;
	SPFunction_t task;
}
serial_port_handler_t;

void serial_port_init(serial_port_num serial_num);

// Input - Output //

int serial_port_write(const serial_port_num serial_num, const uint8_t* buffer, const size_t* length);
void serial_port_read();

// Handlers //

void serial_port_handler_add(const serial_port_num serial_num, const SPFunction_t task);
void serial_port_handler_del(const serial_port_num serial_num, const SPFunction_t task);


void serial_port_stream_start();