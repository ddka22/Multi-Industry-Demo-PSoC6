/*
 * TFT.c
 *
 *  Created on: 07.06.2022
 *      Author: renes
 */

#include "st7789.h"

#define SWRESET 0x01
#define RDDID 0x04
#define RDDST 0x09
#define SLPOUT 0x11
#define INVON 0x21
#define INVOFF 0x20
#define DISPOFF 0x28
#define DISPON 0x29
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C
#define MADCTL 0x36
#define COLMOD 0x3A

#define NATIVE_DISPLAY_WIDTH 240
#define NATIVE_DISPLAY_HEIGHT 320
static uint16_t display_width = NATIVE_DISPLAY_WIDTH;
static uint16_t display_height = NATIVE_DISPLAY_HEIGHT;

#define ABS(x) ((x) > 0 ? (x) : -(x))

GPIO_PRT_Type *GPIO_PORT_9;
GPIO_PRT_Type *GPIO_PORT_5;
GPIO_PRT_Type *GPIO_PORT_7;
GPIO_PRT_Type *GPIO_PORT_12;

void TFT_HwReset()
{
	st7789_write_reset_pin(false);

	// wait 200 ms
	Cy_SysLib_Delay(200);

	st7789_write_reset_pin(true);
}

void TFT_SwReset()
{
	uint8_t color_mode[1] = {0x55}; // RGB 65K

	// needed for faster access
	GPIO_PORT_9 = Cy_GPIO_PortToAddr(9);
	GPIO_PORT_5 = Cy_GPIO_PortToAddr(5);
	GPIO_PORT_7 = Cy_GPIO_PortToAddr(7);
	GPIO_PORT_12 = Cy_GPIO_PortToAddr(12);


	st7789_write_command(SWRESET);

	// wait here 200ms
	Cy_SysLib_Delay(5);

	// leave sleep mode
	st7789_write_command(SLPOUT);

	// wait here 200ms
	Cy_SysLib_Delay(200);

	// set the color mode here
	st7789_write_command(COLMOD);
	st7789_write_data_stream(color_mode, sizeof(color_mode));
}

#define MY 0x80
#define MX 0x40
#define MV 0x20
void TFT_ConfigureRotation(uint16_t rotation)
{
	uint8_t data = 0;

	st7789_write_command(MADCTL);

	// configure MADCTL data
	if(0 == rotation)
	{
		// default
		data = 0x00 | MX | MY;
	}
	else if(90 == rotation)
	{
	}
	else if(180 == rotation)
	{
	}
	else if(270 == rotation)
	{
	}
	else
	{
	}

	st7789_write_data(data);
}

void TFT_ReadDisplayID(uint8_t *id)
{
	st7789_write_command(RDDID);
	st7789_read_data_stream(id, 10);
}

void TFT_ReadDisplayStatus(uint8 *status)
{
	st7789_write_command(RDDST);

	st7789_read_data_stream(status, 5);
}

void TFT_SetColorFormat(uint8_t format)
{
}

void TFT_InvertMode(bool mode)
{
	if(!mode)
	{
		st7789_write_command(INVOFF);
	}
	else
	{
		st7789_write_command(INVON);
	}
}

int TFT_SetWindow(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
	uint8_t col_para[4] = { (uint8_t)((left & 0xFF00) >> 8), (uint8_t)(left & 0x00FF), (uint8_t)((right & 0xFF00) >> 8), (uint8_t)(right & 0x00FF)};
	uint8_t row_para[4] = { (uint8_t)((top & 0xFF00) >> 8), (uint8_t)(top & 0x00FF), (uint8_t)((bottom & 0xFF00) >> 8), (uint8_t)(bottom & 0x00FF)};

	st7789_write_command(CASET);
	st7789_write_data_stream(col_para, 4);

	st7789_write_command(RASET);
	st7789_write_data_stream(row_para, 4);

	st7789_write_command(RAMWR);

	return 0;
}

void TFT_FillScreen(uint16_t color)
{
	int row_cnt, line_cnt;

	uint8_t local_color[2] = {(uint8_t)((color & 0xFF00) >> 8), (uint8_t)(color & 0x00FF)};

	TFT_SetWindow(0, 0, display_width - 1, display_height - 1);

	for(row_cnt = 0; row_cnt < display_height; row_cnt++)
	{
		for(line_cnt = 0; line_cnt < display_width; line_cnt++)
		{
			st7789_write_data_stream(local_color, sizeof(local_color));
		}
	}
}

void TFT_DisplayOn()
{
	st7789_write_command(DISPON);
}

void TFT_DisplayOff()
{
	st7789_write_command(DISPOFF);
}

void TFT_SetPixel(uint16_t x, uint16_t y, uint16_t color)
{
	uint8_t local_color[2] = {(uint8_t)((color & 0xFF00) >> 8), (uint8_t)(color & 0x00FF)};

	TFT_SetWindow(x, y, x, y);

	st7789_write_data_stream(local_color, sizeof(local_color));
}

void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	uint16_t swap;
    uint16_t steep = ABS(y1 - y0) > ABS(x1 - x0);
    if (steep) {
		swap = x0;
		x0 = y0;
		y0 = swap;

		swap = x1;
		x1 = y1;
		y1 = swap;
        //_swap_int16_t(x0, y0);
        //_swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
		swap = x0;
		x0 = x1;
		x1 = swap;

		swap = y0;
		y0 = y1;
		y1 = swap;
        //_swap_int16_t(x0, x1);
        //_swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = ABS(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
        	TFT_SetPixel(y0, x0, color);
        } else {
        	TFT_SetPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void TFT_UpdateArea(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, int16_t *buffer)
{
	int row_cnt, line_cnt;

	TFT_SetWindow(x0, y0, x1 - 1, y1 - 1);

	for(row_cnt = y0; row_cnt < y1; row_cnt++)
	{
		for(line_cnt = x0; line_cnt < x1; line_cnt++)
		{
			uint16_t value = *(buffer + 240 * row_cnt + line_cnt);
			uint8_t color[2] = {(uint8_t)((value & 0xFF00) >> 8), (uint8_t)(value & 0x00FF)};
			st7789_write_data_stream(color, sizeof(color));
		}
	}
}
