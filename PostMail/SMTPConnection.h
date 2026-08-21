/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPConnection.h
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
#include "stdafx.h"
#include "SMTPAddress.h"
#include "SMTPMessage.h"
#include "SecureClientSocket.h"
#include "EventWrapper.h"
#include "RelayDlg.h"
#include "Message.h"

#define  SECURITY_WIN32
#include <security.h>
#include <sspi.h>

#ifndef SEC_SUCCESS
#define SEC_SUCCESS(Status) ((Status) >= 0)
#endif

// Milliseconds waiting on server
// Profiled: Wait time drops 25 % until 100 ms. 
//           Time does not shorten under 100 ms waiting on server
#define SLEEP_WAIT_ON_SERVER 100 

// Timeout by RFC 5321 = 5 minutes
#define SMTP_TIMEOUT (5 * 60 * CLOCKS_PER_SEC)

class SMTPConnection
{
public:
  //Constructors / Destructors
   SMTPConnection();
  ~SMTPConnection();

  //Methods
  bool    Connect(LPCTSTR p_hostName
                 ,XString p_mailid
                 ,XString p_password
                 ,bool    p_doLogin
                 ,int     p_port = 25);
  bool    Disconnect();
  bool    DoLogin(XString& p_mailID,XString& p_password);
  bool    TestMessageTooBig(SMTPMessage& Message);
  XString GetLastCommandResponse() const { return m_lastCommandResponse; };
  int     GetLastCommandResponseCode() const { return m_lastCommandResponseCode; };
  DWORD   GetTimeout() const { return m_timeout; };
  void    SetTimeout(DWORD dwTimeout) { m_timeout = dwTimeout; };
  BOOL    SendMessage(SMTPMessage& Message,RelayDlg* relay);
  void    ResetLastCommandResponse() { m_lastCommandResponse = _T(""); }

protected:
  bool    ExtendedEHLO(LPCTSTR p_hostname);
  BOOL    SimpleHelo(LPCTSTR pszHostName);
  XString ExtractWord(XString& p_line);
  bool    DoBasicLogin  (XString& p_mailID,XString& p_password);
  bool    DoNTLMLogin   (XString& p_mailID,XString& p_password);
  bool    DoXOAUTH2Login(XString& p_mailID,XString& p_password);
  char*   strrstr(char* total,char* part);
  BOOL    SendRCPTForRecipient(SMTPAddress& recipient
                              ,bool         do_notify 
                              ,NOTIFY_TYPE  notify
                              ,XString      type
                              ,RelayDlg*    relay);
  bool    SendAndResponse(XString buffer
                         ,int     expected
                         ,MESSAGE fail_mess
                         ,MESSAGE unex_mess
                         ,int     extra = 0);
  bool    SendAndResponse(BYTE*   buffer
                         ,int     expected
                         ,MESSAGE fail_mess
                         ,MESSAGE unex_mess
                         ,int     extra = 0);
  int     SendPartial(XString p_toSend);
  int     SendMsg(XString p_message);
  bool    ReadCommandResponse(int nExpectedCode,int nExtraCode = 0);
  bool    ReadResponse(LPSTR pszBuffer
                      ,int   nInitialBufSize
                      ,LPSTR pszTerminator);

  // NTLM Authentication
  bool            InitNTLMAuthentication();
  void            NTLMRelease();
  void            FreeSecurityLibrary();
  SECURITY_STATUS NTLMAuthenticate(LPCTSTR pszUserName, LPCTSTR pszPassword);
  SECURITY_STATUS DoNTLMAuthentication(LPCTSTR pszUserName, LPCTSTR pszPassword, LPCTSTR pszDomain);
  SECURITY_STATUS GenClientContext(BYTE* pIn, DWORD cbIn, BYTE* pOut, DWORD* pcbOut, BOOL* pfDone, LPCTSTR pszUserName, LPCTSTR pszPassword, LPCTSTR pszDomain);
  SECURITY_STATUS NTLMAuthPhase1(PBYTE pBuf, DWORD cbBuf);
  SECURITY_STATUS NTLMAuthPhase2(PBYTE pBuf, DWORD cbBuf, DWORD* pcbRead);
  SECURITY_STATUS NTLMAuthPhase3(PBYTE pBuf, DWORD cbBuf);

  // Upgrade to TLS connection with the SMTP Server
  bool    UpgradeToSecureConnection();

  EventWrapper        m_stopevent;
  SecureClientSocket* m_socket { nullptr };

  bool        m_connected { false };
  DWORD       m_timeout   { 20000 };
  bool        m_doLogin   { false };
  bool        m_doNTLM    { false };
  bool        m_doPLAIN   { false };
  bool        m_doXOAUTH  { false };
  XString     m_lastCommandResponse;
  CStringA    m_lastReceivedBuffer;
  int         m_lastCommandResponseCode { 0 };
  int         m_maxMessageSize          { 0 };

  // Status codes of the connection
  bool        m_8bitMime { false };
  bool        m_size     { false };
  bool        m_dsn      { false };
  bool        m_deliver  { false };
  bool        m_starttls { false };
  bool        m_utf8     { false };
  bool        m_errors   { false };

  // NTLM Authentication
  HINSTANCE                      m_secur32;                       // Instance handle of the "Secur32.dll" which houses the functions we want
  COMPLETE_AUTH_TOKEN_FN         m_lpfnCompleteAuthToken;         // The function pointer to "CompleteAuthToken"
  FREE_CREDENTIALS_HANDLE_FN     m_lpfnFreeCredentialsHandle;     // The function pointer to "FreeCredentialsHandle"
  DELETE_SECURITY_CONTEXT_FN     m_lpfnDeleteSecurityContext;     // The function pointer to "DeleteSecurityContext"
  INITIALIZE_SECURITY_CONTEXT_FN m_lpfnInitializeSecurityContext; // The function pointer to "InitializeSecurityContext"
  ACQUIRE_CREDENTIALS_HANDLE_FN  m_lpfnAcquireCredentialsHandle;  // The function pointer to "AcquireCredentialsHandle"
  CredHandle                     m_credentials;                   // The credentials handle for the connection
  SecHandle                      m_context;                       // The security handle for the connection 
  DWORD                          m_sspiBufferSize;                // The size of the buffers we allocate for reading and writing via SSPI
};

