
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#define configGRANULARITY   1000

#define commADDPD     ( BaseType_t ) 1
#define commADDAB     ( BaseType_t ) 2 // command add aperiodic batched
#define commADDAN     ( BaseType_t ) 3 // command add aperiodic now
#define commREMOV     ( BaseType_t ) 4
#define commSCHCK     ( BaseType_t ) 5
#define commSINIT     ( BaseType_t ) 6

#define COMMAND_SIZE        17
#define FUNC_NAME_SIZE      17

#define errINVALIDCOMMAND   -1
#define errMISSINGPARAM     -1000
#define errBADINPUT         -1001
#define errNOTFOUND         -1002

void task0(void *pvParams)
{
	printf("+");
	fflush(stdout);
	vTaskDelete(0);
}

void task1(void *pvParams)
{
	printf("-");
	fflush(stdout);
	vTaskDelete(0);
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
	else
	{
		return 0;
	}
}

/*void task1(void *pvParams)
{
	int i;
	TickType_t xStart = xTaskGetTickCount();
	for (i=0 ; i < 500; i++)
	{
		printf("-");
		fflush(stdout);
	}
	printf("\n%u", xStart - xTaskGetTickCount());
	fflush(stdout);
	vTaskDelete(0);
}

static void (*pfFunctionForString(char pcString[]))(void* pvParams)
{
	if(strcmp(pcString, "task1") == 0)
	{
		return task1;
	}
	else if(strcmp(pcString, "task4") == 0)
	{
		return task4;
	}
	else if(strcmp(pcString, "task2") == 0)
	{
		return task2;
	}
	else
	{
		return NULL;
	}
}

static TickType_t get_computation_time(char pcString[])
{
	if(strcmp(pcString, "task1") == 0)
	{
		return ( TickType_t ) 1U;
	}
	else if(strcmp(pcString, "task4") == 0)
	{
		return ( TickType_t ) 4U;
	}
	else if(strcmp(pcString, "task2") == 0)
	{
		return ( TickType_t ) 2U;
	}
	else
	{
		return NULL;
	}
}*/

static void exception_handler(BaseType_t xError, FILE *file)
{
	if(xError == pdPASS)
		return;
	if(xError == errSCHEDULER_RUNNING)
		fprintf(file, "Cannot add periodic tasks while scheduler is running.");
	if(xError == errSCHEDULE_NOT_FEASIBLE)
		fprintf(file, "Schedule was not feasible with given set of tasks and server parameters.\n");
	if(xError == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
		fprintf(file, "Couldn't allocate required memory.\n");
	if(xError == errNOT_CALLABLE)
		fprintf(file, "This function should only be called after the scheduler is started. SetServer will handle batch distribution before that point.");
	if(xError == errINVALIDCOMMAND)
		fprintf(file, "Your command wasn't recognized.\n");
	if(xError == errMISSINGPARAM)
		fprintf(file, "Command is missing additional parameters.\n");
	if(xError == errBADINPUT)
		fprintf(file, "Parameters couldn't be read properly.\n");
	if(xError == errNOTFOUND)
		fprintf(file, "Couldn't find desired item.\n");
	fflush(file);
}

/*static int filter_number(char string[])
{
	int result;
	char* end;

	result = strtol(string, &end, 0);
	if(result == 0 || strlen(end) > 0)
	{
		return errBADINPUT;
	}
	return result;
} For later, in case numbers need to be read as parameters */

void input_handler(FILE *readFile, FILE *writeFile) {
	for(;;)
	{
		BaseType_t xError = pdPASS;
		BaseType_t xCommand = ( BaseType_t ) 0;
		char pcCommand[COMMAND_SIZE];

		printf("System is listening to commands.\n");
		fflush(stdout);
		fscanf(readFile, "%s", pcCommand);

		if(strcmp(pcCommand, "add_task_periodic") == 0)
			xCommand = commADDPD;
		else if(strcmp(pcCommand, "add_task_later") == 0)
			xCommand = commADDAB;
		else if(strcmp(pcCommand, "add_task") == 0)
			xCommand = commADDAN;
		else if(strcmp(pcCommand, "remove_task") == 0)
			xCommand = commREMOV;
		else if(strcmp(pcCommand, "get_max_server_capacity") == 0)
			xCommand = commSCHCK;
		else if(strcmp(pcCommand, "initialize_server") == 0)
			xCommand = commSINIT;
		else
			xError = errINVALIDCOMMAND;

		if(xCommand == commADDPD)
		{
			char pcName[configMAX_TASK_NAME_LEN];
			char pcFuncName[FUNC_NAME_SIZE];
			char pvParams[configMAX_PARAM_LEN];
			TickType_t xPeriod;
			TaskHandle_t xHandle;

			if(fscanf(readFile, "%s %s %s %u", pcName, pcFuncName, pvParams, &xPeriod) != 4)
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
					xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, &xHandle, 0, xPeriod, xComputationTime);
					if( xError == pdPASS )
					{
						fprintf(writeFile, "%lu\n", ( UBaseType_t ) xHandle);
					}
				}
			}
		}
		else if(xCommand == commADDAB)
		{
			char pcName[configMAX_TASK_NAME_LEN];
			char pcFuncName[FUNC_NAME_SIZE];
			char pvParams[configMAX_PARAM_LEN];
			TickType_t xArrivalTime;
			TaskHandle_t xHandle;

			if(fscanf(readFile, "%s %s %s %u", pcName, pcFuncName, pvParams, &xArrivalTime) != 4)
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
					xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, &xHandle, xArrivalTime, 0, xComputationTime);
					if( xError == pdPASS )
					{
						fprintf(writeFile, "%lu\n", ( UBaseType_t ) xHandle);
					}
				}
			}
		}
		else if(xCommand == commADDAN)
		{
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
					xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, &xHandle, xTaskGetTickCount(), 0, xComputationTime);
					if( xError == pdPASS )
					{
						fprintf(writeFile, "%lu\n", ( UBaseType_t ) xHandle);
					}
				}
			}
		}
		else if(xCommand == commREMOV)
		{
			TaskHandle_t xHandle;

			if(fscanf(readFile, "%lu", &xHandle) != 1)
			{
				xError = errMISSINGPARAM;
			}
			else
			{
				// We'll call a remove task method from task.h here
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
					return;
				}
			}
		}
		exception_handler(xError, writeFile);
	}
}

TaskHandle_t pxTaskOverTime[configGRANULARITY];
TickType_t pxCapacityOverTime[configGRANULARITY];

int main( void )
{
	xTaskCreate(task1, "1", configMINIMAL_STACK_SIZE, NULL, 0, 0, 2, 1);
	//xTaskCreate(task0, "0", configMINIMAL_STACK_SIZE, NULL, 0, 0, 4, 1);

	// Need tasks with computation times: 1, 4, 2

	//xTaskCreate(task1, "1", configMINIMAL_STACK_SIZE, NULL, 0, 0, 0, 1);

	//input_handler(stdin, stderror);
	//vTaskStartScheduler();

	BaseType_t xError = xTaskSetServer(5, 10);
	if(xError == pdPASS) {
		vTaskStartScheduler( pxTaskOverTime, pxCapacityOverTime, configGRANULARITY );
	}

	return 0;

}

