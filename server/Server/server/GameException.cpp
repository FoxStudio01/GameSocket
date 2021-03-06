
#include "gameDefine.h"


extern fvoid RunServerExit();

#ifdef WIN32



#include <stdio.h>
#include <tchar.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <mswsock.h>
#include <Windows.h>


#include <WindowsX.h>

#include <ShellAPI.h>

#include <dbghelp.h>
#pragma comment(linker, "/defaultlib:dbghelp.lib")

#define MAX_OUTPUT_MESSAGE		10000

HANDLE hProcess = NULL;
LPTOP_LEVEL_EXCEPTION_FILTER gPreviousFilter = NULL;
char ExceptionBuf[ MAX_OUTPUT_MESSAGE ];


char* GetExceptionString( DWORD dwCode )
{
#define EXCEPTION( x ) case EXCEPTION_##x: return (#x);

	switch ( dwCode )
	{
		EXCEPTION( ACCESS_VIOLATION )
			EXCEPTION( DATATYPE_MISALIGNMENT )
			EXCEPTION( BREAKPOINT )
			EXCEPTION( SINGLE_STEP )
			EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
			EXCEPTION( FLT_DENORMAL_OPERAND )
			EXCEPTION( FLT_DIVIDE_BY_ZERO )
			EXCEPTION( FLT_INEXACT_RESULT )
			EXCEPTION( FLT_INVALID_OPERATION )
			EXCEPTION( FLT_OVERFLOW )
			EXCEPTION( FLT_STACK_CHECK )
			EXCEPTION( FLT_UNDERFLOW )
			EXCEPTION( INT_DIVIDE_BY_ZERO )
			EXCEPTION( INT_OVERFLOW )
			EXCEPTION( PRIV_INSTRUCTION )
			EXCEPTION( IN_PAGE_ERROR )
			EXCEPTION( ILLEGAL_INSTRUCTION )
			EXCEPTION( NONCONTINUABLE_EXCEPTION )
			EXCEPTION( STACK_OVERFLOW )
			EXCEPTION( INVALID_DISPOSITION )
			EXCEPTION( GUARD_PAGE )
			EXCEPTION( INVALID_HANDLE )
	}

	// If not one of the "known" exceptions, try to get the string
	// from NTDLL.DLL's message table.

	static char szBuffer[512] = { 0 };

	FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
		GetModuleHandle( _T("NTDLL.DLL") ),
		dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

	return szBuffer;
}

BOOL GetLogicalAddress( PVOID addr, char* szModule, DWORD len, DWORD& section, DWORD& offset )
{
	MEMORY_BASIC_INFORMATION mbi;

	if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if ( !GetModuleFileNameA( (HMODULE)hMod, szModule, len ) )
		return FALSE;

	// Point to the DOS header in memory
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

	if ( !pDosHdr )
	{
		return FALSE;
	}

	// must windows nt , error use win7 or visita
	// From the DOS header, find the NT (PE) header
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

	DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

	// Iterate through the section table, looking for the one that encompasses
	// the linear address.
	for (   unsigned i = 0;
		i < pNtHdr->FileHeader.NumberOfSections;
		i++, pSection++ )
	{
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd = sectionStart
			+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		// Is the address in this section???
		if ( (rva >= sectionStart) && (rva <= sectionEnd) )
		{
			// Yes, address is in the section.  Calculate section and offset,
			// and store in the "section" & "offset" params, which were
			// passed by reference.
			section = i+1;
			offset = rva - sectionStart;


			return TRUE;
		}
	}

	return FALSE;   // Should never get here!
}


