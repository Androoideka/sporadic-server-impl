
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#define configGRANULARITY   200

#define commADDPD     ( BaseType_t ) 1
#define commADDAB     ( BaseType_t ) 2 // command add aperiodic batched
#define commADDAN     ( BaseType_t ) 3 // command add aperiodic now
#define commREMOV     ( BaseType_t ) 4
#define commSCHCK     ( BaseType_t ) 5
#define commSINIT     ( BaseType_t ) 6

#define COMMAND_SIZE        17
#define FUNC_NAME_SIZE      17

#define errINVALIDCOMMAND   -100
#define errMISSINGPARAM     -1000
#define errBADINPUT         -1001
#define errNOTFOUND         -1002

static void input_handler();

static void task0(void *pvParams)
{
	printf("+");
	fflush(stdout);
	vTaskDelete(0);
}

static void task1(void *pvParams)
{
	printf("-");
	fflush(stdout);
	vTaskDelete(0);
}

static void vTask1(void *pvParams)
{
	TickType_t startTick = xTaskGetTickCount();
	for(int i = 0; i < 500000; i++) {
		double o = 421634/2341;
	}
	TickType_t endTick = xTaskGetTickCount();
	printf("Task 1 - %d\n", endTick - startTick);
	fflush(stdout);
	vTaskDelete(0);
}

static void vTask2(void *pvParams)
{
	TickType_t startTick = xTaskGetTickCount();
	for(int i = 0; i < 2000000; i++) {
		double o = 421634/2341;
	}
	TickType_t endTick = xTaskGetTickCount();
	printf("Task 2 - %d\n", endTick - startTick);
	fflush(stdout);
	vTaskDelete(0);
}

static void vTask4(void *pvParams)
{
	TickType_t startTick = xTaskGetTickCount();
	for(int i = 0; i < 3100000; i++) {
		double o = 421634/2341;
	}
	TickType_t endTick = xTaskGetTickCount();
	printf("Task 4 - %d\n", endTick - startTick);
	fflush(stdout);
	vTaskDelete(0);
}

static void vInput(void *pvParams)
{
	FILE *readFile = stdin;
	FILE *writeFile = stderr;
	input_handler(readFile, writeFile);
}

static void (*pfFunctionForString(char pcString[]))(void* pvParams)
{
	if(strcmp(pcString, "task0") == 0)
	{
		return task0;
	}
	else if(strcmp(pcString, "task1") == 0)
	{
		return task1;
	}
	else if(strcmp(pcString, "vTask1") == 0)
	{
		return vTask1;
	}
	else if(strcmp(pcString, "vTask2") == 0)
	{
		return vTask2;
	}
	else if(strcmp(pcString, "vTask4") == 0)
	{
		return vTask4;
	}
	else if(strcmp(pcString, "vInput") == 0)
	{
		return vInput;
	}
	else
	{
		return NULL;
	}
}

