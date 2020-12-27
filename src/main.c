
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

void task0(void *pvParams)
{
	int i;
	for (i=0 ; i < 10; i++)
	{
		printf("+");
		fflush(stdout);
		vTaskDelay(1); //zakomentarisati ovu liniju radi testiranja
	}

	vTaskDelete(0);
}

void task1(void *pvParams)
{
	int i;
	for (i=0 ; i < 10; i++)
	{
		printf("-");
		fflush(stdout);
		vTaskDelay(2);
	}

	vTaskDelete(0);
}

int main( void )
{
	xTaskCreate(task1, "1", configMINIMAL_STACK_SIZE, NULL, 0, 150, 1);
	xTaskCreate(task0, "0", configMINIMAL_STACK_SIZE, NULL, 0, 250, 2);

	vTaskStartScheduler();

	return 0;

}

