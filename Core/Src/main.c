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
#include "waveshare/GUI_Paint.h"
#include "waveshare/fonts.h"
#include "waveshare/image.h"
#include "waveshare/LCD_Test.h"
#include "waveshare/LCD_2inch4.h"
#include "waveshare/DEV_Config.h"
#include "buzzer.h"
#include "wyniki.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIZE 6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//UART
uint8_t rx = 0;

//ADC
uint32_t value = 0;
uint32_t value2 = 0;

//Timer
volatile uint32_t game_time = 0;
volatile uint8_t tick_flag = 0;

//Punkty
char punkty1[5];
char punkty2[5];
int p1 = 0;
int p2 = 0;

//Bufor
char buf[256] = {'\0'};

//Kontrola prędkości etc.
int divider = 0;
int wcisniety = 0;
int speed = 2;

//Pozycja paletek
volatile int wl = 100;
volatile int wp = 100;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
void Demo(struct rankingList *ranking);
void punkty_up(int gracz);
char* odczytaj(char *word, size_t size);
void wyslij(char* string);
void Buzzer_SetTone(uint32_t freq_hz, uint8_t volume);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

const Tone_t nokia[] = {

		{NOTE_E5, 8}, {NOTE_D5, 8},{ NOTE_FS4, 4},{ NOTE_GS4, 4},
		{NOTE_CS5, 8}, {NOTE_B4, 8}, {NOTE_D4, 4}, {NOTE_E4, 4},
		{NOTE_B4, 8}, {NOTE_A4, 8}, {NOTE_CS4, 4}, {NOTE_E4, 4},
		{NOTE_A4, 2}, {0,0}
};

int __io_putchar(int ch)
{
	if (ch == '\n') {
		uint8_t ch2 = '\r';
		HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return 1;
}

int __io_getchar(void) {
	uint8_t ch = 0;

	HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);

	if(ch == '\r') {
		HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 10);
		return '\n';
	}

	HAL_UART_Transmit(&huart2, &ch, 1, 100);

	return ch;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6)
	{
		game_time++;
		tick_flag = 1;
	}
}

void Buzzer_SetTone(uint32_t freq_hz, uint8_t volume)
{
	if (freq_hz == 0)
	{
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
		return;
	}

	uint32_t timer_clock = 80000000;
	uint32_t prescaler_value = (timer_clock / 1000000) - 1;
	uint32_t period_value = (1000000 / freq_hz) - 1;
	uint32_t pulse_value = (period_value * volume) / 100;

	// Apply settings
	__HAL_TIM_SET_PRESCALER(&htim4, prescaler_value);
	__HAL_TIM_SET_AUTORELOAD(&htim4, period_value);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse_value);

	// Trigger update to apply changes immediately
	HAL_TIM_GenerateEvent(&htim4, TIM_EVENTSOURCE_UPDATE);
}

void nokia(void) {
	const uint32_t WHOLE_NOTE_DURATION = 1700;
	uint32_t noteDuration = 0;

	for (int i = 0; nokia[i].frequency != 0; i++)
	{
		if(nokia[i].duration_ms > 0) {
			noteDuration = WHOLE_NOTE_DURATION / nokia[i].duration_ms;
		} else {
			noteDuration = WHOLE_NOTE_DURATION / abs(nokia[i].duration_ms)*1.5;
		}

		Buzzer_SetTone(nokia[i].frequency, 50);

		HAL_Delay(noteDuration * 0.9);

		Buzzer_SetTone(0, 0);
		HAL_Delay(noteDuration * 0.1);
	}
	Buzzer_SetTone(0, 0);
}

void wyslij(char* string)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)string, strlen(string), 1000);
}

char* odczytaj(char *word, size_t size) {
	uint8_t value = 0;
	int i=0;
	while(i < size - 1) {
		HAL_UART_Receive(&huart2, &value, 1, HAL_MAX_DELAY);
		if (value == 0x7F) {
			if(i>0) {
				word[i] = '\0';
				i--;
				uint8_t erase[] = "\b \b";
				HAL_UART_Transmit(&huart2, erase, 3, 10);
			}
			continue;
		}
		//jak obsłużymy \r to \n zostaje w buforze
		if (value == '\n') {
			continue;
		}
		if(value == '\r') {
			break;
		}
		word[i] = value;

		HAL_UART_Transmit(&huart2, &value, 1, 10);
		char tempStr[2] = {value, '\0'};
		int current_x = 50 + (i * 20);
		Paint_DrawString_EN (current_x, 155, tempStr, &Font24, WHITE, BLACK);

		i++;
		value = 0;
	}
	word[i] = '\0';
	wyslij("\r\n");
	return word;
}

