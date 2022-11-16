/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Empty PSoC6 Application
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#include "cy_retarget_io.h"

#include "cy8ckit_028_tft_pins.h" /* This is part of the CY8CKIT-028-TFT shield library. */

#include "st7789.h"
#include "TFT.h"

#include "miniGL.h"
#include "miniGL_software_render.h"

#define RGB565_DISPLAY_LAYER
#ifdef ARGB8888_DISPLAY_LAYER
MINIGL_UINT backBuffer[320 * 240] = {0};
#endif

#ifdef RGB565_DISPLAY_LAYER
MINIGL_INT16 backBuffer[320 * 240] = {0};
#endif

void * bsp_GetBackFrameBuffer()
{
	return backBuffer;
}

uint16_t ARGB8888_TO_RGB565(MINIGL_UINT value)
{
	uint16_t red = (value & 0x00F80000) >> 19;
	uint16_t green = (value & 0x0000FC00) >> 10;
	uint16_t blue = (value & 0x000000F8) >> 3;

	return ((red << 11) | (green << 5) | blue);
}

MINIGL_INT bsp_DriverFlushed(MINIGL_CONST MINIGL_INT16 dirtyExtentCount, MINIGL_CONST BSP_CLIPPING_RECTANGLE * dirtyExtentList)
{
	MINIGL_INT16 extentCount;

	for(extentCount = 0; extentCount < dirtyExtentCount; extentCount++)
	{
		TFT_UpdateArea(dirtyExtentList[extentCount].x0, dirtyExtentList[extentCount].y0, dirtyExtentList[extentCount].x1, dirtyExtentList[extentCount].y1, backBuffer);
	}

	return 0;
}

extern uint32_t __altia_table_start__;
MINIGL_UINT8 *altiaTable = (MINIGL_UINT8 *)&__altia_table_start__;

extern uint32_t __altia_images_start__;
MINIGL_UINT8 *altiaImages = (MINIGL_UINT8 *)&__altia_images_start__;

extern uint32_t __altia_fonts_start__;
MINIGL_UINT8 *altiaFonts = (MINIGL_UINT8 *)&__altia_fonts_start__;

MINIGL_UINT8 * bsp_ReflashQueryString(MINIGL_CONST MINIGL_UINT16 * string)
{
	MINIGL_CONST MINIGL_UINT8 *address = NULL;

	char local_string[128] = {0};
	MINIGL_UINT32 count = 0;

	if(string == NULL)
	{
		return altiaTable;
	}

	while((count < 128) && (string[count] != 0x0000))
	{
		local_string[count] = (MINIGL_UINT8)string[count];
		count++;
	}

	if(0 == strcmp(local_string, "\\images"))
	{
		address = altiaImages;
	}
#if 1
	else if(0 == strcmp(local_string, "\\fonts"))
	{
		address = altiaFonts;
	}
#endif
    return (MINIGL_UINT8 *)address;
}

int global_jump_to_screen = 0;
int current_screen = 0;


/* The pins above are defined by the CY8CKIT-028-TFT library. If the display is being used on different hardware the mappings will be different. */
const mtb_st7789v_pins_t tft_pins =
{
    .db08 = CY8CKIT_028_TFT_PIN_DISPLAY_DB8,
    .db09 = CY8CKIT_028_TFT_PIN_DISPLAY_DB9,
    .db10 = CY8CKIT_028_TFT_PIN_DISPLAY_DB10,
    .db11 = CY8CKIT_028_TFT_PIN_DISPLAY_DB11,
    .db12 = CY8CKIT_028_TFT_PIN_DISPLAY_DB12,
    .db13 = CY8CKIT_028_TFT_PIN_DISPLAY_DB13,
    .db14 = CY8CKIT_028_TFT_PIN_DISPLAY_DB14,
    .db15 = CY8CKIT_028_TFT_PIN_DISPLAY_DB15,
    .nrd  = CY8CKIT_028_TFT_PIN_DISPLAY_NRD,
    .nwr  = CY8CKIT_028_TFT_PIN_DISPLAY_NWR,
    .dc   = CY8CKIT_028_TFT_PIN_DISPLAY_DC,
    .rst  = CY8CKIT_028_TFT_PIN_DISPLAY_RST
};

cy_stc_scb_uart_context_t debug_uart_context;

