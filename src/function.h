/*
 * function.h
 *
 *  Created on: 5 Feb 2021
 *      Author: andro
 */

#ifndef FUNCTION_H_
#define FUNCTION_H_

#define FUNC_NAME_SIZE 17

typedef struct xTaskCode
{
	const char* pcName;
	void (*pfFunc)(void* pvParams);
	TickType_t xComputationTime;
} TaskCode_t;

void vRecalculateWCET();

TaskCode_t *pxGetTaskCodes(TaskCode_t xInput, BaseType_t *pxCount);

#endif /* FUNCTION_H_ */
