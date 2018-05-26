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


///////////////////////////////////////////////////////////////
//															 //
// Test 1: N threads, each one printing its number M times //
//															 //
///////////////////////////////////////////////////////////////



VOID Test_RunningReadyStates(UT_ARGUMENT Argument) {

	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;
	printf("\n:: Thread %c - BEGIN :: \n", Char);

	for (Index = 0; Index < 200; ++Index) {
		putchar(Char);

		if ((rand() % 4) == 0) {
			UtYield();
			UtDump();
		}
	}
	Char = (UCHAR)Argument;
	printf("\n:: Thread %c - END :: \n", Char);
}


VOID RunningReadyStates() {
	ULONG Index;


	printf("\n :: Test_RunningReadyStates - BEGIN :: \n\n");


	char str[12] = "thread_";
	for (Index = 0; Index < MAX_THREADS; ++Index) {
		itoa(Index, str + 7, 10);
		UtCreate(Test_RunningReadyStates, (UT_ARGUMENT)('0' + Index), 16 * 4096, str);
	}

	UtRun();

	printf("\n\n :: Test_RunningReadyStates - END :: \n");
}




int main() {
	UtInit();

	RunningReadyStates();
	getchar();

	UtEnd();
	return 0;
}


