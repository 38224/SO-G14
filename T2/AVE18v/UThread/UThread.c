/////////////////////////////////////////////////////////////////
//
// CCISEL 
// 2007-2011
//
// UThread library:
//   User threads supporting cooperative multithreading.
//
// Authors:
//   Carlos Martins, João Trindade, Duarte Nunes, Jorge Martins
// 

#include <malloc.h>
#include <stdio.h>
#include <crtdbg.h>
#include "UThreadInternal.h"
#include "WaitBlock.h"

//////////////////////////////////////
//
// UThread internal state variables.
//

//
// The number of existing user threads.
//
static
ULONG NumberOfThreads;
 

//
// The sentinel of the circular list linking the user threads that are
// currently schedulable. The next thread to run is retrieved from the
// head of this list.
//
static
LIST_ENTRY ReadyQueue;

//
// The currently executing thread.
//
#ifndef UTHREAD_X64
static
#endif
PUTHREAD RunningThread;



//
// The user thread proxy of the underlying operating system thread. This
// thread is switched back in when there are no more runnable user threads,
// causing the scheduler to exit.
//
static
PUTHREAD MainThread;

////////////////////////////////////////////////
//
// Forward declaration of internal operations.
//

//
// The trampoline function that a user thread begins by executing, through
// which the associated function is called.
//
static
VOID InternalStart();


#ifdef UTHREAD_X64
//
// Performs a context switch from CurrentThread to NextThread.
// In x64 calling convention CurrentThread is in RCX and NextThread in RDX.
//
VOID __fastcall  ContextSwitch(PUTHREAD CurrentThread, PUTHREAD NextThread);

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// In x64 calling convention  CurrentThread is in RCX and NextThread in RDX.
//
VOID __fastcall InternalExit(PUTHREAD Thread, PUTHREAD NextThread);

#else

static
VOID __fastcall ContextSwitch(PUTHREAD CurrentThread, PUTHREAD NextThread);

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX
// and NextThread in EDX.
//
static
VOID __fastcall InternalExit(PUTHREAD Thread, PUTHREAD NextThread);
#endif

////////////////////////////////////////
//
// UThread inline internal operations.
//

//
// Returns and removes the first user thread in the ready queue. If the ready
// queue is empty, the main thread is returned.
//
static
FORCEINLINE
PUTHREAD ExtractNextReadyThread() {
	return IsListEmpty(&ReadyQueue)
		? MainThread
		: CONTAINING_RECORD(RemoveHeadList(&ReadyQueue), UTHREAD, Link);
}
	/*

	return IsListEmpty(&ReadyQueue)
		? MainThread
		: CONTAINING_RECORD(RemoveHeadList(&ReadyQueue), UTHREAD, Link);
		*/

//
// Schedule a new thread to run
//
static
FORCEINLINE
VOID Schedule() {
	PUTHREAD NextThread;
	NextThread = ExtractNextReadyThread();
	NextThread->State = Running;
	ContextSwitch(RunningThread, NextThread);
}

///////////////////////////////
//
// UThread public operations.
//

//
// Initialize the scheduler.
// This function must be the first to be called. 
//
VOID UtInit() {
	InitializeListHead(&ReadyQueue);
 
}

//
// Cleanup all UThread internal resources.
//
VOID UtEnd() {
	/* (this function body was intentionally left empty) */
}

//
// Run the user threads. The operating system thread that calls this function
// performs a context switch to a user thread and resumes execution only when
// all user threads have exited.
//
VOID UtRun() {
	UTHREAD Thread; // Represents the underlying operating system thread.

					//
					// There can be only one scheduler instance running.
					//
	_ASSERTE(RunningThread == NULL);

	//
	// At least one user thread must have been created before calling run.
	//
	if (IsListEmpty(&ReadyQueue)) {
		return;
	}

	//
	// Switch to a user thread.
	//
	MainThread = &Thread;
	RunningThread = MainThread;
	Schedule();

	//
	// When we get here, there are no more runnable user threads.
	//
	_ASSERTE(IsListEmpty(&ReadyQueue));
	_ASSERTE(NumberOfThreads == 0);

	//
	// Allow another call to UtRun().
	//
	RunningThread = NULL;
	MainThread = NULL;
}




