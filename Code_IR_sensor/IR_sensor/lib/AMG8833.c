/*
 * AGM8833.c
 *
 *  Created on: Nov 18, 2019
 *      Author: jonac
 */

/***************************************************************************//**
 * @file AMG8833.c
 * @brief Interface with AMG8833 IR Temp camera
 * @version 1.0
 * @author Jona Cappelle & Thomas Feys
 * ****************************************************************************/


#include "AMG8833.h"
#include "I2C.h"

/* dbprint include */
#include "debug_dbprint.h"
#include "em_usart.h"
#include "em_gpio.h"

/* Needed to use uintx_t */
#include <stdint.h>
#include <stdbool.h>


/**************************************************************************//**
 * @brief
 *   Setup AMG8833 Temperature sensor
 *****************************************************************************/
void AMG8833_Init( void )
{
	/* Set the pin high to supply power to the AMG8833 */
	GPIO_PinModeSet(AMG8833_POWER_PORT, AMG8833_POWER_PIN, gpioModePushPull, 1);
}

/**************************************************************************//**
 * @brief
 *   Read the thermistor value of the AMG8833 IR sensor
 *   The data is processed as described in the datasheet
 *
 * @param[in] rBuffer_Thermistor
 *   Address where the read thermistor value is stored
 *
 *****************************************************************************/
void AMG8833_Thermistor_Read(float *rBuffer_Thermistor)
{
	uint8_t rBuffer[2];
	uint8_t wBuffer[2];
	wBuffer[0] = THERMISTOR_OUTPUT_VALUE_L;
	wBuffer[1] = 0x00;

	IIC_WriteReadBuffer(I2C_ADDRESS, wBuffer, 1, rBuffer, 2);

	*rBuffer_Thermistor = ( ( rBuffer[1] << 8 ) | rBuffer[0] ) * THERMISTOR_RES;
}


/**************************************************************************//**
 * @brief
 *   Read the value of all 64 IR pixels
 *   The data is processed as described in the datasheet
 *
 * @param[in] rBuffer_Pixels
 *   Begin address where the read values of the pixel array is stored
 *
 *****************************************************************************/
void AMG8833_Pixels_Read(float *rBuffer_Pixels)
{
	uint8_t rBuffer[128];
	uint8_t wBuffer[2];
	wBuffer[0] = PIXEL_1_L;
	wBuffer[1] = 0x00;

	IIC_WriteReadBuffer(I2C_ADDRESS, wBuffer, 1, rBuffer, 128);

	for (int i=0; i<64; i++)
	{
		rBuffer_Pixels[i] = ( ( rBuffer[2*i+1] << 8 ) | rBuffer[2*i] ) * PIXEL_RES;

	}
}


/**************************************************************************//**
 * @brief
 *   Print all the values of the IR pixel array
 *
 * @param[in] rBuffer_Pixels
 *   Address where the read thermistor value is stored
 *
 *****************************************************************************/
void AMG8833_Pixel_Print(float *rBuffer_Pixels)
{
#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */

	for (int j=0; j<8; j++)
	{
		for (int i=0; i<8; i++)
		{
			dbprintInt( (int) rBuffer_Pixels[j*8+i] );
			dbprint("	");
		}
		dbprintln("");
	}
	dbprintln("---------------------------------------------------");

#endif /* DEBUG_DBPRINT */
}


/**************************************************************************//**
 * @brief
 *   Function to set the AMG8833 to sleep
 *
 *@details
 *	Power consumption sleep: 0.2 mA
 *
 * @note
 * 	In sleep mode, nothing can be read from the registers
 *
 * @param[in] enable
 *   @li 'true' - sleep mode
 *   @li 'false' - normal mode
 *
 *****************************************************************************/
void AMG8833_Sleep(bool enable)
{
	uint8_t rBuffer[1];
	uint8_t wBuffer[2];
	wBuffer[0] = POWER_CONTROL;

	if(enable)
	{
		wBuffer[1] = POWER_CONTROL_SLEEP;
	}else{
		wBuffer[1] = POWER_CONTROL_NORMAL;
	}
	IIC_WriteBuffer(I2C_ADDRESS, wBuffer, 2);

}


/**************************************************************************//**
 * @brief
 *   Set AGM8833 in stand by
 *
 * @details
 * 	Power consumption stand-by: 0.8 mA
 *
 * @note
 *   Can't go from SLEEP to STAND-BY!
 *
 * @param[in] time
 *   @li `STBY_10` - Sets interval to 10sec
 *   @li `STBY_60` - Sets interval to 60sec
 *****************************************************************************/
void AMG8833_StandBy(uint8_t time)
{
	uint8_t wBuffer[2];
	wBuffer[0] = POWER_CONTROL;

	switch(time)
	{
	case STBY_10:
		wBuffer[1] = POWER_CONTROL_STBY_10;
		break;
	case STBY_60:
		wBuffer[1] = POWER_CONTROL_STBY_60;
		break;
	}

	IIC_WriteBuffer(I2C_ADDRESS, wBuffer, 2);
}

/**************************************************************************//**
 * @brief
 *   Resets the AMG8833 sensor to default settings
 *
 *****************************************************************************/
void AMG8833_Reset(void)
{
	//TODO
}

/**************************************************************************//**
 * @brief
 *   Supply Power to the AMG8833 VDD pin
 *
 * @note
 *   Needs to be enabled for the sensor to work
 *
 * @param[in] enable
 *   @li 'true' - Enable power
 *   @li `false' - Disable power
 *****************************************************************************/
void AMG8833_Power(bool enable)
{
	if(enable)
	{
		GPIO_PinOutSet(AMG8833_POWER_PORT, AMG8833_POWER_PIN); /* Enable VDD pin */
	}else{
		GPIO_PinOutClear(AMG8833_POWER_PORT, AMG8833_POWER_PIN); /* Disable VDD pin */
	}
}

/**************************************************************************//**
 * @brief
 *   Sensor Interrupt functionality
 *
 * @details
 * 	@li 'bit0' - INT output active
 * 	@li 'bit1' - Absolute value interrupt mode
 *
 * @param[in] enable
 *   @li 'true' - Enable interrupts
 *   @li `false' - Disable interrupts - other mode
 *****************************************************************************/
void AMG8833_Interrupt(bool enable)
{
	uint8_t rBuffer[1];
	uint8_t wBuffer[2];
	wBuffer[0] = AMG8833_INT_CONTROL_REGISTER;

	if(enable)
	{
		wBuffer[1] = 0x03;
	}else
	{
		wBuffer[1] = 0x00;
	}

	IIC_WriteReadBuffer(I2C_ADDRESS, wBuffer, 1, rBuffer, 0);
}

/**************************************************************************//**
 * @brief
 *   Sensor Interrupt upper limit set: TODO
 *
 * @param[in] value
 *	Upper limit for interrupt
 *****************************************************************************/
void AMG8833_Set_Interrupt_Upper_Value( int value )
{
	uint8_t rBuffer[1];
	uint8_t wBuffer[2];
	wBuffer[0] = AMG8833_INT_LEVEL_REGISTER;

	uint16_t temp;

	temp = (uint16_t) (value / PIXEL_RES);

	wBuffer[1] = temp;
	//TODO

	IIC_WriteReadBuffer(I2C_ADDRESS, wBuffer, 1, rBuffer, 0);
}