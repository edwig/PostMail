////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: PostMail.cpp
//
// Mailer to post an e-mail to the DARPA-net
// 
// Written by W.E. Huisman (2006-2026)
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
#include "Profiles.h"
#include "OdataConfig.h"
#include "MailCache.h"
#include <LogAnalysis.h>
#include <vector>

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

extern TCHAR WhoAmI[];
#define COMPANY               _T("EDO")
#define APPLICATION           _T("Postmail")
#define MAX_LINELEN           1024
#define PRODUCT_REGISTRY      _T("EDO\\Postmail")
#define LOGFILE_OPEN_RETRIES  4

// Log levels of PostMail
#define LOGLEVEL_TERSE        0   // Always print to logfile (headers and such)
#define LOGLEVEL_CLIENT       1   // Outgoing client commands
#define LOGLEVEL_SERVER       2   // Incoming server answers
#define LOGLEVEL_TRACE        3   // Add full hexadecimal tracing of the socket
#define LOGLEVEL_ERROR        4   // Internal, show immediately

XString SystemError();
class   SMTPMessage;

// Logging from SSLSocket to PostMail
void __stdcall PostMailLogString(LPCTSTR lpOutputString);

class PostMail : public CWinApp
{
public:
	PostMail();

  int           WideMessageBox(HWND p_hwnd,LPCTSTR p_message,LPCTSTR p_title ,int p_buttons = MB_OK);
  XString       WindowsError();
  bool          OpenLogfile(int loglevel = 2);
  void          CloseLogfile();
  void          Log(int level,const XString& text,bool newline = true);
  void          LogString(LPCTSTR p_string);
  void          LogHexString(LPCTSTR p_string);
  void          WriteChanges(const XString& p_type,const XString& p_text);
  void          DisplayLogfile();
  bool          GetViewer()     { return m_viewer;     }
  bool          GetServer()     { return m_server;     }
  bool          GetTextMode()   { return m_text;       }
  int           GetLoglevel()   { return m_logLevel;   }
  XString       WorkingDir()    { return m_workingDir; } 
  Profiles&     GetProfiles();
  ODataConfig&  GetODataConfig();
  void          ResetErrorState();
  void          DoHelpPage();
  void          DoHelpFile();
  void          DoStackErrors(bool p_stack);

  // Overrides
public:
	virtual BOOL InitInstance() override;
  virtual int  ExitInstance() override;

  // Implementation
	DECLARE_MESSAGE_MAP()

private:
  void    GetWorkingDir();
  void    GetOptionsFromCommandline();
  void    HandleMailFile(SMTPMessage& p_mail,bool p_graph);
  bool    HandleIDCache (SMTPMessage& p_mail,bool p_graph);
  void    NotifyResults (SMTPMessage& p_mail);
  void    GetDefaultProfile(XString& p_line);
  void    GetIDOption(XString& p_line);
  void    GetLogFile (XString& p_line);
  void    GetReconnectCommand(const XString& p_line);
  void    GetReadyCommand(XString& p_line);
  void    GetPingCommand(XString& p_line);
  void    GetProfileCommand(XString& p_line);
  void    ChooseProfile();
  void    CheckRuntimer(XString p_runtimer);
  void    CheckMailServers();
  void    LogInCentralLogfile(SMTPMessage& p_mail);
  int     RunERPCommand(XString p_commando);
  void    NotifyERPFramework();
  bool    CheckJSONConfig();
  bool    CheckSMTP();
  bool    CheckGraph();
  BOOL    IsUserInteractive();

  // WinFile   m_file;

  XString   m_mailFilename;
  XString   m_chngFilename;
  XString   m_totalErrorState;
  XString   m_workingDir;
  XString   m_logFilename;
  int       m_logLevel;
  bool      m_logAppend;
  XString   m_pingCommand;
  XString   m_reconnectCommand;
  XString   m_readyCommand;
  XString   m_profileCommand;
  // Return value to the operating system
  int       m_returnValue;
  bool      m_server;
  bool      m_text;
  bool      m_viewer;
  bool      m_config;
  bool      m_doProfile;
  bool      m_doHelp;
  bool      m_doOutbox;
  int       m_defaultProfile;
  bool      m_stackErrors;
  bool      m_doLogfile;
  bool      m_doHexlog;
  bool      m_doEncrypt;
  XString   m_profile;
  Profiles      m_profiles;
  ODataConfig   m_odataConfig;
  LogAnalysis*  m_logfile;
  Profile*      m_currentProfile;
  // ID Cache
  bool      m_useIDCache;
  unsigned  m_mailID;
  XString   m_mailParam;
  XString   m_parameter;
  MailCache m_cache;
};

inline Profiles& 
PostMail::GetProfiles()
{
  return m_profiles;
}

inline ODataConfig&
PostMail::GetODataConfig()
{
  return m_odataConfig;
}

inline void
PostMail::ResetErrorState()
{
  m_totalErrorState.Empty();
}

inline void
PostMail::DoStackErrors(bool p_stack)
{
  m_stackErrors = p_stack;
}

extern PostMail theApp;