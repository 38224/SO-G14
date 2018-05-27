// SimpleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "usynch.h"
#include "List.h"

/////////////////////////////////////////////
//
// CCISEL 
// 2007-2010
//
// UThread    Library First    Test
//
// Jorge Martins, 2011
////////////////////////////////////////////
#define DEBUG

#define MAX_THREADS 10

#include <crtdbg.h>
#include <stdio.h>

#include "..\Include\Uthread.h"

 
VOID Do_Nothing(UT_ARGUMENT Argument) {
	return;
}


VOID Thread_Commutation_Timer() {
	DWORD initial = GetTickCount();
	int number_of_threads = 100000;
	for (int i = 0; i < number_of_threads; i++)
		UtCreate(Do_Nothing, (UT_ARGUMENT)('0' + i), 4096, "thread_X");
	UtRun();

	DWORD duration = GetTickCount() - initial;
	printf("\n :: Testing commutation time for %d threads", number_of_threads);
	printf("\n\n :: Execution took %d miliseconds", duration);
	printf("\n\n :: Thread commutation time is: %f miliseconds", float(duration) / number_of_threads);

}


int main() {
	UtInit();

	Thread_Commutation_Timer();
	getchar();

	UtEnd();
	return 0;
}