void WriteStackDetails(	PCONTEXT pContext, bool bWriteVariables )  // true if local/params should be output
{
	sprintf_s( ExceptionBuf, "%s\r\nCall stack:\r\n", ExceptionBuf );
	sprintf_s( ExceptionBuf, "%sAddress   Frame     Function\r\n", ExceptionBuf );

	DWORD dwMachineType = 0;
	// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

	STACKFRAME sf;
	memset( &sf, 0, sizeof(sf) );

#ifdef _M_IX86
	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	sf.AddrPC.Offset       = pContext->Eip;
	sf.AddrPC.Mode         = AddrModeFlat;
	sf.AddrStack.Offset    = pContext->Esp;
	sf.AddrStack.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset    = pContext->Ebp;
	sf.AddrFrame.Mode      = AddrModeFlat;

	dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif

	int i = 3;
	while ( 1 )
	{
		i--;
		// Get the next stack frame
		if ( ! StackWalk(  dwMachineType, hProcess,	GetCurrentThread(),	&sf, pContext, 0, SymFunctionTableAccess, SymGetModuleBase, 0 ) )
			break;

		if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
			break;                      // the frame is OK.  Bail if not.

		sprintf_s( ExceptionBuf, "%s%08X  %08X  ", ExceptionBuf, sf.AddrPC.Offset, sf.AddrFrame.Offset );
		/*
		// Get the name of the function for this stack frame entry
		BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + 1024 ];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLen = 1024;

		DWORD64 symDisplacement = 0;    // Displacement of the input address,
		// relative to the start of the symbol

		if ( SymFromAddr(hProcess,sf.AddrPC.Offset,&symDisplacement,pSymbol))
		{
		sprintf_s( ExceptionBuf, "%s%hs+%I64X", ExceptionBuf, pSymbol->Name, symDisplacement );
		}
		else    // No symbol found.  Print out the logical address instead.
		{
		char szModule[MAX_OUTPUT_MESSAGE];
		memset( szModule, 0, MAX_OUTPUT_MESSAGE );
		DWORD section = 0, offset = 0;

		GetLogicalAddress(  (PVOID)sf.AddrPC.Offset, szModule, sizeof(szModule), section, offset );

		sprintf_s( ExceptionBuf, "%s%04X:%08X", ExceptionBuf, section, offset );
		}

		*/
		// Get the source line for this stack frame entry
		IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
		DWORD dwLineDisplacement;
		if ( SymGetLineFromAddr( hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo ) )
		{
			sprintf_s( ExceptionBuf, "%s  %s line %u", ExceptionBuf,lineInfo.FileName,lineInfo.LineNumber); 
		}


		sprintf_s( ExceptionBuf, "%s\r\n", ExceptionBuf );

		/*
		// Write out the variables, if desired
		if ( bWriteVariables )
		{
		// Use SymSetContext to get just the locals/params for this frame
		IMAGEHLP_STACK_FRAME imagehlpStackFrame;
		imagehlpStackFrame.InstructionOffset = sf.AddrPC.Offset;
		SymSetContext( m_hProcess, &imagehlpStackFrame, 0 );

		// Enumerate the locals/parameters
		SymEnumSymbols( m_hProcess, 0, 0, EnumerateSymbolsCallback, &sf );

		_tprintf( _T("\r\n") );
		}
		*/
	}

}



