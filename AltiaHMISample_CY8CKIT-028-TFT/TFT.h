/*
 * TFT.h
 *
 *  Created on: 07.06.2022
 *      Author: renes
 */

#ifndef TFT_H_
#define TFT_H_

#define BLACK 0x0000
#define RED   0xF800
#define GREEN 0x07E0
#define BLUE  0x001F
#define WHITE 0xFFFF
#define PINK  0xF81F

void TFT_HwReset(void);
void TFT_SwReset(void);
void TFT_ReadDisplayID(uint8 *id);
void TFT_InvertMode(bool mode);
int  TFT_SetWindow(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);
void TFT_ReadDisplayStatus(uint8 *status);
void TFT_DisplayOn(void);
void TFT_DisplayOff(void);
void TFT_FillScreen(uint16_t color);
void TFT_ConfigureRotation(uint16_t rotation);
void TFT_SetPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void TFT_UpdateArea(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, int16_t *buffer);

#endif /* TFT_H_ */
