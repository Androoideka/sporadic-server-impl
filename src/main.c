
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Custom includes. */
#include "function.h"

#define commCWCET     ( BaseType_t ) 0
#define commADDPD     ( BaseType_t ) 1
#define commADDAP     ( BaseType_t ) 2
#define commSHWTC     ( BaseType_t ) 3
#define commREMOV     ( BaseType_t ) 4
#define commSTATS     ( BaseType_t ) 5
#define commSCHCK     ( BaseType_t ) 6
#define commSINIT     ( BaseType_t ) 7

#define COMMAND_SIZE        17

#define errINVALIDCOMMAND   -100
#define errMISSINGPARAM     -1000
#define errBADINPUT         -1001
#define errNOTFOUND         -1002

static TaskCode_t *pxTaskCodes;
static UBaseType_t xCount;

static TaskCode_t *pxFindTaskCode( char *pcName )
{
	TaskCode_t *pxReturn = NULL;
	for( UBaseType_t i = ( UBaseType_t ) 0U; i < xCount; i++ )
	{
		if( strcmp( pcName, pxTaskCodes[ i ].pcName ) == 0 )
		{
			pxReturn = &( pxTaskCodes[ i ] );
			break;
		}
	}
	return pxReturn;
}

static void input_handler();

static void vInput( void *pvParams )
{
	FILE *readFile = stdin;
	FILE *writeFile = stderr;
	input_handler( readFile, writeFile );
}

static void exception_handler( BaseType_t xError, FILE *file )
{
	if( xError != pdPASS ) {
		if( xError == errSCHEDULER_RUNNING )
			fprintf( file, "Cannot execute this command while the scheduler is active.\n" );
		else if( xError == errSCHEDULE_NOT_FEASIBLE )
			fprintf( file, "Schedule was not feasible with given set of tasks. Your batch has been dropped.\n" );
		else if( xError == errSERVER_NOT_FEASIBLE )
			fprintf( file, "Schedule was not feasible with given server parameters.\n" );
		else if( xError == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY )
			fprintf( file, "Couldn't allocate required memory.\n" );
		else if( xError == errNOTFOUND )
			fprintf( file, "Couldn't find desired item.\n" );
		else if( xError == errINVALIDCOMMAND )
			fprintf( file, "Your command wasn't recognized.\n" );
		else if( xError == errMISSINGPARAM )
			fprintf( file, "Command is missing additional parameters.\n" );
		else if( xError == errBADINPUT )
			fprintf( file, "Parameters couldn't be read properly.\n" );
	}
	fflush( file );
}

static TickStats_t * pxTickStats = NULL;
static TickType_t xStatGranularity = ( TickType_t ) 0U;

static void vWriteStatsTask( void *pvParams )
{
	FILE *statFile;
	statFile = fopen( "log.txt", "w" );

	if( statFile == NULL )
	{
		printf( "Error while opening file\n" );
		fflush( stdout );
		return;
	}

	taskENTER_CRITICAL();
	{
		for( UBaseType_t i = ( UBaseType_t ) 0U; i < ( UBaseType_t ) xStatGranularity * 2; i++ )
		{
			if( pxTickStats[ i ].xHandle == NULL )
			{
				break;
			}
			fprintf( statFile, "%u %p %u %ld\n", pxTickStats[ i ].xTick, pxTickStats[ i ].xHandle, pxTickStats[ i ].xCapacity, pxTickStats[ i ].xMarker );
		}
	}
	taskEXIT_CRITICAL();

	fclose( statFile );
	vTaskDelete(0);
}

