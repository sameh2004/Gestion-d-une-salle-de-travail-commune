#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
#define TEMP_MIN        18      /* Minimum threshold in °C */
#define TEMP_MAX        25      /* Maximum threshold in °C */

#define LED_COLD_PORT   GPIOE
#define LED_COLD_PIN    GPIO_PIN_2   /* PE4 -> Cold indicator LED */

#define LED_HOT_PORT    GPIOE
#define LED_HOT_PIN     GPIO_PIN_3   /* PE5 -> Hot indicator LED */

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief  Reads the LM35 sensor via ADC and returns temperature in Celsius.
 * @param  hadc  Pointer to the ADC handle (ADC1 configured on PA0 / Channel 0)
 * @retval Temperature value in °C (float)
 */
float Temperature_Read(ADC_HandleTypeDef *hadc);

/**
 * @brief  Displays the temperature on the LCD and manages threshold LEDs.
 *         - Below TEMP_MIN : shows "Trop Froid!" and turns on PE4
 *         - Above TEMP_MAX : shows "Trop Chaud!" and turns on PE5
 *         - Within range   : shows "Temp OK"     and turns off both LEDs
 * @param  temp  Temperature value returned by Temperature_Read()
 */
void Temperature_Display(float temp);

#endif /* TEMPERATURE_H */