static TickType_t xGetComputationTime(char pcString[])
{
	if(strcmp(pcString, "task0") == 0)
	{
		return 1;
	}
	else if(strcmp(pcString, "task1") == 0)
	{
		return 1;
	}
	else if(strcmp(pcString, "vTask1") == 0)
	{
		return 1;
	}
	else if(strcmp(pcString, "vTask2") == 0)
	{
		return 2;
	}
	else if(strcmp(pcString, "vTask4") == 0)
	{
		return 4;
	}
	else if(strcmp(pcString, "vInput") == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void exception_handler(BaseType_t xError, FILE *file)
{
	if(xError != pdPASS) {
		if(xError == errSCHEDULER_RUNNING)
			fprintf(file, "Cannot execute this command while the scheduler is active.\n");
		else if(xError == errSCHEDULE_NOT_FEASIBLE)
			fprintf(file, "Schedule was not feasible with given set of tasks. Your batch has been dropped.\n");
		else if(xError == errSERVER_NOT_FEASIBLE)
			fprintf(file, "Schedule was not feasible with given server parameters.\n");
		else if(xError == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
			fprintf(file, "Couldn't allocate required memory.\n");
		else if(xError == errNOTFOUND)
			fprintf(file, "Couldn't find desired item.\n");
		else if(xError == errINVALIDCOMMAND)
			fprintf(file, "Your command wasn't recognized.\n");
		else if(xError == errMISSINGPARAM)
			fprintf(file, "Command is missing additional parameters.\n");
		else if(xError == errBADINPUT)
			fprintf(file, "Parameters couldn't be read properly.\n");
	}
	fflush(file);
}

static void input_handler(FILE *readFile, FILE *writeFile) {
	for(;;)
	{
		BaseType_t xError = pdPASS;
		BaseType_t xCommand = ( BaseType_t ) 0;
		char pcCommand[COMMAND_SIZE];

		fscanf(readFile, "%s", pcCommand);

		if(strcmp(pcCommand, "add_task_periodic") == 0)
			xCommand = commADDPD;
		else if(strcmp(pcCommand, "add_task_later") == 0)
			xCommand = commADDAB;
		else if(strcmp(pcCommand, "add_task") == 0)
			xCommand = commADDAN;
		else if(strcmp(pcCommand, "stop_task") == 0)
			xCommand = commREMOV;
		else if(strcmp(pcCommand, "get_max_server_capacity") == 0)
			xCommand = commSCHCK;
		else if(strcmp(pcCommand, "initialise_server") == 0)
			xCommand = commSINIT;
		else
			xError = errINVALIDCOMMAND;

		if(xCommand == commADDPD || xCommand == commADDAB || xCommand == commADDAN) {
			char pcName[configMAX_TASK_NAME_LEN];
			char pcFuncName[FUNC_NAME_SIZE];
			char pvParams[configMAX_PARAM_LEN];
			TaskHandle_t xHandle;

			if(fscanf(readFile, "%s %s %s", pcName, pcFuncName, pvParams) != 3)
			{
				xError = errMISSINGPARAM;
			}
			else if(strlen(pcName) >= configMAX_TASK_NAME_LEN || strlen(pcFuncName) >= FUNC_NAME_SIZE || strlen(pvParams) >= configMAX_PARAM_LEN)
			{
				xError = errBADINPUT;
			}
			else
			{
				void (*pfFunc)(void* pvParams) = pfFunctionForString(pcFuncName);
				TickType_t xComputationTime = xGetComputationTime(pcFuncName);
				if (pfFunc == NULL)
				{
					xError = errNOTFOUND;
				}
				else
				{
					TickType_t xArrivalTime = (TickType_t) 0U;
					TickType_t xPeriod = (TickType_t) 0U;
					if(xCommand == commADDAN)
					{
						xArrivalTime = xTaskGetTickCount();
					}
					if(xCommand == commADDPD && fscanf(readFile, "%u", &xPeriod) != 1)
					{
						xError = errMISSINGPARAM;
					}
					else if(xCommand == commADDAB && fscanf(readFile, "%u", &xArrivalTime) != 1)
					{
						xError = errMISSINGPARAM;
					}
					else
					{
						if(strcmp(pvParams, "NULL") == 0)
						{
							xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, NULL, &xHandle, xArrivalTime, xPeriod, xComputationTime);
						}
						else
						{
							xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, &xHandle, xArrivalTime, xPeriod, xComputationTime);
						}
						if( xError == pdPASS )
						{
							fprintf(writeFile, "Handle: %p\n", xHandle);
						}
					}
				}
			}
		}
		else if(xCommand == commREMOV)
		{
			TaskHandle_t xHandle;

			if(fscanf(readFile, "%p", &xHandle) != 1)
			{
				xError = errMISSINGPARAM;
			}
			else
			{
				vTaskDelete(xHandle);
			}
		}
		else if(xCommand == commSCHCK)
		{
			TickType_t xCapacity;
			TickType_t xPeriod;

			if(fscanf(readFile, "%u", &xPeriod) != 1)
			{
				xError = errMISSINGPARAM;
			}
			else
			{
				xError = xTaskCalcMaxServer(&xCapacity, xPeriod);
				if(xError == pdPASS)
				{
					fprintf(writeFile, "%u\n", xCapacity);
				}
			}
		}
		else if(xCommand == commSINIT)
		{
			TickType_t xCapacity;
			TickType_t xPeriod;

			if(fscanf(readFile, "%u %u", &xCapacity, &xPeriod) != 2)
			{
				xError = errMISSINGPARAM;
			}
			else
			{
				xError = xTaskSetServer(xCapacity, xPeriod);
				if(xError == pdPASS) {
					fprintf(writeFile, "Handle: %p\n", xTaskGetIdleTaskHandle());
					fflush(writeFile);
					return;
				}
			}
		}
		exception_handler(xError, writeFile);
	}
}

static TickStats_t pxTickStats[configGRANULARITY];

static void vWriteStatsTask(void *pvParams)
{
	FILE *statFile;
	statFile = fopen("log.txt", "w");

	if (statFile == NULL)
	{
		printf("Error while opening file\n");
		fflush(stdout);
		return;
	}

	taskENTER_CRITICAL();
	{
		for( UBaseType_t i = ( UBaseType_t ) 0U; i < configGRANULARITY; i++ )
		{
			fprintf(statFile, "%u %p %u\n", pxTickStats[i].xTick, pxTickStats[i].xHandle, pxTickStats[i].xCapacity);
		}
	}
	taskEXIT_CRITICAL();

	fclose(statFile);
	vTaskDelete(0);
}

int main( void )
{
	FILE *readFile = stdin;
	FILE *writeFile = stderr;

	/* Stat writing */
	BaseType_t xError = pdPASS;
	TaskHandle_t xHandle;
	xError = xTaskCreate(vWriteStatsTask, "stat", configMINIMAL_STACK_SIZE, NULL, &xHandle, 0, configGRANULARITY / 2, 1);
	if( xError == pdPASS )
	{
		fprintf(writeFile, "Handle: %p\n", xHandle);
		fflush(writeFile);
	}

	input_handler(readFile, writeFile);
	vTaskStartScheduler(pxTickStats, configGRANULARITY);

	return 0;

}

