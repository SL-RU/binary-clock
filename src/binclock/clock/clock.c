#include "clock.h"

controls_gpio_OutputState clock_o[12] = {
    {1, GPIOA, GPIO_PIN_0, 1}, //0

    {1, GPIOA, GPIO_PIN_1, 1}, //1
    {1, GPIOA, GPIO_PIN_2, 1}, //2
    {1, GPIOA, GPIO_PIN_3, 1}, //3
    {1, GPIOA, GPIO_PIN_4, 1}, //4
    {1, GPIOA, GPIO_PIN_5, 1}, //5
    {1, GPIOA, GPIO_PIN_6, 1}, //6
    
    {1, GPIOB, GPIO_PIN_10, 1},//7
    {1, GPIOB, GPIO_PIN_11, 1},//8
    {1, GPIOB, GPIO_PIN_12, 1},//9
    {1, GPIOB, GPIO_PIN_13, 1},//10

    {1, GPIOB, GPIO_PIN_14, 1},//11
};

controls_gpio_ButtonState clock_b[2] = {
    {1, 1, GPIOB, GPIO_PIN_2, 1}, //-
    {1, 2, GPIOB, GPIO_PIN_1, 1}, //+
};

RTC_TimeTypeDef clock_time;

uint8_t clock_mode = 0;
uint32_t clock_set_min = 0;
uint32_t clock_set_hour = 0;
void clock_handler(
    uint32_t id,
    uint8_t event,
    uint32_t time)
{
    printf("but %d %d\n", id, event);
    if(event & CONTROLS_LONG_CLICK)
    {
	if(id == 2)
	{
	    if(clock_mode == 0)
	    {
		clock_set_min = clock_time.Minutes;
		clock_set_hour = clock_time.Hours;
		clock_mode = 1;
	    }
	    else if(clock_mode == 1)
	    {
		clock_mode = 2;
	    }
	    else if(clock_mode == 2)
	    {
		clock_mode = 0;
		clock_time.Minutes = clock_set_min % 60;
		clock_time.Hours = clock_set_hour % 24;
		clock_time.Seconds = 0;
		HAL_RTC_SetTime(&hrtc, &clock_time, RTC_FORMAT_BIN);
	    }
	}
	else //cancel
	{
	    clock_mode = 0;
	}
    }
    else if(event & CONTROLS_CLICK)
    {
	if(clock_mode == 1)
	{
	    if(id == 2)
		clock_set_hour = (clock_set_hour + 1) % 24;
	    else
	    {
		if(clock_set_hour == 0)
		    clock_set_hour = 23;
		else
		    clock_set_hour = clock_set_hour - 1;
	    }
	    printf("hour %d\n", clock_set_hour);
	}
	else 	if(clock_mode == 2)
	{
	    if(id == 2)
		clock_set_min = (clock_set_min + 1) % 60;
	    else
	    {
		if(clock_set_min == 0)
		    clock_set_min = 59;
		else
		    clock_set_min = clock_set_min - 1;
	    }

	    printf("min %d\n", clock_set_min);
	}

    }
}

void clock_init()
{
    controls_gpio_init(clock_b, 2, clock_o, 12, &clock_handler);
}

void clock_show(uint32_t min, uint32_t hours, uint8_t sec)
{
    uint32_t t = 0b1;
    clock_o[0].val = (sec % 2) ? 0 : 1;
    for (uint8_t i = 0; i < 6; i++) {
	clock_o[6 - i].val = (min & t) ? 0 : 1;
	t = t << 1;
    }
    t = 0b1;
    if(hours >= 12)
    {
	hours -= 12;
	clock_o[11].val = 0;
    }
    else
	clock_o[11].val = 1;
    
    for (uint8_t i = 0; i < 4; i++) {
	clock_o[i + 7].val = (hours & t) ? 0 : 1;
	t = t << 1;
    }

}

void clock_upd()
{
    HAL_RTC_GetTime(&hrtc, &clock_time, RTC_FORMAT_BIN);
    uint32_t min = clock_time.Minutes;
    uint32_t hours = clock_time.Hours;

    if(clock_mode == 0)
    {
	clock_show(min, hours, clock_time.Seconds);
    }
    else     if(clock_mode == 1)
    {
	clock_show(clock_set_min,
		   ((HAL_GetTick() / 3) % 2) ? clock_set_hour : 0,
		   clock_time.Seconds);
    }
    else     if(clock_mode == 2)
    {
	clock_show( ((HAL_GetTick() / 3) % 2) ? clock_set_min : 0,
		    clock_set_hour,
		    clock_time.Seconds);
    }

    //printf("%d:%d:%d\n", clock_time.Hours, clock_time.Minutes, clock_time.Seconds);
    
    controls_gpio_update();
}
