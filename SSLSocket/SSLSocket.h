/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SSLSocket.h
// BaseLibary: Secure and non-secure sockets
// 
//  ______    _          _         _    _       _
// |  ____|  | |        (_)       | |  | |     (_)
// | |__   __| |_      ___  __ _  | |__| |_   _ _ ___ _ __ ___   __ _ _ __  
// |  __| / _` \ \ /\ / / |/ _` | |  __  | | | | / __| '_ ` _ \ / _` | '_ \
// | |___| (_| |\ V  V /| | (_| | | |  | | |_| | \__ \ | | | | | (_| | | | |
// |______\__,_| \_/\_/ |_|\__, | |_|  |_|\__,_|_|___/_| |_| |_|\__,_|_| |_|
//                          __/ |                                           
//                         |___/                                            
//
// Created: 2014-2026 ir. W.E. Huisman
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#pragma once

// Minimal requirements from the MS-Windows OS
#include <windows.h>

// For the memory leak checks.
//
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#pragma warning(push)
#pragma warning(disable: 4005)
#include <crtdbg.h>
#pragma warning(pop)
#define alloc_new new(_NORMAL_BLOCK,__FILE__,__LINE__)
// Possibly replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
#else
#define alloc_new new
#endif

// Selecting the right library to link with automatically
// So we do not need to worry about which library to use in the linker settings
// As long as we use the $(SolutionDir)Lib\ as the library location of all our projects

#ifdef _UNICODE
#if defined _M_IX86
#define SSLSOCKET_PLATFORM "Ux86"
#else
#define SSLSOCKET_PLATFORM "Ux64"
#endif

#else // UNICODE

#if defined _M_IX86
#define SSLSOCKET_PLATFORM "x86"
#else
#define SSLSOCKET_PLATFORM "x64"
#endif

#endif  // UNICODE

#if defined _DEBUG
#define SSLSOCKET_CONFIGURATION "D"
#else
#define SSLSOCKET_CONFIGURATION "R"
#endif 

#ifndef SSLSOCKET_NOAUTOLINK
#pragma comment(lib,"SSLSocket_"                        SSLSOCKET_PLATFORM SSLSOCKET_CONFIGURATION ".lib")
#pragma message("Automatically linking with SSLSocket_" SSLSOCKET_PLATFORM SSLSOCKET_CONFIGURATION ".lib")
#else
#pragma message("Creating library SSLSocket_" SSLSOCKET_PLATFORM SSLSOCKET_CONFIGURATION ".lib")
#endif 
