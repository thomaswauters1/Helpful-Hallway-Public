/*
 * AGM8833.h
 *
 *  Created on: Nov 18, 2019
 *      Author: jonac
 */

/***************************************************************************//**
 * @file AMG8833.h
 * @brief Interface with AMG8833 IR Temp camera
 * @version 1.0
 * @author Jona Cappelle & Thomas Feys
 * ****************************************************************************/

#ifndef _AMG8833_H_
#define _AMG8833_H_

/* Needed to use uintx_t */
#include <stdint.h>										/* Needed to use uintx_t */
#include <stdbool.h>									/* Needed to use booleans */

#define I2C_ADDRESS                     ( 0x69 << 1 )	/* MCU works with 10bit I2C addresses, so we need to shift our address 1 bit */

#define POWER_CONTROL					0x00			/* Power control address */
#define POWER_CONTROL_NORMAL			0x00			/* Power control set to normal mode */
#define POWER_CONTROL_SLEEP				0x10			/* Power control set to sleep mode */
#define POWER_CONTROL_STBY_60			0x20			/* Power control set to standby, measure every 60 sec */
#define POWER_CONTROL_STBY_10			0x21			/* Power control set to standby, measure every 10 sec */

#define AMG8833_POWER_PORT				gpioPortE		/* Power Port */
#define AMG8833_POWER_PIN				13				/* Power Pin */

#define AMG8833_INTERRUPT_PORT			gpioPortD		/* Interrupt Port */
#define AMG8833_INTERRUPT_PIN			4				/* Interrupt Pin */
#define AMG8833_INT_CONTROL_REGISTER	0x03			/* Interrupt control register */
#define AMG8833_INT_LEVEL_REGISTER		0x08			/* Interrupt level register (upper limit) */

#define RESET							0x01			/* Reset register */

#define THERMISTOR_OUTPUT_VALUE_L		0x0E			/* Begin address of thermistor output */

#define PIXEL_1_L						0x80			/* Begin address of IR pixel array (128 registers long) */

#define THERMISTOR_RES					0.0625			/* Constant: thermistor resolution */
#define PIXEL_RES						0.25			/* Constant: pixel resolution */

#define THRESHOLD_VALUE					20				/* Threshold value for interrupt */

/* SWITCH CASES */
#define STBY_60			0x00							/* Register value to set stand-by mode to 60 sec measurements */
#define STBY_10			0x01							/* Register value to set stand-by mode to 10 sec measurements */

void AMG8833_Init( void );
void AMG8833_Thermistor_Read(float *rBuffer_Thermistor);
void AMG8833_Pixels_Read(float *rBuffer_Pixels);
void AMG8833_Sleep(bool enable);
void AMG8833_StandBy(uint8_t time);
void AMG8833_Reset(void);
void AMG8833_Power(bool enable);
void AMG8833_Interrupt(bool enable);

#endif