void end(struct rankingList *ranking) {
	char tekst[8] = {'\0'};
	printf("\n");
	Paint_DrawString_EN (50, 70, "KONIEC GRY", &Font24, BLACK, WHITE);
	nokia();
	HAL_Delay(1000);

	int score = 0;
	int winner = (p1 > p2) ? 1 : 2;

	LCD_2IN4_Clear(WHITE);
	Paint_DrawString_EN (10, 30, "KONIEC GRY", &Font24, WHITE, BLACK);
	if (winner == 1) {
		Paint_DrawString_EN (30, 70, "Wygral P1!", &Font24, WHITE, BLACK);
		score = calculatePoints(p1, p2);
	} else {
		Paint_DrawString_EN (30, 70, "Wygral P2!", &Font24, WHITE, BLACK);
		score = calculatePoints(p2, p1);
	}
	Paint_DrawString_EN (30, 130, "Wpisz imie:", &Font24, WHITE, BLACK);
	odczytaj(tekst, sizeof(tekst));
	Paint_DrawString_EN (50, 155, tekst,        &Font24,    BLACK,  WHITE);

	struct player nowy_gracz;
	addPlayerData(&nowy_gracz, tekst, score);
	addPlayerToList(ranking, &nowy_gracz);


	printf("Dodano gracza: %s z wynikiem: %d\n", tekst, score);
	HAL_Delay(2000); // Czas na przeczytanie
}

void punkty_up(int gracz) {
	if(gracz == 1) {
		p1++;
		sprintf(punkty1, "%d", p1);
	}
	if(gracz == 2) {
		p2++;
		sprintf(punkty2, "%d", p2);
	}

	Paint_DrawString_EN (50, 10, punkty1, &Font24, BLACK, WHITE);
	Paint_DrawString_EN (200, 10, punkty2, &Font24, BLACK, WHITE);
}

