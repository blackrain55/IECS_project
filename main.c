//main.c
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "fdc2214.h"


void Sys_Init(void);

float res0,res1,res2,res3;
float temp0,temp1,temp2,temp3;

int main(void)
{
  Sys_Init();
    while(1)
    {
        res0 = Cap_Calculate(0);//Collect data (original data)
      res1 = Cap_Calculate(1);
      res2 = Cap_Calculate(2);
      res3 = Cap_Calculate(3);

        res0 = res0-temp0;//Capacitive interface empty load reduces the initial value
        res1 = res1-temp1;
        res2 = res2-temp2;
        res3 = res3-temp3;

        OLED_P6x8Str(0,1,"CH0:",0);Oled_Printf_Float(22,1,res0,0);
        OLED_P6x8Str(0,3,"CH1:",0);Oled_Printf_Float(22,3,res1,0);
        OLED_P6x8Str(0,5,"CH2:",0);Oled_Printf_Float(22,5,res2,0);
        OLED_P6x8Str(0,7,"CH3:",0);Oled_Printf_Float(22,7,res3,0);

        printf("CH0:%3.3f CH1:%3.3f CH2:%3.3f CH3:%3.3f\r\n",res0,res1,res2,res3);

        led=~led;

        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
    }
}

void Sys_Init(void)
{
    delay_init();
    NVIC_Configuration();
    uart_init(115200);     //Serial port rate 115200
    LED_Init();
    OLED_Init();
    printf("OLED OK\r\n");
    OLED_P6x8Str(82,3,"Float",0);
    OLED_P6x8Str(67,5,"Technology",0);
    while(FDC2214_Init());
    led=0;
    delay_ms(100);
    temp0 = Cap_Calculate(0);//Read the initial value
    temp1 = Cap_Calculate(1);
    temp2 = Cap_Calculate(2);
    temp3 = Cap_Calculate(3);
}