/* This structure is used to initialize a single GPIO pin using PDL configuration. */
const cy_stc_gpio_pin_config_t P0_4_Pin_Init =
{
	.outVal     = 1u,                   /* Pin output state */
	.driveMode  = CY_GPIO_DM_PULLUP,    /* Drive mode */
	.hsiom      = HSIOM_SEL_GPIO,       /* HSIOM selection */
	.intEdge    = CY_GPIO_INTR_FALLING, /* Interrupt Edge type */
	.intMask    = CY_GPIO_INTR_EN_MASK, /* Interrupt enable mask */
	.vtrip      = CY_GPIO_VTRIP_CMOS,   /* Input buffer voltage trip type */
	.slewRate   = CY_GPIO_SLEW_FAST,    /* Output buffer slew rate */
	.driveSel   = CY_GPIO_DRIVE_FULL,   /* Drive strength */
	.vregEn     = 0u,                   /* SIO pair output buffer mode */
	.ibufMode   = 0u,                   /* SIO pair input buffer mode */
	.vtripSel   = 0u,                   /* SIO pair input buffer trip point */
	.vrefSel    = 0u,                   /* SIO pair reference voltage for input buffer trip point */
	.vohSel     = 0u                    /* SIO pair regulated voltage output level */
};

const cy_stc_gpio_pin_config_t P1_4_Pin_Init =
{
	.outVal     = 1u,                   /* Pin output state */
	.driveMode  = CY_GPIO_DM_PULLUP,    /* Drive mode */
	.hsiom      = HSIOM_SEL_GPIO,       /* HSIOM selection */
	.intEdge    = CY_GPIO_INTR_FALLING, /* Interrupt Edge type */
	.intMask    = CY_GPIO_INTR_EN_MASK, /* Interrupt enable mask */
	.vtrip      = CY_GPIO_VTRIP_CMOS,   /* Input buffer voltage trip type */
	.slewRate   = CY_GPIO_SLEW_FAST,    /* Output buffer slew rate */
	.driveSel   = CY_GPIO_DRIVE_FULL,   /* Drive strength */
	.vregEn     = 0u,                   /* SIO pair output buffer mode */
	.ibufMode   = 0u,                   /* SIO pair input buffer mode */
	.vtripSel   = 0u,                   /* SIO pair input buffer trip point */
	.vrefSel    = 0u,                   /* SIO pair reference voltage for input buffer trip point */
	.vohSel     = 0u                    /* SIO pair regulated voltage output level */
};

/* This structure initializes the Port0 interrupt for the NVIC */
cy_stc_sysint_t intrCfg0_4 =
{
	.intrSrc = ioss_interrupts_gpio_0_IRQn, /* Interrupt source is GPIO port 0 and 1 interrupt */
	.intrPriority = 2UL                     /* Interrupt priority is 2 */
};

cy_stc_sysint_t intrCfg1_4 =
{
	.intrSrc = ioss_interrupts_gpio_1_IRQn, /* Interrupt source is GPIO port 0 and 1 interrupt */
	.intrPriority = 2UL                     /* Interrupt priority is 2 */
};


#define GPIO_ISR_FLAG           (1u)
#define GPIO_ISR_MASKED         (1u)
#define INTCAUSE0_PORT0         (1u)
uint32_t screenChange = 0;
uint32_t aboutTrigger = 0;
uint32_t aboutScreenShown = 0;

