#ifndef FAN_H
#define FAN_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
/*
 * Fan control pin: PD0
 * Connect PD0 -> Base of NPN transistor (e.g. 2N2222 or BC547)
 *               -> Collector to DC Motor (-)
 *               -> Motor (+) to 5V/12V external supply
 *               -> Emitter to GND
 * Add a flyback diode across the motor terminals.
 */
#define FAN_PORT    GPIOD
#define FAN_PIN     GPIO_PIN_0

/* Fan state -----------------------------------------------------------------*/
typedef enum
{
    FAN_OFF = 0,
    FAN_ON  = 1
} FanState;

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief  Initializes the fan pin to OFF state.
 */
void Fan_Init(void);

/**
 * @brief  Turns the fan ON (sets PD0 high).
 */
void Fan_TurnOn(void);

/**
 * @brief  Turns the fan OFF (sets PD0 low).
 */
void Fan_TurnOff(void);

/**
 * @brief  Returns the current fan state.
 * @retval FAN_ON or FAN_OFF
 */
FanState Fan_GetState(void);

/**
 * @brief  Automatically controls the fan based on temperature.
 *         - temp >= TEMP_MAX : fan turns ON
 *         - temp <= TEMP_MIN : fan turns OFF
 *         - Between thresholds: fan keeps its current state (hysteresis)
 * @param  temp  Current temperature in °C
 */
void Fan_AutoControl(float temp);

#endif /* FAN_H */