void pong(struct rankingList *ranking) {
	int x = 120, y = 160;
	int dx = 1, dy = 1;
	int wl_old = 0, wp_old = 0;

	//reset zmiennych
	p1 = p2 = wcisniety = 0;
	game_time = 0;
	sprintf(punkty1, "0");
	sprintf(punkty2, "0");
	speed = 2;

	LCD_2IN4_Clear(BLACK);
	srand(HAL_GetTick());

	Paint_DrawString_EN (50, 10, "0", &Font24, BLACK, WHITE);
	Paint_DrawString_EN (200, 10, "0", &Font24, BLACK, WHITE);
	HAL_TIM_Base_Start_IT(&htim6);

	while(1) {
		//timer
		if(tick_flag)
		{
			tick_flag = 0;
			uint32_t mins = game_time / 60;
			uint32_t secs = game_time % 60;
			printf("Czas gry: %02lu:%02lu \r\n", mins, secs);
		}

		//handling przycisku USRBTN
		if (HAL_GPIO_ReadPin(USRBTN_GPIO_Port, USRBTN_Pin) == GPIO_PIN_RESET
				&& wcisniety == 0) {
			wcisniety = 1;
			speed += 5;
			if(speed == 22) speed = 2;
			sprintf(buf, "Predkosc: %d", speed);
			Paint_DrawString_EN (30, 100, buf,        &Font24,    BLACK,  WHITE);
			HAL_Delay(1000);
			LCD_2IN4_Clear(BLACK);
		}

		if (HAL_GPIO_ReadPin(USRBTN_GPIO_Port, USRBTN_Pin) == GPIO_PIN_SET
				&& wcisniety == 1) {
			wcisniety = 0;
		}

		//paletki
		if(wl_old != wl) {
			LCD_2IN4_SetWindow(wl_old, 20, wl_old+80, 20+8);
			LCD_2IN4_WriteData_WordBuffer(BLACK, 80*8);
		}
		if(wp_old != wp) {
			LCD_2IN4_SetWindow(wp_old, 300, wp_old+80, 300+8);
			LCD_2IN4_WriteData_WordBuffer(BLACK, 80*8);
		}
		wl_old = wl; wp_old = wp;

		LCD_2IN4_SetWindow(wl, 20, wl+80, 20+8);
		LCD_2IN4_WriteData_WordBuffer(WHITE, 80*8);
		LCD_2IN4_SetWindow(wp, 300, wp+80, 300+8);
		LCD_2IN4_WriteData_WordBuffer(WHITE, 80*8);

		//piłeczka
		LCD_2IN4_SetWindow(x, y, x+SIZE, y+SIZE);
		LCD_2IN4_WriteData_WordBuffer(WHITE, SIZE*SIZE);

		LCD_2IN4_SetWindow(x, y, x + SIZE, y + SIZE);
		LCD_2IN4_WriteData_WordBuffer(BLACK, SIZE*SIZE);

		//update pozycji
		x += dx;
		y += dy;

		if(y <= 0) {
			punkty_up(2);
			y = 160;
			x = 120;
		}
		if(y + SIZE >= 320) {
			punkty_up(1);
			y = 160;
			x = 120;
		}
		//warunek wygranej
		if(p1 == 10 || p2 == 10) {
			end(ranking);
			break;
		}

		// Bounce off edges
		if(x <= 0 || x + SIZE >= 240) dx=-dx;

		if(y <= 0 || y + SIZE >= 320) {
			dx = (rand() % 2 == 0) ? 1 : -1;
			dy = (rand() % 2 == 0) ? 1 : -1;
		}

		// Bounce off paletki
		if(y == 28 && x > wl-10 && x < wl+90) {
			dy = -dy;
			Buzzer_SetTone(1000, 50);
			HAL_Delay(20);
			Buzzer_SetTone(0,0);
		}

		if(y == 292 && x > wp-10 && x < wp+90) {
			dy = -dy;
			Buzzer_SetTone(1000, 50);
			HAL_Delay(20);
			Buzzer_SetTone(0,0);
		}

		LCD_2IN4_SetWindow(x, y, x + SIZE, y + SIZE);
		LCD_2IN4_WriteData_WordBuffer(WHITE, SIZE*SIZE);


		HAL_Delay(speed);

		//pomiar co 2 takty gry
		if(divider<10) divider++;
		else divider = 0;
		if(divider%2 == 0) {
			value = HAL_ADC_GetValue(&hadc1);
			value2 = HAL_ADC_GetValue(&hadc2);
			//float voltage = value * 3.3f / 255.0f;
			//printf("\rADC: %d, ADC2: %d", value, value2);
			wl = (value > 160) ? 160 : value;
			wp = (value2 > 160) ? 160 : value2;
		}
	}
}
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

	/* Configure the peripherals common clocks */
	PeriphCommonClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_RTC_Init();
	MX_SPI1_Init();
	MX_USART2_UART_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_ADC1_Init();
	MX_ADC2_Init();
	MX_TIM6_Init();
	/* USER CODE BEGIN 2 */
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start(&hadc1);
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_ADC_Start(&hadc2);

	LCD_2in4_test();
	//	rickroll();

	struct rankingList ranking = {0};

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		LCD_2IN4_Clear(WHITE);
		if (ranking.cPlayers > 0) {
			Paint_DrawString_EN(40, 20, "TOP 5 GRACZY:", &Font20, WHITE, BLACK);

			int limit = (ranking.cPlayers > 5) ? 5 : ranking.cPlayers; // Max 5 lub mniej
			char line_buf[50];
			int y_pos = 60; // Pozycja startowa Y dla listy

			for(int i = 0; i < limit; i++) {
				// Formatowanie tekstu: "1. Nick: Wynik"
				snprintf(line_buf, sizeof(line_buf), "%d.%s: %d",
						i + 1,
						ranking.pList[i].nick,
						ranking.pList[i].score);

				Paint_DrawString_EN(20, y_pos, line_buf, &Font16, WHITE, BLACK);
				y_pos += 25; // Odstęp między wierszami
			}
		}
		Paint_DrawString_EN(20, 200, "Wcisnij przycisk", &Font16, WHITE, BLACK);
		Paint_DrawString_EN(50, 220, "aby zagrac!!!", &Font20, WHITE, BLACK);

		printf("Czekam na przycisk...\n");

		while(HAL_GPIO_ReadPin(USRBTN_GPIO_Port, USRBTN_Pin) != GPIO_PIN_RESET) {
			HAL_Delay(50);
		}

		//debounce
		HAL_Delay(200);

		//Wyświetlenie rankingu na UART
		printf("\r\n--- RANKING GRACZY ---\r\n");
		printList(&ranking);
		printf("----------------------\r\nStart gry za 2 sekundy...\r\n");

		pong(&ranking);
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
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Initializes the peripherals clock
	 */
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
	PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
	PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
	PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
	PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
	PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

	ADC_MultiModeTypeDef multimode = {0};
	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc1.Init.Resolution = ADC_RESOLUTION_8B;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	hadc1.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure the ADC multi-mode
	 */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void)
{

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */

	/** Common config
	 */
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc2.Init.Resolution = ADC_RESOLUTION_8B;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc2.Init.LowPowerAutoWait = DISABLE;
	hadc2.Init.ContinuousConvMode = ENABLE;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.DMAContinuousRequests = DISABLE;
	hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	hadc2.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc2) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */

	/* USER CODE END ADC2_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void)
{

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

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
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
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

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 300-1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 1000-1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);

}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void)
{

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 0;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65535;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */
	HAL_TIM_MspPostInit(&htim4);

}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void)
{

	/* USER CODE BEGIN TIM6_Init 0 */

	/* USER CODE END TIM6_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM6_Init 1 */

	/* USER CODE END TIM6_Init 1 */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 7999;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 9999;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM6_Init 2 */

	/* USER CODE END TIM6_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

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
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, DC_Pin|RST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USRBTN_Pin */
	GPIO_InitStruct.Pin = USRBTN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USRBTN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : DC_Pin RST_Pin */
	GPIO_InitStruct.Pin = DC_Pin|RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : CS_Pin */
	GPIO_InitStruct.Pin = CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);

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
