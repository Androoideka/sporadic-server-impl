/*
 * function.c
 *
 *  Created on: 5 Feb 2021
 *      Author: andro
 */
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Custom includes. */
#include "function.h"

#define NUMBER_OF_TESTS 1000

static void spend_ticks( UBaseType_t xAmount )
{
	UBaseType_t xLimit = xAmount * 400000 + (xAmount - 1) * 180000;
	for( UBaseType_t i = ( UBaseType_t ) 0U; i < xLimit; i++ )
	{
		double o = 421634/2341;
	}
}

static void task0( void *pvParams )
{
	printf( "+" );
	fflush( stdout );
	vTaskDelete(0);
}

static void task1( void *pvParams )
{
	printf( "-" );
	fflush( stdout );
	vTaskDelete(0);
}

static void vTask1( void *pvParams )
{
	TickType_t startTick = xTaskGetTickCount();
	spend_ticks(1);
	TickType_t endTick = xTaskGetTickCount();
	printf( "Task 1 - %d\n", endTick - startTick );
	fflush( stdout );
	vTaskDelete(0);
}

static void vTask2( void *pvParams )
{
	TickType_t startTick = xTaskGetTickCount();
	spend_ticks(2);
	TickType_t endTick = xTaskGetTickCount();
	printf( "Task 2 - %d\n", endTick - startTick );
	fflush(stdout);
	vTaskDelete(0);
}

static void vTask3( void *pvParams )
{
	TickType_t startTick = xTaskGetTickCount();
	spend_ticks(3);
	TickType_t endTick = xTaskGetTickCount();
	printf( "Task 3 - %d\n", endTick - startTick );
	fflush(stdout);
	vTaskDelete(0);
}

static void vTask4( void *pvParams )
{
	TickType_t startTick = xTaskGetTickCount();
	spend_ticks(4);
	TickType_t endTick = xTaskGetTickCount();
	printf( "Task 4 - %d\n", endTick - startTick );
	fflush(stdout);
	vTaskDelete(0);
}

LARGE_INTEGER Frequency;

static TickType_t xCalculateWCET( void ( *pfFunc )( UBaseType_t ), UBaseType_t param )
{
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMilliseconds;
	TickType_t xWCET = ( TickType_t ) 0;

	for( UBaseType_t i = 0; i < NUMBER_OF_TESTS; i++ )
	{
		QueryPerformanceCounter( &StartingTime );
		pfFunc( param );
		QueryPerformanceCounter( &EndingTime );

		ElapsedMilliseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

		ElapsedMilliseconds.QuadPart *= 1000;
		ElapsedMilliseconds.QuadPart /= Frequency.QuadPart;

		TickType_t xCurrentRun = ElapsedMilliseconds.QuadPart;
		if( xCurrentRun > xWCET )
		{
			xWCET = xCurrentRun;
		}
	}

	return xWCET + 1;
}

static TaskCode_t pxTaskCodes[ 7 ];

void vRecalculateWCET()
{
	pxTaskCodes[ 2 ].xComputationTime = xCalculateWCET( spend_ticks, 1 );
	pxTaskCodes[ 3 ].xComputationTime = xCalculateWCET( spend_ticks, 2 );
	pxTaskCodes[ 4 ].xComputationTime = xCalculateWCET( spend_ticks, 3 );
	pxTaskCodes[ 5 ].xComputationTime = xCalculateWCET( spend_ticks, 4 );
}

TaskCode_t *pxGetTaskCodes( TaskCode_t xInput, UBaseType_t *pxCount )
{
	QueryPerformanceFrequency( &Frequency );

	pxTaskCodes[ 0 ].pcName = "task0";
	pxTaskCodes[ 0 ].pfFunc = task0;
	pxTaskCodes[ 0 ].xComputationTime = 1;

	pxTaskCodes[ 1 ].pcName = "task1";
	pxTaskCodes[ 1 ].pfFunc = task1;
	pxTaskCodes[ 1 ].xComputationTime = 1;

	pxTaskCodes[ 2 ].pcName = "vTask1";
	pxTaskCodes[ 2 ].pfFunc = vTask1;
	pxTaskCodes[ 2 ].xComputationTime = 1;

	pxTaskCodes[ 3 ].pcName = "vTask2";
	pxTaskCodes[ 3 ].pfFunc = vTask2;
	pxTaskCodes[ 3 ].xComputationTime = 2;

	pxTaskCodes[ 4 ].pcName = "vTask3";
	pxTaskCodes[ 4 ].pfFunc = vTask3;
	pxTaskCodes[ 4 ].xComputationTime = 3;

	pxTaskCodes[ 5 ].pcName = "vTask4";
	pxTaskCodes[ 5 ].pfFunc = vTask4;
	pxTaskCodes[ 5 ].xComputationTime = 4;

	pxTaskCodes[ 6 ] = xInput;

	*pxCount = 7;
	return pxTaskCodes;
}
