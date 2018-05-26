// SimpleTest.cpp : Defines the entry point for the console application.
//
#include <malloc.h>
 
 
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

  

VOID stackSizeTest(UT_ARGUMENT Argument) {
	UCHAR Char;
	Char = (UCHAR)Argument;
	printf("\n Thread %c - size : %d  \n", Char,utGetStackSize(UtSelf()));
	UtExit();
}

VOID Test1() {
	ULONG Index;

	 

 

	printf("\n :: stackSizeTest - BEGIN :: \n\n");
	char str[12] = "thread_";
	for (Index = 1; Index <= MAX_THREADS; ++Index) {
		itoa(Index, str + 7, 10);
		UtCreate(stackSizeTest, (UT_ARGUMENT)('0' + Index), Index * 16 * 4096, str);
	}

	printf("Stack size of all threads(index*16*4096 filled stack):\n");
	UtRun();
	 
	printf("\n\n :: stackSizeTest - END :: \n");
}




int main() {
	UtInit();

	Test1();
	getchar();


	UtEnd();
	return 0;
}


