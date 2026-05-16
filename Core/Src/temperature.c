#include "temperature.h"
#include "main.h"
#include <stdio.h>

/* ---------------------------------------------------------------------------
 * LCD helper functions — declared extern so we can call them from main.c
 * without re-defining them here.
 * ---------------------------------------------------------------------------*/
extern void LCD_Clear(void);
extern void LCD_SetCursor(uint8_t row, uint8_t col);
extern void LCD_String(char *str);

/* ---------------------------------------------------------------------------
 * Temperature_Read
 * ---------------------------------------------------------------------------
 * LM35 characteristics:
 *   - Output voltage : 10 mV / °C
 *   - Supply voltage : 3.3 V  (same as STM32 ADC reference)
 *
 * ADC conversion:
 *   Voltage (mV) = (adc_value / 4096) * 3300
 *   Temperature  = Voltage (mV) / 10
 *
 *   => Temperature (°C) = (adc_value * 3300.0) / (4096 * 10)
 * ---------------------------------------------------------------------------*/
float Temperature_Read(ADC_HandleTypeDef *hadc)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;

    HAL_ADC_ConfigChannel(hadc, &sConfig);

    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);

    uint32_t adc_val = HAL_ADC_GetValue(hadc);

    HAL_ADC_Stop(hadc);

    float voltage_mV = (adc_val * 3300.0f) / 4096.0f;
    return voltage_mV / 10.0f;
}

/* ---------------------------------------------------------------------------
 * Temperature_Display
 * ---------------------------------------------------------------------------*/
void Temperature_Display(float temp)
{
    char buffer[16];

    /* --- Build temperature string with 1 decimal place --- */
    int t_int  = (int)temp;
    int t_frac = (int)((temp - t_int) * 10);
    sprintf(buffer, "Temp: %d.%d C", t_int, t_frac);

    /* --- Line 1: temperature value --- */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String(buffer);

    /* --- Line 2: status + LED control --- */
    LCD_SetCursor(1, 0);

    if (temp < TEMP_MIN)
    {
        LCD_String("Trop Froid!  ");
        HAL_GPIO_WritePin(LED_COLD_PORT, LED_COLD_PIN, GPIO_PIN_SET);   /* PE4 ON  */
        HAL_GPIO_WritePin(LED_HOT_PORT,  LED_HOT_PIN,  GPIO_PIN_RESET); /* PE5 OFF */
    }
    else if (temp > TEMP_MAX)
    {
        LCD_String("Trop Chaud!  ");
        HAL_GPIO_WritePin(LED_HOT_PORT,  LED_HOT_PIN,  GPIO_PIN_SET);   /* PE5 ON  */
        HAL_GPIO_WritePin(LED_COLD_PORT, LED_COLD_PIN, GPIO_PIN_RESET); /* PE4 OFF */
    }
    else
    {
        LCD_String("Temp OK      ");
        HAL_GPIO_WritePin(LED_COLD_PORT, LED_COLD_PIN, GPIO_PIN_RESET); /* PE4 OFF */
        HAL_GPIO_WritePin(LED_HOT_PORT,  LED_HOT_PIN,  GPIO_PIN_RESET); /* PE5 OFF */
    }
}