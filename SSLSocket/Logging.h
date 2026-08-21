/////////////////////////////////////////////////////////////////////////////
// 
// SSL Socket Library
//
// Original idea:
// David Maw: https://www.codeproject.com/Articles/1000189/A-Working-TCP-Client-and-Server-With-SSL
//
#pragma once

// Logging level in the SSL socket library
//
#define SOCK_LOGGING_OFF       0    // No logging
#define SOCK_LOGGING_ON        1    // Results logging
#define SOCK_LOGGING_TRACE     2    // Hexdump tracing first line
#define SOCK_LOGGING_FULLTRACE 3    // Full hexdump tracing

extern int SSL_socket_logging;  // Holds the current logging level

// Definition of a 'real' printing function
typedef void(__stdcall *OutputString)(LPCTSTR lpOutputString);
// Holds the 'real' printing function, defaulting to "OutputDebugString"
extern OutputString printing;

// Functions

void LogError(const TCHAR* p_format,...);
void DebugMsg(const TCHAR* p_format,...);
void PrintHexDump(DWORD p_length,const void* p_buffer);
void SetSocketLogging(int p_logging);
