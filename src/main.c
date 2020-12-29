
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#define commADDPD     ( BaseType_t ) 1
#define commADDAPD    ( BaseType_t ) 2
#define commREMOV     ( BaseType_t ) 3
#define commSINIT     ( BaseType_t ) 4

#define COMMAND_SIZE        17
#define FUNC_NAME_SIZE      17

#define errINVALIDCOMMAND   -1
#define errMISSINGPARAM     -1000
#define errBADINPUT         -1001
#define errNOTFOUND         -1002

/*void task0(void *pvParams)
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
}*/

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

/*void task0(void *pvParams)
{
	int i;
	for (i=0 ; i < 10; i++)
	{
		printf("+");
		fflush(stdout);
		vTaskDelete(0); //zakomentarisati ovu liniju radi testiranja
	}
}

void task1(void *pvParams)
{
	int i;
	for (i=0 ; i < 10; i++)
	{
		printf("-");
		fflush(stdout);
		vTaskDelete(0);
	}
}*/

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

static void exception_handler(BaseType_t xError)
{
	if(xError == pdPASS)
		return;
	if(xError == errSCHEDULE_NOT_FEASIBLE)
		printf("Schedule was not feasible with new additions.\n");
	if(xError == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
		printf("Couldn't allocate required memory.\n");
	if(xError == errINVALIDCOMMAND)
		printf("Your command wasn't recognized.\n");
	if(xError == errMISSINGPARAM)
		printf("Command is missing additional parameters.\n");
	if(xError == errBADINPUT)
		printf("Parameters couldn't be read properly.\n");
	if(xError == errNOTFOUND)
		printf("Couldn't find desired item.\n");
	fflush(stdout);
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

void input_handler(FILE *file) {
	for(;;)
	{
		BaseType_t xError = pdPASS;
		BaseType_t xCommand = ( BaseType_t ) 0;
		char pcCommand[COMMAND_SIZE];

		printf("System is listening to commands.\n");
		fflush(stdout);
		fscanf(file, "%s", pcCommand);

		if(strcmp(pcCommand, "add_task_periodic") == 0)
			xCommand = commADDPD;
		else if(strcmp(pcCommand, "add_task") == 0)
			xCommand = commADDAPD;
		else if(strcmp(pcCommand, "remove_task") == 0)
			xCommand = commREMOV;
		else if(strcmp(pcCommand, "initialize_server") == 0)
			xCommand = commSINIT;
		else
			xError = errINVALIDCOMMAND;

		if(xCommand == commADDPD)
		{
			char pcName[configMAX_TASK_NAME_LEN];
			char pcFuncName[FUNC_NAME_SIZE];
			char pvParams[configMAX_PARAM_LEN];
			int xPeriod;

			if(fscanf(file, "%s %s %s %d", pcName, pcFuncName, pvParams, &xPeriod) != 4)
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
				if (pfFunc == NULL)
				{
					xError = errNOTFOUND;
				}
				else
				{
					xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, 0, 0, 150, 1);
				}
			}
		}
		else if(xCommand == commADDAPD)
		{
			char pcName[configMAX_TASK_NAME_LEN];
			char pcFuncName[FUNC_NAME_SIZE];
			char pvParams[configMAX_PARAM_LEN];
			int xStartTime;

			if(fscanf(file, "%s %s %s %d", pcName, pcFuncName, pvParams, &xStartTime) != 4)
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
				if (pfFunc == NULL)
				{
					xError = errNOTFOUND;
				}
				else
				{
					xError = xTaskCreate(pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, 0, 0, 0, 1);
				}
			}
		}
		else if(xCommand == commREMOV)
		{
			char pcName[configMAX_TASK_NAME_LEN];

			if(fscanf(file, "%s", pcName) != 1)
			{
				xError = errMISSINGPARAM;
			}
			else
			{
				// We'll call a remove task method from task.h here
			}
		}
		else if(xCommand == commSINIT)
		{
			// We'll initialize a server here (work in progress
		}
		exception_handler(xError);
	}
}

int main( void )
{
	xTaskCreate(task1, "1", configMINIMAL_STACK_SIZE, NULL, 0, 0, 2, 1);
	xTaskCreate(task0, "0", configMINIMAL_STACK_SIZE, NULL, 0, 0, 4, 1);

	vTaskStartScheduler();

	return 0;

}