//
// Terminates the execution of the currently running thread. All associated
// resources are released after the context switch to the next ready thread.
//
VOID UtExit() {
	NumberOfThreads -= 1;

	while (!IsListEmpty(&RunningThread->JoinList)) {
		PLIST_ENTRY curEntry = RemoveHeadList(&RunningThread->JoinList);
		PWAIT_BLOCK waitBlock = CONTAINING_RECORD(curEntry, WAIT_BLOCK, Link);
		PUTHREAD Thread = (PUTHREAD)waitBlock->Thread;
		Thread->JoinCounter -= 1;
		if (Thread->JoinCounter == 0) {
			InsertTailList(&ReadyQueue, &Thread->Link);
			Thread->State = Ready;
		}
		free(waitBlock);
	}
 
	InternalExit(RunningThread, ExtractNextReadyThread());
	_ASSERTE(!"Supposed to be here!");
}

//
// Relinquishes the processor to the first user thread in the ready queue.
// If there are no ready threads, the function returns immediately.
//
VOID UtYield() {
	if (!IsListEmpty(&ReadyQueue)) {
		InsertTailList(&ReadyQueue, &RunningThread->Link);
		RunningThread->State = Ready;
		Schedule();
	}
}

//
// Returns a HANDLE to the executing user thread.
//
HANDLE UtSelf() {
	return (HANDLE)RunningThread;
}
 
int utGetStackSize(HANDLE ThreadHandle) {
	int count = 0;
	PUTHREAD t = (PUTHREAD)ThreadHandle;
	while (*(t->Stack + count++) != '\0');
	return count;
 
}


//
// Halts the execution of the current user thread.
//
VOID UtDeactivate() {
	Schedule();
}


//
// Places the specified user thread at the end of the ready queue, where it
// becomes eligible to run.
//
VOID UtActivate(HANDLE ThreadHandle) {
	PUTHREAD Thread = (PUTHREAD)ThreadHandle;
	InsertTailList(&ReadyQueue, &Thread->Link);
	Thread->State = Ready;
}
//Realize a função UtDump, útil para debug, que mostra no standard output a lista das threads existentes,
//apresentando o seu handle, o nome, o estado e a taxa de ocupação do stack.   
VOID UtDump() {
	printf("\n\n\nRUNNING THREAD info(Handle,Name,State:Running = 2, Ready = 1, Blocked = 0,STACK size) :\n");
	printf("%d	%s	%d	%d \n\n",(HANDLE)RunningThread,RunningThread->Name , UtThreadState(RunningThread), utGetStackSize(RunningThread));
	
	printf("READY THREAD info(Handle,Name,State:Running = 2, Ready = 1, Blocked = 0,STACK size) :\n");

	PUTHREAD dummy = RunningThread->Link.Flink ;

	PUTHREAD  stopPoint = dummy;

	while (dummy->Link.Flink != stopPoint) {

		printf("%d	%s	%d	%d\n", (HANDLE)dummy, dummy->Name, UtThreadState(dummy), utGetStackSize(dummy));
		dummy = dummy->Link.Flink;

	}
	printf("\n\n");
}

INT UtThreadState(HANDLE ThreadHandle) {
	return ((PUTHREAD)ThreadHandle)->State;
}

 
VOID InternalStart() {
	RunningThread->Function(RunningThread->Argument);
	UtExit();
}

//
// Blocks the invoking thread until the termination of all the specified
// user threads.
//
BOOL UtMultJoin(HANDLE handle[], int size) {
	for (int i = 0; i < size; ++i) {
		if (handle[i] == UtSelf() || UtThreadState(handle[i]) == Blocked) return FALSE;
	}

	for (int i = 0; i < size; ++i) {
		PWAIT_BLOCK waitBlock = (PWAIT_BLOCK)malloc(sizeof(WAIT_BLOCK));
		InitializeWaitBlock(waitBlock);
		PUTHREAD Thread = (PUTHREAD)handle[i];
		InsertTailList(&Thread->JoinList, &waitBlock->Link);
		RunningThread->JoinCounter += 1;
	}
	RunningThread->State = Blocked;
	UtDeactivate();
	return TRUE;
}

//
// functions with implementation dependent of X86 or x64 platform
//

