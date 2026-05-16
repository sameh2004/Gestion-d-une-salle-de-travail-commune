#include "fan.h"
#include "temperature.h"   /* for TEMP_MIN, TEMP_MAX */

/* ---------------------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------------------*/
static FanState current_state = FAN_OFF;

/* ---------------------------------------------------------------------------
 * Fan_Init
 * ---------------------------------------------------------------------------*/
void Fan_Init(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_RESET);
    current_state = FAN_OFF;
}

/* ---------------------------------------------------------------------------
 * Fan_TurnOn
 * ---------------------------------------------------------------------------*/
void Fan_TurnOn(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_SET);
    current_state = FAN_ON;
}

/* ---------------------------------------------------------------------------
 * Fan_TurnOff
 * ---------------------------------------------------------------------------*/
void Fan_TurnOff(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_RESET);
    current_state = FAN_OFF;
}

/* ---------------------------------------------------------------------------
 * Fan_GetState
 * ---------------------------------------------------------------------------*/
FanState Fan_GetState(void)
{
    return current_state;
}

/* ---------------------------------------------------------------------------
 * Fan_AutoControl
 * ---------------------------------------------------------------------------
 * Hysteresis logic:
 *   temp >= TEMP_MAX (25°C)  →  Force ON
 *   temp <= TEMP_MIN (18°C)  →  Force OFF
 *   TEMP_MIN < temp < TEMP_MAX  →  Keep current state (no unnecessary toggling)
 * ---------------------------------------------------------------------------*/
void Fan_AutoControl(float temp)
{
    if (temp >= (float)TEMP_MAX)
    {
        Fan_TurnOn();
    }
    else if (temp <= (float)TEMP_MIN)
    {
        Fan_TurnOff();
    }
    /* else: temperature is within range — fan keeps its last state */
}