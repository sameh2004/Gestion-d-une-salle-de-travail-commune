/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include "temperature.h"
#include "fan.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
#define LDR_THRESHOLD 2000   // à ajuster selon ton capteur

#define LAMP1_PIN GPIO_PIN_4
#define LAMP2_PIN GPIO_PIN_5
#define LAMP_PORT GPIOE
/* USER CODE BEGIN 0 */
#define LCD_RS GPIO_PIN_0
#define LCD_E  GPIO_PIN_1
#define LCD_D4 GPIO_PIN_4
#define LCD_D5 GPIO_PIN_5
#define LCD_D6 GPIO_PIN_6
#define LCD_D7 GPIO_PIN_7
#define LCD_PORT GPIOB

void LCD_Enable(void)
{
  HAL_GPIO_WritePin(LCD_PORT, LCD_E, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(LCD_PORT, LCD_E, GPIO_PIN_RESET);
  HAL_Delay(1);
}

void LCD_Send4Bit(uint8_t data)
{
  HAL_GPIO_WritePin(LCD_PORT, LCD_D4, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_PORT, LCD_D5, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_PORT, LCD_D6, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_PORT, LCD_D7, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  LCD_Enable();
}

void LCD_Command(uint8_t cmd)
{
  HAL_GPIO_WritePin(LCD_PORT, LCD_RS, GPIO_PIN_RESET);

  LCD_Send4Bit(cmd >> 4);
  LCD_Send4Bit(cmd & 0x0F);

  HAL_Delay(2);
}

void LCD_Char(char data)
{
  HAL_GPIO_WritePin(LCD_PORT, LCD_RS, GPIO_PIN_SET);

  LCD_Send4Bit(data >> 4);
  LCD_Send4Bit(data & 0x0F);

  HAL_Delay(2);
}

void LCD_String(char *str)
{
  while(*str)
  {
    LCD_Char(*str++);
  }
}

void LCD_Clear(void)
{
  LCD_Command(0x01);
  HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
  uint8_t address;

  if(row == 0)
    address = 0x80 + col;
  else
    address = 0xC0 + col;

  LCD_Command(address);
}

void LCD_Init(void)
{
  HAL_Delay(50);

  HAL_GPIO_WritePin(LCD_PORT, LCD_RS, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_PORT, LCD_E, GPIO_PIN_RESET);

  LCD_Send4Bit(0x03);
  HAL_Delay(5);
  LCD_Send4Bit(0x03);
  HAL_Delay(5);
  LCD_Send4Bit(0x03);
  HAL_Delay(5);
  LCD_Send4Bit(0x02);

  LCD_Command(0x28);
  LCD_Command(0x0C);
  LCD_Command(0x06);
  LCD_Command(0x01);

  HAL_Delay(5);
}
void LCD_Number(int num)
{
    char buffer[16];

    sprintf(buffer, "%d", num);

    LCD_String(buffer);
}
/* USER CODE END 0 */
uint32_t LDR_Read(ADC_HandleTypeDef *hadc)
{
    uint32_t value = 0;

    HAL_ADC_Start(hadc);
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        value = HAL_ADC_GetValue(hadc);
    }
    HAL_ADC_Stop(hadc);

    return value;
}
uint32_t ADC_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);

    uint32_t value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return value;
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
int nb_personnes = 0;
uint32_t fan_timer_tick = 0;
uint8_t  fan_on_pause   = 0;

