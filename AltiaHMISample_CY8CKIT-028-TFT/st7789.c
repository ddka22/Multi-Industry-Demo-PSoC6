/*
 * st7789.c
 *
 *  Created on: 10.08.2022
 *      Author: renes
 */

/***************************************************************************//**
* \file st7789v.c
*
* \brief
*    This is display software i8080 interface source file (modified for
*    for better performance)
*
********************************************************************************
* \copyright
* Copyright 2018-2020 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "st7789.h"
#include "cyhal.h"

static const mtb_st7789v_pins_t *pins;


/*******************************************************************************
 * Changes data bus GPIO pins drive mode to digital Hi-Z with enabled input
 * buffer.
 *******************************************************************************/
static inline void data_io_set_input(void)
{
    /* enable input */
    cyhal_gpio_configure(pins->db08, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db09, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db10, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db11, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db12, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db13, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db14, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
    cyhal_gpio_configure(pins->db15, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE);
}


/*******************************************************************************
 * Changes data bus GPIO pins drive mode to strong drive with disabled input
 * buffer.
 *******************************************************************************/
static inline void data_io_set_output(void)
{
    /* enable output */
    cyhal_gpio_configure(pins->db08, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db09, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db10, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db11, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db12, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db13, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db14, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
    cyhal_gpio_configure(pins->db15, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG);
}


/*******************************************************************************
 * Writes one byte of data to the software i8080 interface.
 *******************************************************************************/

extern GPIO_PRT_Type *GPIO_PORT_9;
extern GPIO_PRT_Type *GPIO_PORT_5;
extern GPIO_PRT_Type *GPIO_PORT_7;
extern GPIO_PRT_Type *GPIO_PORT_12;

#define FAST_MODE

#define DATA_TO_PORT_9(x) (0x00 | (x & 0x07) | ((x & 0x18) << 1))
#define DATA_TO_PORT_5(x) (0x00 | (x & 0x20) << 2)
#define DATA_TO_PORT_7(x) (0x00 | (x & 0xC0) >> 1)

static inline void write_data(uint8_t data)
{
#ifdef SLOW_MODE
    cyhal_gpio_write(pins->db08,  data     & 0x01);
    cyhal_gpio_write(pins->db09, (data>>1) & 0x01);
    cyhal_gpio_write(pins->db10, (data>>2) & 0x01);
    cyhal_gpio_write(pins->db11, (data>>3) & 0x01);
    cyhal_gpio_write(pins->db12, (data>>4) & 0x01);
    cyhal_gpio_write(pins->db13, (data>>5) & 0x01);
    cyhal_gpio_write(pins->db14, (data>>6) & 0x01);
    cyhal_gpio_write(pins->db15, (data>>7) & 0x01);
#endif
#ifdef FAST_MODE
    GPIO_PORT_9->OUT = (uint32_t)DATA_TO_PORT_9(data);
    GPIO_PORT_5->OUT = (uint32_t)DATA_TO_PORT_5(data);
    GPIO_PORT_7->OUT = (uint32_t)DATA_TO_PORT_7(data);
#endif

#ifdef SLOW_MODE
    cyhal_gpio_write(pins->nwr, 0u);
    cyhal_gpio_write(pins->nwr, 1u);
#endif
#ifdef FAST_MODE
    GPIO_PORT_12->OUT_SET = 0x01;
    GPIO_PORT_12->OUT_CLR = 0x01;
#endif
}


/*******************************************************************************
 * Reads one byte of data from the software i8080 interface.
 *******************************************************************************/
static inline uint8_t read_data(void)
{
    uint8_t data = 0u;

    cyhal_gpio_write(pins->nrd, 0u);

    data |= cyhal_gpio_read(pins->db08);
    data |= cyhal_gpio_read(pins->db09) << 1;
    data |= cyhal_gpio_read(pins->db10) << 2;
    data |= cyhal_gpio_read(pins->db11) << 3;
    data |= cyhal_gpio_read(pins->db12) << 4;
    data |= cyhal_gpio_read(pins->db13) << 5;
    data |= cyhal_gpio_read(pins->db14) << 6;
    data |= cyhal_gpio_read(pins->db15) << 7;

    cyhal_gpio_write(pins->nrd, 1u);

    return data;
}


/*******************************************************************************
 * Initializes GPIOs for the software i8080 8-bit interface.
 *******************************************************************************/
cy_rslt_t st7789_init8(const mtb_st7789v_pins_t *data)
{
    pins = data;

    cy_rslt_t rslt =
        cyhal_gpio_init(pins->db08, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);

    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db09, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db10, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db11, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db12, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db13, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db14, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->db15, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->nwr, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->dc, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->rst, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1u);
    if (CY_RSLT_SUCCESS == rslt)
        rslt = cyhal_gpio_init(pins->nrd, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1u);
    return rslt;
}


/*******************************************************************************
 * Sets value of the display Reset pin.
 *******************************************************************************/
void st7789_write_reset_pin(bool value)
{
    cyhal_gpio_write(pins->rst, value);
}


/*******************************************************************************
 * Writes one byte of data to the software i8080 interface with the LCD_DC pin
 *******************************************************************************/
void st7789_write_command(uint8_t data)
{
    cyhal_gpio_write(pins->dc, 0u);
    write_data(data);
}


/*******************************************************************************
 * Writes one byte of data to the software i8080 interface with the LCD_DC pin
 *******************************************************************************/
void st7789_write_data(uint8_t data)
{
    cyhal_gpio_write(pins->dc, 1u);
    write_data(data);
}


/*******************************************************************************
 * Writes multiple command bytes to the software i8080 interface with the LCD_DC
 * pin set to 0.
 *******************************************************************************/
void st7789_write_command_stream(uint8_t *data, int num)
{
    int i;

    cyhal_gpio_write(pins->dc, 0u);

    for (i = 0; i < num; i++)
    {
        write_data(data[i]);
    }
}


/*******************************************************************************
 * Writes multiple bytes of data to the software i8080 interface with the LCD_DC
 * pin set to 1.
 *******************************************************************************/
void st7789_write_data_stream(uint8_t *data, int num)
{
    int i;

    cyhal_gpio_write(pins->dc, 1u);

    for (i = 0; i < num; i++)
    {
        write_data(data[i]);
    }
}


/*******************************************************************************
 * Reads one byte of data from the software i8080 interface with the LCD_DC pin
 * set to 1.
 *******************************************************************************/
uint8_t st7789_read_data(void)
{
    uint8_t data;

    cyhal_gpio_write(pins->dc, 1u);

    data_io_set_input();
    data = read_data();
    data_io_set_output();

    return data;
}


/*******************************************************************************
 * Reads multiple bytes of data from the software i8080 interface with the LCD_DC
 * pin set to 1.
 *******************************************************************************/
void st7789_read_data_stream(uint8_t *data, int num)
{
    cyhal_gpio_write(pins->dc, 1u);

    data_io_set_input();

    for (int i = 0; i < num; i++)
    {
        data[i] = read_data();
    }

    data_io_set_output();
}


/*******************************************************************************
 * Free all resources used for the software i8080 interface.
 *******************************************************************************/
void st7789_free(void)
{
    cyhal_gpio_free(pins->db08);
    cyhal_gpio_free(pins->db09);
    cyhal_gpio_free(pins->db10);
    cyhal_gpio_free(pins->db11);
    cyhal_gpio_free(pins->db12);
    cyhal_gpio_free(pins->db13);
    cyhal_gpio_free(pins->db14);
    cyhal_gpio_free(pins->db15);
    cyhal_gpio_free(pins->nwr);
    cyhal_gpio_free(pins->dc);
    cyhal_gpio_free(pins->rst);
    cyhal_gpio_free(pins->nrd);
}

/* [] END OF FILE */




