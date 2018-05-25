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

ULONG Test1_Count;
int count = 0;


VOID Test1_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;

	for (Index = 0; Index < 100000; ++Index) {
		putchar(Char);

		if ((rand() % 1000) == 0) {
			count++;
			UtDump();
			UtYield();
			
		}
	}

	++Test1_Count;
	printf("%d", count);
}

VOID Test1() {
	ULONG Index;


	Test1_Count = 0;



	printf("\n :: Test 1 - BEGIN :: \n\n");
	char str[12] = "thread_";
	for (Index = 0; Index < MAX_THREADS; ++Index) {
		itoa(Index, str + 7, 10);
		UtCreate(Test1_Thread, (UT_ARGUMENT)('0' + Index), 16 * 4096, str);
	}


	UtRun();

	_ASSERTE(Test1_Count == MAX_THREADS);
	printf("\n\n :: Test 1 - END :: \n");
}




int main() {
	UtInit();

	Test1();
	getchar();


	UtEnd();
	return 0;
}


