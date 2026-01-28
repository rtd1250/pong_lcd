#include "waveshare/image.h"
#include "waveshare/LCD_Test.h"
#include "waveshare/LCD_2inch4.h"
#include "waveshare/DEV_Config.h"

void LCD_2in4_test()
{
	printf("LCD_2IN4_test Demo\r\n");
	DEV_Module_Init();

	printf("LCD_2IN4_ Init and Clear...\r\n");
	LCD_2IN4_Init();
	LCD_2IN4_Clear(BLACK);

	printf("Paint_NewImage\r\n");
	Paint_NewImage(LCD_2IN4_WIDTH,LCD_2IN4_HEIGHT, ROTATE_0, BLACK);

	printf("Set Clear and Display Funtion\r\n");
	Paint_SetClearFuntion(LCD_2IN4_Clear);
	Paint_SetDisplayFuntion(LCD_2IN4_DrawPaint);

	//	printf("Paint_Clear\r\n");
	//	Paint_Clear(WHITE);
	//	DEV_Delay_ms(100);
	//
	//	printf("Painting...\r\n");
	Paint_SetRotate(ROTATE_90);
	//	Paint_DrawString_EN (5, 10, "JEBAC",        &Font24,    YELLOW,  RED);
	//	Paint_DrawString_EN (5, 34, "LODZ",  &Font24,    BLUE,    CYAN);
	//	Paint_DrawFloatNum  (5, 150 ,987.654321,5,  &Font20,    WHITE,   LIGHTBLUE);
	//	Paint_DrawString_EN (5,170, "24h",    &Font24,    WHITE,   BLUE);
	//	Paint_DrawString_CN (5,190, "΢ѩ����",     &Font24CN,  WHITE,   RED);
	//
	//	Paint_DrawRectangle (125, 240, 225, 300,    RED     ,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
	//	Paint_DrawLine      (125, 240, 225, 300,    MAGENTA ,DOT_PIXEL_2X2,LINE_STYLE_SOLID);
	//	Paint_DrawLine      (225, 240, 125, 300,    MAGENTA ,DOT_PIXEL_2X2,LINE_STYLE_SOLID);
	//
	//	Paint_DrawCircle(150,100,  25,        BLUE    ,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
	//	Paint_DrawCircle(180,100,  25,        BLACK   ,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
	//	Paint_DrawCircle(210,100,  25,        RED     ,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
	//	Paint_DrawCircle(165,125,  25,        YELLOW  ,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
	//	Paint_DrawCircle(195,125,  25,        GREEN   ,DOT_PIXEL_2X2,DRAW_FILL_EMPTY);

	// Paint_DrawImage(gImage_aa,12,0,216,320);


	DEV_Delay_ms(3000);

	printf("quit...\r\n");
	//DEV_Module_Exit();

}