static void input_handler( FILE *readFile, FILE *writeFile ) {
	for(;;)
	{
		BaseType_t xError = pdPASS;
		BaseType_t xCommand = errINVALIDCOMMAND;
		char pcCommand[ COMMAND_SIZE ];

		fscanf( readFile, "%s", pcCommand );

		if( strcmp( pcCommand, "add_task_periodic" ) == 0 )
			xCommand = commADDPD;
		else if( strcmp( pcCommand, "recalculate_wcet" ) == 0 )
			xCommand = commCWCET;
		else if( strcmp( pcCommand, "add_task" ) == 0 )
			xCommand = commADDAP;
		else if( strcmp( pcCommand, "show_task_codes" ) == 0 )
			xCommand = commSHWTC;
		else if( strcmp( pcCommand, "stop_task" ) == 0 )
			xCommand = commREMOV;
		else if( strcmp( pcCommand, "configure_stats" ) == 0 )
			xCommand = commSTATS;
		else if( strcmp( pcCommand, "get_max_server_capacity" ) == 0 )
			xCommand = commSCHCK;
		else if( strcmp( pcCommand, "initialise_server" ) == 0 )
			xCommand = commSINIT;
		else
			xError = errINVALIDCOMMAND;

		if( xCommand == commADDPD || xCommand == commADDAP ) {
			char pcName[ configMAX_TASK_NAME_LEN ];
			char pcFuncName[ FUNC_NAME_SIZE ];
			char pvParams[ configMAX_PARAM_LEN ];
			TaskHandle_t xHandle;

			if( fscanf( readFile, "%s %s %s", pcName, pcFuncName, pvParams ) != 3 )
			{
				xError = errMISSINGPARAM;
			}
			else if( strlen( pcName ) >= configMAX_TASK_NAME_LEN || strlen( pcFuncName ) >= FUNC_NAME_SIZE || strlen( pvParams ) >= configMAX_PARAM_LEN )
			{
				xError = errBADINPUT;
			}
			else
			{
				TaskCode_t *pxTaskCode = pxFindTaskCode( pcFuncName );
				if ( pxTaskCode == NULL )
				{
					xError = errNOTFOUND;
				}
				else
				{
					void ( *pfFunc )( void* pvParams ) = pxTaskCode->pfFunc;
					TickType_t xComputationTime = pxTaskCode->xComputationTime;
					TickType_t xArrivalTime = xTaskGetTickCount();
					TickType_t xPeriod = ( TickType_t ) 0U;
					TickType_t xOffset = ( TickType_t ) 0U;
					if( xCommand == commADDPD && fscanf( readFile, "%u", &xPeriod ) != 1 )
					{
						xError = errMISSINGPARAM;
					}
					else if( xCommand == commADDAP && fscanf( readFile, "%u", &xOffset ) != 1 )
					{
						xError = errMISSINGPARAM;
					}
					else
					{
						if( xCommand == commADDPD && xPeriod == ( TickType_t ) 0U )
						{
							xError = errBADINPUT;
						}
						else
						{
							xArrivalTime += xOffset;
							if( strcmp( pvParams, "NULL" ) == 0 )
							{
								pvParams = NULL;
							}
							xError = xTaskCreate( pfFunc, pcName, configMINIMAL_STACK_SIZE, pvParams, &xHandle, xArrivalTime, xPeriod, xComputationTime );
							if( xError == pdPASS )
							{
								fprintf( writeFile, "Handle: %p, Arrival: %u\n", xHandle, xArrivalTime );
							}
						}
					}
				}
			}
		}
		else if( xCommand == commCWCET )
		{
			vRecalculateWCET();
		}
		else if( xCommand == commSHWTC )
		{
			fprintf( writeFile, "%lu\n", xCount );
			for( UBaseType_t i = 0; i < xCount; i++ )
			{
				fprintf( writeFile, "%s %u\n", pxTaskCodes[ i ].pcName, pxTaskCodes[ i ].xComputationTime );
			}
		}
		else if( xCommand == commREMOV )
		{
			TaskHandle_t xHandle;

			if( fscanf( readFile, "%p", &xHandle ) != 1 )
			{
				xError = errMISSINGPARAM;
			}
			else
			{
				if( xHandle != 0 )
				{
					vTaskDelete( xHandle );
				}
			}
		}
		else if( xCommand == commSTATS && xStatGranularity == ( TickType_t ) 0U )
		{
			TaskHandle_t xHandle;
			if( fscanf( readFile, "%u", &xStatGranularity ) != 1 )
			{
				xError = errMISSINGPARAM;
			}
			else if( xStatGranularity == ( TickType_t ) 0U )
			{
				xError = errBADINPUT;
			}
			else
			{
				xError = xTaskCreate( vWriteStatsTask, "statwriter", configMINIMAL_STACK_SIZE, NULL, &xHandle, 0, xStatGranularity, 1 );
				if( xError == pdPASS )
				{
					fprintf( writeFile, "Handle: %p, Arrival: %u\n", xHandle, 0U );
				}
				else
				{
					xStatGranularity = ( TickType_t ) 0U;
				}
			}
		}
		else if( xCommand == commSCHCK )
		{
			TickType_t xCapacity;
			TickType_t xPeriod;

			if( fscanf( readFile, "%u", &xPeriod ) != 1 )
			{
				xError = errMISSINGPARAM;
			}
			else if( xPeriod == ( TickType_t ) 0U )
			{
				xError = errBADINPUT;
			}
			else
			{
				xError = xTaskCalcMaxServer( &xCapacity, xPeriod );
				if( xError == pdPASS )
				{
					fprintf( writeFile, "%u\n", xCapacity );
				}
			}
		}
		else if( xCommand == commSINIT )
		{
			TickType_t xCapacity;
			TickType_t xPeriod;

			if( fscanf( readFile, "%u %u", &xCapacity, &xPeriod ) != 2 )
			{
				xError = errMISSINGPARAM;
			}
			else if( xPeriod == ( TickType_t ) 0U )
			{
				xError = errBADINPUT;
			}
			else
			{
				xError = xTaskSetServer( xCapacity, xPeriod );
				if( xError == pdPASS ) {
					fprintf( writeFile, "Handle: %p, Arrival: %u\n", xTaskGetIdleTaskHandle(), 0U );
					fflush( writeFile );
					return;
				}
				else if( xError == errSCHEDULE_NOT_FEASIBLE )
				{
					xStatGranularity = ( TickType_t ) 0U;
				}
			}
		}
		exception_handler( xError, writeFile );
	}
}

int main( void )
{
	TaskCode_t xInput = { "vInput", vInput, 1 };
	pxTaskCodes = pxGetTaskCodes( xInput, &xCount );

	FILE *readFile = stdin;
	FILE *writeFile = stderr;

	input_handler( readFile, writeFile );
	if( xStatGranularity == ( TickType_t ) 0U )
	{
		vTaskStartScheduler( NULL, xStatGranularity );
	}
	else
	{
		TickStats_t pxStatsArray[ ( UBaseType_t ) xStatGranularity * 2 ];
		pxTickStats = pxStatsArray;
		vTaskStartScheduler( pxStatsArray, ( UBaseType_t ) xStatGranularity * 2 );
	}
	return 0;

}