#ifndef UTHREAD_X64
//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreate(UT_FUNCTION Function, UT_ARGUMENT Argument, size_t stackSize, char* threadName) {
	PUTHREAD Thread;

	if (stackSize == 0) stackSize = STACK_SIZE;


	//
	// Dynamically allocate an instance of UTHREAD and the associated stack.
	//
	Thread = (PUTHREAD)malloc(sizeof(UTHREAD));
	Thread->Stack = (PUCHAR)malloc(stackSize);
 
	_ASSERTE(Thread != NULL && Thread->Stack != NULL);

	//
	// Zero the stack for emotional confort.
	//
	memset(Thread->Stack, 1, stackSize);

	memcpy(Thread->Name, threadName, NAME_SIZE);
	//
	// Memorize Function and Argument for use in InternalStart.
	//
	Thread->Function = Function;
	Thread->Argument = Argument;
	 
	//
	// Map an UTHREAD_CONTEXT instance on the thread's stack.
	// We'll use it to save the initial context of the thread.
	//
	// +------------+
	// | 0x00000000 |    <- Highest word of a thread's stack space
	// +============+       (needs to be set to 0 for Visual Studio to
	// |  RetAddr   | \     correctly present a thread's call stack).
	// +------------+  |
	// |    EBP     |  |
	// +------------+  |
	// |    EBX     |   >   Thread->ThreadContext mapped on the stack.
	// +------------+  |
	// |    ESI     |  |
	// +------------+  |
	// |    EDI     | /  <- The stack pointer will be set to this address
	// +============+       at the next context switch to this thread.
	// |            | \
		// +------------+  |
// |     :      |  |
//       :          >   Remaining stack space.
// |     :      |  |
// +------------+  |
// |            | /  <- Lowest word of a thread's stack space
// +------------+       (Thread->Stack always points to this location).
//

	Thread->ThreadContext = (PUTHREAD_CONTEXT)(Thread->Stack +
		stackSize - sizeof(ULONG) - sizeof(UTHREAD_CONTEXT));

	//
	// Set the thread's initial context by initializing the values of EDI,
	// EBX, ESI and EBP (must be zero for Visual Studio to correctly present
	// a thread's call stack) and by hooking the return address.
	// 
	// Upon the first context switch to this thread, after popping the dummy
	// values of the "saved" registers, a ret instruction will place the
	// address of InternalStart on EIP.
	//
	Thread->ThreadContext->EDI = 0x33333333;
	Thread->ThreadContext->EBX = 0x11111111;
	Thread->ThreadContext->ESI = 0x22222222;
	Thread->ThreadContext->EBP = 0x00000000;
	Thread->ThreadContext->RetAddr = InternalStart;


	InitializeListHead(&Thread->JoinList);
	Thread->JoinCounter = 0;
	//
	// Ready the thread.
	//
	NumberOfThreads += 1;
	UtActivate((HANDLE)Thread);

	return (HANDLE)Thread;
}

//
// Performs a context switch from CurrentThread to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX and NextThread in EDX.
// __declspec(naked) directs the compiler to omit any prologue or epilogue.
//
__declspec(naked)
VOID __fastcall ContextSwitch(PUTHREAD CurrentThread, PUTHREAD NextThread) {
	__asm {
		// Switch out the running CurrentThread, saving the execution context on the thread's own stack.   
		// The return address is atop the stack, having been placed there by the call to this function.
		//
		push	ebp
		push	ebx
		push	esi
		push	edi
		//
		// Save ESP in CurrentThread->ThreadContext.
		//
		mov		dword ptr[ecx].ThreadContext, esp
		//
		// Set NextThread as the running thread.
		//
		mov     RunningThread, edx
		//
		// Load NextThread's context, starting by switching to its stack, where the registers are saved.
		//
		mov		esp, dword ptr[edx].ThreadContext
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		//
		// Jump to the return address saved on NextThread's stack when the function was called.
		//
		ret
	}
}

//
// Frees the resources associated with Thread.
// __fastcall sets the calling convention such that Thread is in ECX.
//

