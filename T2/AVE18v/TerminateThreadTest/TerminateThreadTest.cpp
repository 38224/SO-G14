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


VOID Test2_Thread1(UT_ARGUMENT Argument) {
	for (int i = 0; i < 10; i++) {
		printf("thread_1:counting to 10: %d\n", i);
		UtYield();
		printf("thread_1:I was supposed to have stopped...('failed to stop')\n");
	}
}

VOID Test2_Thread2(UT_ARGUMENT Argument) {
	HANDLE thread = (HANDLE)Argument;
	printf("thread_2:Stopping thread_1\n");
	UtTerminateThread(thread);
	printf("thread_2:Thread_1 stopped, wont print again \n");
}

VOID Test2_Thread3(UT_ARGUMENT Argument) {
	printf("thread_3:Stopping self thread and i wont end my task to print again\n");
	HANDLE thread = UtSelf();
	UtTerminateThread(thread);
	printf("thread_3:I was supposed to have stopped...('failed to stop')\n");
}


VOID Test2() {
	printf("TerminateThreadTest start! \n\n");
	HANDLE first = UtCreate(Test2_Thread1, NULL, 16 * 4096, "thread_1");
	UtCreate(Test2_Thread2, (UT_ARGUMENT)first, 16*4096,"thread_2");
	UtCreate(Test2_Thread3, NULL, 16 * 4096, "thread_3");
	UtRun();
	printf("\n\nTerminateThreadTest done");
}

int main() {
	UtInit();

	Test2();


	UtRun();
	getchar();
	UtEnd();
	return 0;
}