void GPIO_Interrupt()
{
	   uint32 portIntrStatus;

	        /* Check to determine which pin in the port generated interrupt. */
	        portIntrStatus = P0_4_PORT->INTR;
	        if(CY_GPIO_INTR_STATUS_MASK == ((portIntrStatus >> P0_4_NUM) & CY_GPIO_INTR_STATUS_MASK))
	        {
	        	screenChange = GPIO_ISR_FLAG;

	    	    /* Clear pin interrupt logic. Required to detect next interrupt */
	    	    Cy_GPIO_ClearInterrupt(P0_4_PORT, P0_4_NUM);
	        }

	        /* Check to determine which pin in the port generated interrupt. */
	        portIntrStatus = P1_4_PORT->INTR;
	        if(CY_GPIO_INTR_STATUS_MASK == ((portIntrStatus >> P1_4_NUM) & CY_GPIO_INTR_STATUS_MASK))
	        {
	        	aboutTrigger = GPIO_ISR_FLAG;

	    	    /* Clear pin interrupt logic. Required to detect next interrupt */
	    	    Cy_GPIO_ClearInterrupt(P1_4_PORT, P1_4_NUM);
	        }
}

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    __enable_irq();

    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    /* Initialize the display controller */
    result = st7789_init8(&tft_pins);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* init user buttons */
    Cy_GPIO_Pin_Init(P0_4_PORT, P0_4_NUM, &P0_4_Pin_Init);
    Cy_GPIO_Pin_Init(P1_4_PORT, P1_4_NUM, &P1_4_Pin_Init);

    /* Configure GPIO pin to generate interrupts */
    Cy_GPIO_SetInterruptEdge(P0_4_PORT, P0_4_NUM, CY_GPIO_INTR_RISING);
    Cy_GPIO_SetInterruptMask(P0_4_PORT, P0_4_NUM, CY_GPIO_INTR_EN_MASK);
    Cy_GPIO_SetInterruptEdge(P1_4_PORT, P1_4_NUM, CY_GPIO_INTR_RISING);
    Cy_GPIO_SetInterruptMask(P1_4_PORT, P1_4_NUM, CY_GPIO_INTR_EN_MASK);

    /* Configure CM4+ CPU GPIO interrupt vector for Port 0 */
    Cy_SysInt_Init(&intrCfg0_4, GPIO_Interrupt);
    NVIC_ClearPendingIRQ(intrCfg0_4.intrSrc);
    NVIC_EnableIRQ((IRQn_Type)intrCfg0_4.intrSrc);

    /* Configure CM4+ CPU GPIO interrupt vector for Port 0 */
    Cy_SysInt_Init(&intrCfg1_4, GPIO_Interrupt);
    NVIC_ClearPendingIRQ(intrCfg1_4.intrSrc);
    NVIC_EnableIRQ((IRQn_Type)intrCfg1_4.intrSrc);

    TFT_HwReset();
    TFT_SwReset();
    TFT_ConfigureRotation(0);
    TFT_DisplayOn();


    TFT_FillScreen(BLACK);
#if 0
    TFT_DrawLine(0, 0, 239, 319, WHITE);
    TFT_DrawLine(239, 0, 0, 319, RED);
    TFT_DrawLine(120, 0, 120, 319, GREEN);
    TFT_DrawLine(0, 160, 239, 160, BLUE);
    TFT_DrawLine(0, 0, 120 ,0, PINK);
#endif

    printf("Hello Altia!\n\r");

    /* init Altia */
    if(altiaInitDriver() < 0)
    {
    }

    altiaFlushOutput();

    for (;;)
    {
    	// change screen
    	if(screenChange)
    	{
    		if(global_jump_to_screen == 0) //
    		{
    			altiaSendEvent(ALT_ANIM(INTRO_to_main), 1);

    			global_jump_to_screen = 1;
    		}
    		else
    		{
    			AltiaEventType value;
    		    altiaPollEvent(ALT_ANIM(GLOBAL_demo_type), &value);

    		    global_jump_to_screen = (int)value + 1;

    			if(global_jump_to_screen > 4) global_jump_to_screen = 1;
    		}

    		// make sure that an eventually running About Screen is set invisible
    		altiaSendEvent(ALT_ANIM(ABOUT_screen_visible), 0);
			altiaSendEvent(ALT_ANIM(GLOBAL_jump_to_screen), global_jump_to_screen);

			// when a screen change was done ABOUT screen is not shown
			aboutScreenShown = 0;

			screenChange = 0;
    	}

    	// ABOUT screen
    	if(aboutTrigger)
    	{
    		if(!aboutScreenShown)
    		{
    			AltiaEventType value;

    			// check if INTRO is still running
    			altiaPollEvent(ALT_ANIM(INTRO_screen_visible), &value);

    			if(value == 1)
    			{
    				// do nothing
    			}
    			else
    			{
    				// figure out which demo screen is currently shown
    				altiaPollEvent(ALT_ANIM(GLOBAL_demo_type), &value);

    				if(value == 1)
    				{
    					altiaSendEvent(ALT_ANIM(MEDICAL_stimulus_about), 1);
    					aboutScreenShown = 1;
    				}
    				else if(value == 2)
    				{
    					altiaSendEvent(ALT_ANIM(HOMEAPP_stimulus_about), 1);
    					aboutScreenShown = 1;
    				}
    				else if(value == 3)
    				{
    					altiaSendEvent(ALT_ANIM(CONAG_stimulus_about), 1);
    					aboutScreenShown = 1;
    				}
    				else if(value == 4)
    				{
    					altiaSendEvent(ALT_ANIM(AUTO_stimulus_about), 1);
    					aboutScreenShown = 1;
    				}
    			}
    		}
    		else
    		{
    			altiaSendEvent(ALT_ANIM(ABOUT_stimulus_back), 1);
    			aboutScreenShown = 0;
    		}

    		aboutTrigger = 0;
    	}

    	altiaSendEvent(ALT_ANIM(do_timer), 1);

    	altiaFlushOutput();
    }
}

/* [] END OF FILE */
