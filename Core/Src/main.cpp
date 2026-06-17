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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "menu.h"
#include "st7789.h"
#include "mpu9250.h"
#include <stdio.h>

// BIBLIOTECA DA INTELIGÊNCIA ARTIFICIAL
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
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
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t g_start_sampling = 0;
uint8_t g_run_inference = 0;
volatile uint8_t g_timer_flag = 0;

char uart_buf[128];
MPU9250_t my_mpu;
volatile float debug_ax = 0.0f;
volatile uint8_t g_motor_state = 0;
uint8_t mpu_connected = 1;

// Buffers da IA
float ei_features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE]; // Vetor com o tamanho exato que a IA pede
int ei_feature_ix = 0; // Apontador
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    huart1.Init.BaudRate = 460800;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    // Initialize the original ST7789 driver
    ST7789_Init();

    // ==========================================
	// INÍCIO DO SCANNER I2C
	// ==========================================[
    /*
	ST7789_Fill_Color(BLUE);
	ST7789_WriteString(10, 10, "SCANNER I2C...", Font_11x18, WHITE, BLUE);

	int y_pos = 40;
	uint8_t count = 0;
	char scan_msg[30];

	// Varre todos os endereços de 1 a 127
	for(uint8_t i = 1; i < 128; i++) {
		// A HAL exige o endereço deslocado 1 bit para a esquerda
		if(HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i << 1), 1, 10) == HAL_OK) {
			snprintf(scan_msg, sizeof(scan_msg), "Achou: 0x%02X", i);
			ST7789_WriteString(10, y_pos, scan_msg, Font_11x18, GREEN, BLUE);
			y_pos += 20;
			count++;
		}
	}

	if(count == 0) {
		ST7789_WriteString(10, y_pos, "Barramento MORTO!", Font_11x18, RED, BLUE);
		ST7789_WriteString(10, y_pos + 20, "Verifique fios", Font_7x10, WHITE, BLUE);
	} else {
		snprintf(scan_msg, sizeof(scan_msg), "Total: %d disp.", count);
		ST7789_WriteString(10, y_pos + 10, scan_msg, Font_11x18, YELLOW, BLUE);
	}

	// Trava o processador aqui para lermos a tela
	while(1) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
		HAL_Delay(200);
	}
	*/

    HAL_Delay(250);


    // Inicializa o MPU e pega o status
        uint8_t mpu_status = MPU9250_Init(&hi2c1);

        if (mpu_status == 0) {
            ST7789_Fill_Color(GREEN);
            ST7789_WriteString(10, 50, "MPU9250 OK!", Font_11x18, BLACK, GREEN);
        } else if (mpu_status == 1) {
            ST7789_Fill_Color(RED);
            ST7789_WriteString(10, 50, "FALHA NO I2C (ERR 1)", Font_11x18, WHITE, RED);
            while(1);
        } else if (mpu_status == 3) {
            ST7789_Fill_Color(RED);
            ST7789_WriteString(10, 50, "FALHA RESET (ERR 3)", Font_11x18, WHITE, RED);
            while(1);
        } else {
            // Se não for 0, 1 ou 3, a variável contém o ID real do seu chip!
            char err_msg[20];
            ST7789_Fill_Color(RED);
            snprintf(err_msg, sizeof(err_msg), "ID REAL: 0x%02X", mpu_status);

            ST7789_WriteString(10, 30, "CHIP DIFERENTE!", Font_11x18, WHITE, RED);
            ST7789_WriteString(10, 60, err_msg, Font_11x18, YELLOW, RED);

            // Pisca o LED indicando o erro de ID
            while(1) {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
                HAL_Delay(200);
            }
        }

    // Brief delay to let you read the "MPU9250 OK!" message before drawing the menu
    HAL_Delay(1000);

    // Initialize the blue menu
    Menu_Init();

    HAL_TIM_Base_Start_IT(&htim3);
    /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    uint32_t display_update_counter = 0;

	while (1)
	{
		if (g_timer_flag)
		{
			g_timer_flag = 0;

			// 1. "Pinga" o barramento I2C para ver se o módulo físico está lá (Endereço 0x68 deslocado)
			if (HAL_I2C_IsDeviceReady(&hi2c1, (0x68 << 1), 1, 10) == HAL_OK)
			{
				// Se estava desconectado e acabou de voltar, reconfigura o chip e limpa a tela
				if (mpu_connected == 0) {
					mpu_connected = 1;
					MPU9250_Init(&hi2c1); // Recarrega os registradores do MPU (vital após perda de energia)

					// Restaura a interface visual dependendo da tela atual
					if (g_start_sampling) {
						ST7789_Fill_Color(BLACK);
						ST7789_WriteString(10, 10, "COLETANDO...", Font_11x18, YELLOW, BLACK);
					} else {
						Menu_Init();
					}
				}

				// 2. Faz a leitura e o envio apenas se o MPU estiver conectado
				MPU9250_Read_All(&hi2c1, &my_mpu);

				if (g_start_sampling)
				{
					uint32_t current_time = HAL_GetTick(); // Tempo em ms

					int len = snprintf(uart_buf, sizeof(uart_buf),
									   "%lu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\r\n",
									   current_time,
									   my_mpu.Ax, my_mpu.Ay, my_mpu.Az,
									   my_mpu.Gx, my_mpu.Gy, my_mpu.Gz,
									   my_mpu.Mx, my_mpu.My, my_mpu.Mz,
									   g_motor_state);

					HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 2);

					// Atualiza tela a cada 100ms
					display_update_counter++;
					if (display_update_counter >= 25) {
						display_update_counter = 0;
						char buf[32];

						snprintf(buf, sizeof(buf), "A:%4.1f %4.1f %4.1f", my_mpu.Ax, my_mpu.Ay, my_mpu.Az);
						ST7789_WriteString(10, 30, buf, Font_7x10, WHITE, BLACK);

						snprintf(buf, sizeof(buf), "G:%4.0f %4.0f %4.0f", my_mpu.Gx, my_mpu.Gy, my_mpu.Gz);
						ST7789_WriteString(10, 45, buf, Font_7x10, YELLOW, BLACK);

						snprintf(buf, sizeof(buf), "M:%4.0f %4.0f %4.0f", my_mpu.Mx, my_mpu.My, my_mpu.Mz);
						ST7789_WriteString(10, 60, buf, Font_7x10, CYAN, BLACK);

						if (g_motor_state == 0) {
							ST7789_WriteString(10, 110, "LBL: Parado    ", Font_11x18, GREEN, BLACK);
						} else {
							if (g_motor_state == 1){
								ST7789_WriteString(10, 110, "LBL: Estavel    ", Font_11x18, GREEN, BLACK);

							} else {
								char lbl_buf[24];
								snprintf(lbl_buf, sizeof(lbl_buf), "LBL: INSTAVELN%d", g_motor_state - 1);
								ST7789_WriteString(10, 110, lbl_buf, Font_11x18, RED, BLACK);
							}
						}
					}
				}

				// ==========================================
				// LÓGICA DE INFERÊNCIA DA REDE NEURAL
				// ==========================================
				if (g_run_inference)
				{
					// Enfileira os 6 eixos na ordem exata que você treinou!
					ei_features[ei_feature_ix++] = my_mpu.Ax;
					ei_features[ei_feature_ix++] = my_mpu.Ay;
					ei_features[ei_feature_ix++] = my_mpu.Az;
					ei_features[ei_feature_ix++] = my_mpu.Gx;
					ei_features[ei_feature_ix++] = my_mpu.Gy;
					ei_features[ei_feature_ix++] = my_mpu.Gz;

					// O vetor encheu? (Passaram-se 2 segundos de amostras)
					if (ei_feature_ix >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
					{
						ei_feature_ix = 0; // Reseta o apontador para a próxima janela

						// Pacote de sinal do Edge Impulse
						signal_t signal;
						int err = numpy::signal_from_buffer(ei_features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

						if (err == 0) {
							ei_impulse_result_t result = { 0 };

							// Roda a rede neural
							err = run_classifier(&signal, &result, false);

							if (err == EI_IMPULSE_OK) {
								float max_val = 0.0f;
								int max_idx = 0;
								for(int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
									if (result.classification[i].value > max_val) {
										max_val = result.classification[i].value;
										max_idx = i;
									}
								}

								// Exibe a Conclusão da IA na Tela TFT
								char ia_buf[30];
								ST7789_WriteString(10, 130, "                ", Font_11x18, BLACK, BLACK);
								snprintf(ia_buf, sizeof(ia_buf), "%s (%.2f)", result.classification[max_idx].label, max_val);

								uint16_t cor = WHITE;
								if (max_idx == 0 || max_idx == 5) cor = GREEN; // Estavel ou Parado
								else cor = RED; // Defeitos

								ST7789_WriteString(10, 130, ia_buf, Font_11x18, cor, BLACK);

								// =========================================================
								// EXIBE RESULTADO POR 3s ANTES DE DORMIR
								// =========================================================
								HAL_Delay(3000);

								// =========================================================
								// INÍCIO DO CICLO DE ECONOMIA DE ENERGIA (1 MINUTO)
								// =========================================================

								// 1. Para o Timer de amostragem
								HAL_TIM_Base_Stop_IT(&htim3);

								// 2. Apaga a tela fisicamente
								ST7789_Fill_Color(BLACK);
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // Apaga a luz de fundo

								// 3. Desliga o sensor MPU9250 (Coloca em Sleep Mode)
								uint8_t pwr_mgmt_1_reg = 0x6B;
								uint8_t sleep_cmd = 0x40;
								HAL_I2C_Mem_Write(&hi2c1, (0x68 << 1), pwr_mgmt_1_reg, 1, &sleep_cmd, 1, 100);

								// 4. Coloca a CPU do STM32 para dormir por 60 segundos
								uint32_t start_sleep = HAL_GetTick();
								while((HAL_GetTick() - start_sleep) < 60000) {
									__WFI();
								}

								// =========================================================
								// ACORDANDO O SISTEMA
								// =========================================================

								// 5. Acorda o sensor MPU9250
								uint8_t wake_cmd = 0x00;
								HAL_I2C_Mem_Write(&hi2c1, (0x68 << 1), pwr_mgmt_1_reg, 1, &wake_cmd, 1, 100);
								HAL_Delay(100); // Dá tempo para o MPU estabilizar

								// 6. DÁ UM RESET DE HARDWARE E RELIGA A TELA
								ST7789_WriteCommand(0x11); // SLPOUT (Sleep Out)
								HAL_Delay(120);            // OBRIGATÓRIO: Aguarda 120ms a bomba de carga estabilizar
								ST7789_WriteCommand(0x29); // DISPON (Liga o painel)
								HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); // Religa a luz de fundo

								// 7. Refaz a interface gráfica limpa
								ST7789_Fill_Color(BLACK);
								ST7789_WriteString(10, 10, "MONITOR DE IA", Font_11x18, MAGENTA, BLACK);
								ST7789_WriteString(10, 130, "Lendo Sensores...", Font_7x10, WHITE, BLACK);

								// 8. Religa o Timer para a próxima coleta
								HAL_TIM_Base_Start_IT(&htim3);
							}
						}
					}

					// Atualiza a tela dos sensores em tempo real enquanto o vetor enche (a cada 100ms)
					display_update_counter++;
					if (display_update_counter >= 25) {
						display_update_counter = 0;
						char buf[32];

						snprintf(buf, sizeof(buf), "A:%4.1f %4.1f %4.1f", my_mpu.Ax, my_mpu.Ay, my_mpu.Az);
						ST7789_WriteString(10, 30, buf, Font_7x10, WHITE, BLACK);

						snprintf(buf, sizeof(buf), "G:%4.0f %4.0f %4.0f", my_mpu.Gx, my_mpu.Gy, my_mpu.Gz);
						ST7789_WriteString(10, 45, buf, Font_7x10, YELLOW, BLACK);

						snprintf(buf, sizeof(buf), "M:%4.0f %4.0f %4.0f", my_mpu.Mx, my_mpu.My, my_mpu.Mz);
						ST7789_WriteString(10, 60, buf, Font_7x10, CYAN, BLACK);
					}
				}
			}
			else
			{
				// 3. Módulo MPU caiu ou fio soltou
				if (mpu_connected == 1) {
					mpu_connected = 0;

					// Pinta a tela de erro e avisa o usuário (Trava visual)
					ST7789_Fill_Color(RED);
					ST7789_WriteString(10, 80, "ERRO CRITICO:", Font_11x18, WHITE, RED);
					ST7789_WriteString(10, 110, "MPU9250 DESCONECTADO", Font_7x10, YELLOW, RED);
					ST7789_WriteString(10, 140, "Aguardando conexao...", Font_7x10, WHITE, RED);
				}

				// O loop continua rodando aqui embaixo vazio, sem enviar lixo na UART,
				// apenas esperando o MPU voltar a dar "ACK" no barramento I2C.
			}


		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); // PA1 - Backlight ON

  /*Configure GPIO pins : PE3 PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4 */
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4; // PA7 Removido daqui
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static uint32_t last_interrupt_time = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint32_t current_time = HAL_GetTick();

    // Debounce de 200ms
    if ((current_time - last_interrupt_time) > 200) {
        last_interrupt_time = current_time;

        // Chama a lógica de menu
        Menu_ProcessButton(GPIO_Pin);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM3) {
    // Agora o Timer dispara tanto na coleta quanto na inferência!
    if (g_start_sampling || g_run_inference) {
        g_timer_flag = 1;
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  // Configura o pino PA6 (LED) como saída caso não esteja
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  __disable_irq();
  while (1)
  {
      // Pisca o LED freneticamente para indicar que caiu no Error_Handler
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

      // Delay tosco (já que interrupções estão desativadas)
      for(volatile uint32_t i=0; i<5000000; i++);
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