static
VOID __fastcall CleanupThread(PUTHREAD Thread) {
	free(Thread->Stack);
	free(Thread);
}

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX and NextThread in EDX.
// __declspec(naked) directs the compiler to omit any prologue or epilogue.
//
__declspec(naked)
VOID __fastcall InternalExit(PUTHREAD CurrentThread, PUTHREAD NextThread) {
	__asm {

		//
		// Set NextThread as the running thread.
		//
		mov     RunningThread, edx

		//
		// Load NextThread's stack pointer before calling CleanupThread(): making the call while
		// using CurrentThread's stack would mean using the same memory being freed -- the stack.
		//
		mov		esp, dword ptr[edx].ThreadContext

		call    CleanupThread

		//
		// Finish switching in NextThread.
		//
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret
	}
}

#else

//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreate(UT_FUNCTION Function, UT_ARGUMENT Argument, size_t stackSize, char* threadName) {
	
	PUTHREAD Thread;

	if (stackSize == 0) stackSize = STACK_SIZE;
	//
	// Dynamically allocate an instance of UTHREAD and the associated stack.
	//
	Thread = (PUTHREAD)malloc(sizeof(UTHREAD));
	Thread->Stack = (PUCHAR)malloc(stackSize);
 
	_ASSERTE(Thread != NULL && Thread->Stack != NULL);

	//
	// Zero the stack for emotional confort.
	//
	memset(Thread->Stack, 0, stackSize);
	memcpy(Thread->Name, threadName, 256);
	//
	// Memorize Function and Argument for use in InternalStart.
	//
	Thread->Function = Function;
	Thread->Argument = Argument;

	//
	// Map an UTHREAD_CONTEXT instance on the thread's stack.
	// We'll use it to save the initial context of the thread.
	//
	// +------------+  <- Highest word of a thread's stack space
	// | 0x00000000 |    (needs to be set to 0 for Visual Studio to
	// +------------+      correctly present a thread's call stack).   
	// | 0x00000000 |  \
		// +------------+   |
// | 0x00000000 |   | <-- Shadow Area for Internal Start 
// +------------+   |
// | 0x00000000 |   |
// +------------+   |
// | 0x00000000 |  /
// +============+       
// |  RetAddr   | \    
// +------------+  |
// |    RBP     |  |
// +------------+  |
// |    RBX     |   >   Thread->ThreadContext mapped on the stack.
// +------------+  |
// |    RDI     |  |
// +------------+  |
// |    RSI     |  |
// +------------+  |
// |    R12     |  |
// +------------+  |
// |    R13     |  |
// +------------+  |
// |    R14     |  |
// +------------+  |
// |    R15     | /  <- The stack pointer will be set to this address
// +============+       at the next context switch to this thread.
// |            | \
	// +------------+  |
// |     :      |  |
//       :          >   Remaining stack space.
// |     :      |  |
// +------------+  |
// |            | /  <- Lowest word of a thread's stack space
// +------------+       (Thread->Stack always points to this location).
//

	Thread->ThreadContext = (PUTHREAD_CONTEXT)(Thread->Stack +
		stackSize - sizeof(UTHREAD_CONTEXT) - sizeof(ULONGLONG) * 5);

	//
	// Set the thread's initial context by initializing the values of 
	// registers that must be saved by the called (R15,R14,R13,R12, RSI, RDI, RBCX, RBP)

	// 
	// Upon the first context switch to this thread, after popping the dummy
	// values of the "saved" registers, a ret instruction will place the
	// address of InternalStart on EIP.
	//
	Thread->ThreadContext->R15 = 0x77777777;
	Thread->ThreadContext->R14 = 0x66666666;
	Thread->ThreadContext->R13 = 0x55555555;
	Thread->ThreadContext->R12 = 0x44444444;
	Thread->ThreadContext->RSI = 0x33333333;
	Thread->ThreadContext->RDI = 0x11111111;
	Thread->ThreadContext->RBX = 0x22222222;
	Thread->ThreadContext->RBP = 0x11111111;
	Thread->ThreadContext->RetAddr = InternalStart;

	//
	// Ready the thread.
	//
	NumberOfThreads += 1;
	UtActivate((HANDLE)Thread);

	return (HANDLE)Thread;
}


//
// Frees the resources associated with Thread.
// In X64 calling convention Thread comes in RCX.
//
VOID CleanupThread(PUTHREAD Thread) {
	free(Thread->Stack);
 
	free(Thread);
}


#endif