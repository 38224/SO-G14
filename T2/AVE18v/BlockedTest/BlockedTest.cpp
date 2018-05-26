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


HANDLE thread_0, thread_1, thread_2;


VOID Test_BlockedThreads(UT_ARGUMENT Argument) {
		UCHAR Char;
		ULONG Index;
		Char = (UCHAR)Argument;
		printf("\n:: Thread %c - BEGIN :: \n", Char);
		printf("Thread_0 is currently in state : %i\n", UtThreadState(thread_0));
		printf("Thread_1 is currently in state : %i\n", UtThreadState(thread_1));
		printf("Thread_2 is currently in state : %i\n", UtThreadState(thread_2));
		printf(":: Thread %c - JOINING THREAD 1 :: \n", Char);
		HANDLE handle[1] = { thread_2 };
		UtMultJoin(handle,1);
		printf("\n\n\n Thread %c - END :: \n\n\n", Char);
}

VOID Test_Yield(UT_ARGUMENT Argument) {

	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;

	for (Index = 0; Index < 1000; ++Index) {
		putchar(Char);

		if ((rand() % 4) == 0) {
			UtYield();
			UtDump();
		}
	}
}


VOID RunningReadyStates() {
	ULONG Index;


	printf("\n :: Test_BlockedThreads - BEGIN :: \n\n");


	thread_0 = UtCreate(Test_BlockedThreads, (UT_ARGUMENT)'0', 16 * 4096, "thread_0");
	thread_1 = UtCreate(Test_Yield, (UT_ARGUMENT)'1', 16 * 4096, "thread_1");
	thread_2 = UtCreate(Test_Yield, (UT_ARGUMENT)'2', 16 * 4096, "thread_2");

	UtRun();

	printf("\n\n :: Test_BlockedThreads - END :: \n");
}




int main() {
	UtInit();

	RunningReadyStates();
	getchar();

	UtEnd();
	return 0;
}