LONG WINAPI UnknowExceptionHandler( _EXCEPTION_POINTERS* pExceptionInfo ) throw () 
{
	RunServerExit();
	

	memset( ExceptionBuf, 0, MAX_OUTPUT_MESSAGE );

	SYSTEMTIME time;
	GetSystemTime( &time );
	sprintf_s( ExceptionBuf, "\nException Time: %d:%d:%d (%d-%d-%d)\n\n", time.wHour + 8, time.wMinute, time.wSecond, time.wYear, time.wMonth, time.wDay );

	DWORD nThreadId  = ::GetCurrentThreadId();
	DWORD nProcessId = ::GetCurrentProcessId();
	hProcess = GetCurrentProcess();
	sprintf_s( ExceptionBuf, "%sProcess ID=%d, Thread ID=%d\n", ExceptionBuf, nProcessId, nThreadId );

	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;
	sprintf_s( ExceptionBuf, "%sException code: %08X %s\r\n", ExceptionBuf, pExceptionRecord->ExceptionCode, GetExceptionString(pExceptionRecord->ExceptionCode) );


	char szFaultingModule[MAX_OUTPUT_MESSAGE];
	DWORD section, offset;

 	if ( GetLogicalAddress(  pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof( szFaultingModule ), section, offset ) )
 		sprintf_s( ExceptionBuf, "%sFault address:  %08X %02X:%08X\r\nFile: %s\r\n", ExceptionBuf, pExceptionRecord->ExceptionAddress, section, offset, szFaultingModule );

	PCONTEXT pCtx = pExceptionInfo->ContextRecord;
	sprintf_s( ExceptionBuf, "%s\r\nRegisters:\r\n", ExceptionBuf );
	sprintf_s( ExceptionBuf, "%sEAX:%08X\tEBX:%08X\r\nECX:%08X\tEDX:%08X\r\nESI:%08X\tEDI:%08X\r\n", ExceptionBuf, pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx,pCtx->Esi, pCtx->Edi );
	sprintf_s( ExceptionBuf, "%sCS:EIP:%04X:%08X\r\n", ExceptionBuf,pCtx->SegCs, pCtx->Eip );
	sprintf_s( ExceptionBuf, "%sSS:ESP:%04X:%08X  EBP:%08X\r\n", ExceptionBuf,pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
	sprintf_s( ExceptionBuf, "%sDS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", ExceptionBuf,pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
	sprintf_s( ExceptionBuf, "%sFlags:%08X\r\n", ExceptionBuf,pCtx->EFlags );

	SymSetOptions( SYMOPT_DEFERRED_LOADS );
	// Initialize DbgHelp
	if ( !SymInitialize( GetCurrentProcess(), 0, TRUE ) )
	{
		FILE* fp = fopen("./Error.log" , "at+");
		if (!fp)
			fp = fopen("./Error.log" , "wb+");
		fwrite("\n________________________________________\n\n" , 1 , strlen("\n________________________________________\n\n") , fp);
		fwrite( ExceptionBuf, sizeof(char), strlen(ExceptionBuf), fp );
		fclose( fp );
		::MessageBoxA( NULL, ExceptionBuf, "program stopped.", MB_OK );

		if ( gPreviousFilter ) 
			return gPreviousFilter( pExceptionInfo ); 
		else 
			return EXCEPTION_CONTINUE_SEARCH; 
	}

	CONTEXT trashableContext = *pCtx;
	WriteStackDetails( &trashableContext, false );

	/*
	#ifdef _M_IX86  // X86 Only!


	sprintf_s( buf, "%s========================\r\n", buf );
	sprintf_s( buf, "%sLocal Variables And Parameters\r\n", buf );

	trashableContext = *pCtx;
	WriteStackDetails( &trashableContext, true );

	sprintf_s( buf, "%s========================\r\n", buf );
	sprintf_s( buf, "%sGlobal Variables\r\n", buf );

	SymEnumSymbols( GetCurrentProcess(), (DWORD64)GetModuleHandle(szFaultingModule), 0, EnumerateSymbolsCallback, 0 );

	#endif      // X86 Only!

	SymCleanup( GetCurrentProcess() );
	*/


	FILE* fp = fopen("./Error.log" , "at+");
	if (!fp)
		fp = fopen("./Error.log" , "wb+");
	fwrite("\n________________________________________\n\n" , 1 , strlen("\n________________________________________\n\n") , fp);
	fwrite( ExceptionBuf, sizeof(char), strlen(ExceptionBuf), fp );
	fclose( fp );

	::MessageBoxA( NULL, ExceptionBuf, "program stopped.", MB_OK );
	exit(1);

	/*
	if ( g_previousFilter ) 
	return g_previousFilter( pExceptionInfo ); 
	else 
	return EXCEPTION_CONTINUE_SEARCH; 
	*/
}

void InitException()
{
	gPreviousFilter = SetUnhandledExceptionFilter( UnknowExceptionHandler );

}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
using namespace FOXSDK;


static void printReason(int sig, siginfo_t * info, void *secret)
{
	RunServerExit();
    
	void* array[10];
	size_t size;
#ifdef F_USE_PRINT
	char** strings;
	size_t i;
	size = backtrace( array , 10 );
	strings = backtrace_symbols( array, (int)size );
	printf( "obtained %zd stack frames.\n" , size );
	for ( i = 0; i < size; i++ )
	{
		printf( "%s\n" , strings[ i ] );
	}
	free( strings );
#else
	int fd = open( "./error.log", O_CREAT | O_WRONLY );
	size = backtrace( array , 10 );
	backtrace_symbols_fd( array , (int)size , fd );
	close( fd );
#endif
	exit(0);
}

void InitException()
{
	struct sigaction myAction;
	myAction.sa_sigaction = printReason;
	sigemptyset( &myAction.sa_mask );
	myAction.sa_flags = SA_RESTART | SA_SIGINFO;
	sigaction( SIGSEGV , &myAction , NULL );
	sigaction( SIGUSR1 , &myAction , NULL );
	sigaction( SIGFPE , &myAction , NULL );
	sigaction( SIGILL , &myAction , NULL );
	sigaction( SIGBUS , &myAction , NULL );
	sigaction( SIGABRT , &myAction , NULL );
	sigaction( SIGSYS , &myAction , NULL );
	sigaction( SIGINT , &myAction , NULL );

}




#endif