HAL_Delay(100);  // Give the LCD 50ms to boot up
LCD_Init();
Fan_Init();
LCD_Command(0x01);
LCD_String("System Ready");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
uint32_t last_temp_tick = 0;
  while (1){
// ===== TEMPERATURE CHECK (every 5 seconds) =====
if (HAL_GetTick() - last_temp_tick >= 5000)
{
    last_temp_tick = HAL_GetTick();
    if (nb_personnes < 8)
    {
				float temp = Temperature_Read(&hadc1);
        // Save state before AutoControl to detect when fan just turned ON
        FanState prev = Fan_GetState();
        Fan_AutoControl(temp);
        if (prev == FAN_OFF && Fan_GetState() == FAN_ON)
        {
            fan_timer_tick = HAL_GetTick();   // start 3s run timer
            fan_on_pause   = 0;
        }

        Temperature_Display(temp);
        HAL_Delay(2000);
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_String("System Ready");
    }
}

// ================= MINUTERIE VENTILATEUR =================
if (Fan_GetState() == FAN_ON && !fan_on_pause)
{
    if (HAL_GetTick() - fan_timer_tick >= 3000)   // 3s = 30min
    {
        Fan_TurnOff();
        fan_on_pause   = 1;
        fan_timer_tick = HAL_GetTick();            // start pause timer
    }
}
if (fan_on_pause)
{
    if (HAL_GetTick() - fan_timer_tick >= 1000)   // 1s = 10min
    {
        fan_on_pause   = 0;
        Fan_TurnOn();
        fan_timer_tick = HAL_GetTick();            // restart run timer
    }
}
		
		
		
		
		
		
		
		
		
		// PC0 -> GREEN LED PE2
		  // PC2 -> Bouton Alerte
  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET)
  {

      // Tant que le bouton reste appuye
      while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET)
      {
          // Buzzer ON
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);

          LCD_Clear();
          LCD_SetCursor(0,0);
          LCD_String("ALERTE ACTIVE");

          HAL_Delay(300);

          // Buzzer OFF
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);

          HAL_Delay(300);
      }

      

      HAL_Delay(200);
  }


  // ================= ENTREE =================
  // PC0 -> GREEN LED PE2
  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_SET)
  {
      if(nb_personnes < 8)
      {
          nb_personnes++;

          // GREEN LED ON
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);

          LCD_Clear();
          LCD_SetCursor(0, 0);
          LCD_String("Personnes:");
          LCD_SetCursor(1, 0);
          LCD_Number(nb_personnes);

          HAL_Delay(3000);

          // GREEN LED OFF
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
      }

      while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_SET);
      HAL_Delay(200);
  }


  // ================= SORTIE =================
  // PC1 -> RED LED PE3
  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET)
  {
      if(nb_personnes > 0)
      {
          nb_personnes--;
      }

      // RED LED ON
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);

      LCD_Clear();
      LCD_SetCursor(0, 0);
      LCD_String("Personnes:");
      LCD_SetCursor(1, 0);
      LCD_Number(nb_personnes);

      HAL_Delay(3000);

      // RED LED OFF
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

      while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET);
      HAL_Delay(200);
  }
// ================= LUMIERE (LDR CONTROL) =================
// ================= LUMIERE (LDR CONTROL) =================
uint32_t ldr_value = ADC_ReadChannel(ADC_CHANNEL_1); // PA1

if (ldr_value < 2000)
{
    // 2 lampes ON
    HAL_GPIO_WritePin(LAMP_PORT, LAMP1_PIN | LAMP2_PIN, GPIO_PIN_SET);

    LCD_SetCursor(0,0);
    LCD_String("Lumiere FAIBLE ");
    LCD_SetCursor(1,0);
    LCD_String("2 Lampes ON    ");
}
else if (ldr_value >= 2000 && ldr_value < 3000)
{
    // 1 lampe ON
    HAL_GPIO_WritePin(LAMP_PORT, LAMP1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LAMP_PORT, LAMP2_PIN, GPIO_PIN_RESET);

    LCD_SetCursor(0,0);
    LCD_String("Lumiere MOYENNE");
    LCD_SetCursor(1,0);
    LCD_String("1 Lampe ON     ");
}
else
{
    // 2 lampes OFF
    HAL_GPIO_WritePin(LAMP_PORT, LAMP1_PIN | LAMP2_PIN, GPIO_PIN_RESET);

    LCD_SetCursor(0,0);
    LCD_String("Lumiere FORTE  ");
    LCD_SetCursor(1,0);
    LCD_String("Lampes OFF     ");
}

  // ================= SALLE COMPLETE =================
  if(nb_personnes >= 8)
  {
      LCD_Clear();
      LCD_SetCursor(0,0);
      LCD_String("Salle Complete");

      HAL_Delay(1000);
  }
}}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE5
                           PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB4
                           PB5 PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD0 PD1 PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */