/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPConnection.cpp
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
#include "stdafx.h"
#include "SMTPConnection.h"
#include "PostMail.h"
#include "Message.h"
#include "Base64.h"
#include "ConvertWideString.h"
#include "Certificate.h"
#include <atlenc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

SMTPConnection::SMTPConnection()
{
  m_connected = false;
  m_timeout   = SMTP_TIMEOUT;

  // NTLM Authentication
  m_secur32                       = nullptr;
  m_lpfnCompleteAuthToken         = nullptr;
  m_lpfnFreeCredentialsHandle     = nullptr;
  m_lpfnDeleteSecurityContext     = nullptr;
  m_lpfnInitializeSecurityContext = nullptr;
  m_lpfnAcquireCredentialsHandle  = nullptr;
  m_sspiBufferSize                = 0L;

  m_context.dwLower               = 0L;
  m_context.dwUpper               = 0L;
  m_credentials.dwLower           = 0L;
  m_credentials.dwUpper           = 0L;
}

SMTPConnection::~SMTPConnection()
{
  if(m_connected && !m_errors)
  {
    // Disconnect socket
    Disconnect();
  }

  // Free up the NTLM handles
  NTLMRelease();

  // Free the handle on the security library
  FreeSecurityLibrary();
}

bool
SMTPConnection::Connect(LPCTSTR p_hostName
                       ,XString p_mailid
                       ,XString p_password
                       ,bool    p_doLogin
                       ,int     p_port /*=25*/)
{
  // Reset login logic
  m_doLogin = p_doLogin;

  // parameter validity checking
  if(p_hostName == NULL || p_hostName[0] == 0)
  {
    // MESS_NOHOST "No SMTP mail host to send the mail from (HOST:)"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_NOHOST][g_lang]);
    return false;
  }

  // Create new socket
  m_socket = new SecureClientSocket(m_stopevent);

  // Settings for a client socket
  m_socket->SetConnTimeoutSeconds(15);        // General DNS timeout
  m_socket->SetRecvTimeoutSeconds(5 * 60);    // RFC 5321: Initial receive timeout is 5 minutes
  m_socket->SetSendTimeoutSeconds(5 * 60);    // RFC 5321: Server timeout is 5 minutes
  m_socket->SetUseKeepAlive(true);            // Default keep-alive : 2 hours
  // Set all three TLS service levels at once
  m_socket->SetSSLProtectionLevel((SSLProtClass) (TLS_10 | TLS_11 | TLS_12));
  // Connect our certificate methods
  m_socket->m_serverCertAcceptable    = CertAcceptable;
  m_socket->m_selectClientCertificate = SelectClientCertificate;

  // Now do the initialization
  m_socket->Initialize();

  // Log our connect 
  XString trying;
  trying.Format(_T("Try connecting to the mailserver [%s] through port [%d] ..."),p_hostName,p_port);
  theApp.Log(LOGLEVEL_TERSE,trying);

  // Connect to the SMTP Host
  if (!m_socket->Connect(p_hostName,(USHORT) p_port))
  {
    XString errMsg = theApp.WindowsError();
    XString mess;
    // MESS_NOCONNECT "Could not connect to the SMTP server %s on port %d"
    mess.Format(g_message[MESS_NOCONNECT][g_lang], p_hostName, p_port);
    theApp.Log(LOGLEVEL_ERROR,mess);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    return FALSE;
  }
  // We're now connected !!
  m_connected = true;
  theApp.Log(LOGLEVEL_TERSE,_T("Connected!"));

  // check the response to the connect
  if (!ReadCommandResponse(220))
  {
    XString errMsg = theApp.WindowsError();
    // MESS_UNEX_LOGON "Error at connecting to SMTP server: An unexpected SMTP connection response was received"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_LOGON][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    Disconnect();
    return false;
  }

  // Test connection with the EHLO command
  if(ExtendedEHLO(p_hostName))
  {
    // If STARTTLS is broadcasted and this is not the standard IANA port
    if(m_starttls && p_port != 25)
    {
      if(UpgradeToSecureConnection())
      {
        ExtendedEHLO(p_hostName);
      }
    }

    // Connection set. Try login
    if(p_doLogin)
    {
      return DoLogin(p_mailid,p_password);
    }
    // Try sending without login
    return true;
  }

  // Connection made, but no commands coming through
  return false;
}

bool
SMTPConnection::TestMessageTooBig(SMTPMessage& Message)
{
  // If no SIZE given by the EHLO, than we cannot calculate the size
  if(m_size == false)
  {
    return false;
  }

  // Add all the sizes
  int totalSize = 0;

  // Add all the attachment sizes
  for(int i = 0; i<Message.m_attachments.GetSize(); i++)
  {
    totalSize += Message.m_attachments.GetAt(i)->GetEncodedSize();
  }
  // Get all body texts
  totalSize += Message.GetBody().GetLength();
  totalSize += Message.GetBodyHTML().GetLength();
  // Space for all headers and message body dividers
  totalSize += (2 * 1024); // General headers

  // If total size bigger than allowed max message size
  // We cannot begin to send the message: The mailserver will never accept it!
  return (totalSize > m_maxMessageSize);
}

bool    
SMTPConnection::ExtendedEHLO(LPCTSTR p_hostname)
{
  XString buffer;
  buffer.Format(_T("EHLO %s\r\n"),p_hostname);
  theApp.Log(LOGLEVEL_CLIENT,buffer);
  if(SendPartial(buffer) == SOCKET_ERROR)
  {
    XString errMsg = theApp.WindowsError();
    // MESS_FAIL_EHLO "An unexpected error occurred while sending the EHLO (Extended-HALO) command"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_EHLO][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    Disconnect();
    return false;
  }
  bool readingEHLO = true;

  while(readingEHLO)
  {
    //check the response to the EHLO command
    if(!ReadCommandResponse(250))
    {
      // MESS_UNEX_EHLO "An unexpected EHLO (Extended HALO) response was received, trying simple HELO"
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_EHLO][g_lang]);
      if(!SimpleHelo(p_hostname))
      {
        Disconnect();
        return false;
      }
      // Sorry cannot get supported extensions
      return true;
    }
    if(m_lastCommandResponse.GetLength() < 5)
    {
      break;
    }
    // BREAK    criterion is the space (last line of many)
    // CONTINUE criterion is the hyphen '-'
    if(m_lastCommandResponse.GetAt(3) == _T(' '))
    {
      readingEHLO = false;
    }

    // Find all options, and record those we shall need/use
    XString option = ExtractWord(m_lastCommandResponse);
         if(option.CompareNoCase(_T("8BITMIME"))  == 0) m_8bitMime = true;
    else if(option.CompareNoCase(_T("DSN"))       == 0) m_dsn      = true;
    else if(option.CompareNoCase(_T("DELIVERBY")) == 0) m_deliver  = true;
    else if(option.CompareNoCase(_T("STARTTLS"))  == 0) m_starttls = true;
    else if(option.CompareNoCase(_T("SMTPUTF8"))  == 0) m_utf8     = true;
    else if(option.CompareNoCase(_T("SIZE")) == 0)
    {
      m_size = true;
      m_maxMessageSize = _ttoi(m_lastCommandResponse.Trim());
    }
    else if(option.CompareNoCase(_T("AUTH"))      == 0)
    {
      // See if we must
      if(m_lastCommandResponse.Find(_T("LOGIN")) >= 0)
      {
        m_doLogin = true;

        // If just "AUTH LOGIN" then do a plain login
        if(m_lastCommandResponse.GetLength() == 5)
        {
          m_doPLAIN = true;
        }
      }
      // Find a supported login type
      if(m_lastCommandResponse.Find(_T("NTLM")) >= 0)
      {
        m_doNTLM = true;
      }
      else if(m_lastCommandResponse.Find(_T("PLAIN")) >= 0)
      {
        m_doPLAIN = true;
      }
      else if(m_lastCommandResponse.Find(_T("XOAUTH2")) >= 0)
      {
        m_doXOAUTH = true;
      }
      // Check: can we do a login
      if(m_doLogin && (m_lastCommandResponse.GetLength() > 5) && 
        (m_doNTLM == false && m_doPLAIN == false && m_doXOAUTH == false))
      {
        // Oops: must login but not a supported login type
        theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_AUTHTYPE][g_lang]);
        theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
        return false;
      }
    }
    else
    {
      // Unrecognized !! NOT AN ERROR. IGNORE THE OPTION
    }
  }
  return true;
}

// Fall back on a simple 'helo', without options
// Letting the server know we mean business.
BOOL 
SMTPConnection::SimpleHelo(LPCTSTR p_hostname)
{
  //Send the HELO command
  XString buffer;
  buffer.Format(_T("HELO %s\r\n"), p_hostname);
  // An unexpected error while sending the "HELO" command
  if(!SendAndResponse(buffer,250,MESS_FAIL_HELO,MESS_UNEX_HELO))
  {
    return FALSE;
  }
  return TRUE;
}

XString 
SMTPConnection::ExtractWord(XString& p_line)
{
  XString word;

  // Trim CR-LF
  p_line = p_line.TrimRight(_T("\r\n"));

  // Drop command code and ' ' or '-'
  if(_istdigit((TCHAR)p_line.GetAt(0)) && _istdigit((TCHAR)p_line.GetAt(1)) && _istdigit((TCHAR)p_line.GetAt(2)))
  {
    p_line = p_line.Mid(4);
  }

  // Find end of the first word
  int pos = p_line.Find(_T(' '));
  
  // Split first word
  if(pos > 0 && pos < p_line.GetLength())
  {
    word   = p_line.Left(pos);
    p_line = p_line.Mid(pos + 1);
    return word;
  }
  else
  {
    // Line contains one (1) word only
    word = p_line;
    p_line.Empty();
  }
  return word;
}

// Upgrade to TLS connection with the SMTP Server
bool    
SMTPConnection::UpgradeToSecureConnection()
{
  // RFC 3207 states that "STARTTLS" must answer with a '220' status for a go-ahead
  // Other legal answers are:
  // 501: Syntax error
  // 454: TLS (temporary) not available

  // Send the STARTTLS command
  XString command(_T("STARTTLS\r\n"));

  // MESS_FAIL_TLS STARTTLS commando could not be sent to the server, secure mode not initialized
  // MESS_UNEX_TLS Unexpected answer received at the STARTTLS command, no way to start secure mode
  if(!SendAndResponse(command,220,MESS_FAIL_TLS,MESS_UNEX_TLS))
  {
    if(m_lastCommandResponseCode == 454)
    {
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_TEMP_NOTLS][g_lang]);
    }
    return false;
  }

  // Try to initialize a TLS connection
  HRESULT hr = m_socket->InitializeSSL();
  if(SUCCEEDED(hr))
  {
    theApp.Log(LOGLEVEL_TERSE,_T("Client switched to secure TLS mode"));
  }
  else
  {
    theApp.Log(LOGLEVEL_TERSE,_T("SSL/TLS client mode initialization failed"));
  }
  return SUCCEEDED(hr);
}

bool
SMTPConnection::Disconnect()
{
  BOOL bSuccess = FALSE;      

  // disconnect from the SMTP server if connected 
  if (m_connected)
  {
    XString sBuf(_T("QUIT\r\n"));
    // Sending the QUIT command went wrong
    // Error while logging out from the SMTP server: Received unexpected response on the QUIT command

    // There are potentially two different answers
    // Code 221: Quit connection
    // Code 250: Message queued for delivery
    if(SendAndResponse(sBuf,221,MESS_FAIL_QUIT,MESS_UNEX_QUIT,250))
    {
      bSuccess = TRUE;
    }
	  //Reset all the state variables after SendAndResponse
    m_connected = false;
  }
  else
  {
    XString errMsg = theApp.WindowsError();
    // MESS_QUIT_ALREADY "Already disconnected from SMTP server, doing nothing"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_QUIT_ALREADY][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
  } 

  // Break the listening side
  ::SetEvent(m_stopevent);
  // free up our socket
  m_socket->Close();

  // Destroy the socket
  delete m_socket;
  m_socket = nullptr;
 
  return bSuccess;
}

bool
SMTPConnection::DoLogin(XString& p_mailID,XString& p_password)
{
  if(m_doPLAIN)
  {
    // As a fall-back: use basic login
    return DoBasicLogin(p_mailID,p_password);
  }
  // SMTP Server knows about NTLM, So try it
  else if(m_doNTLM)
  {
    return DoNTLMLogin(p_mailID,p_password);
  }
  // SMTP Server knows about XOAUTH2 method, so try it
  else if(m_doXOAUTH)
  {
    return DoXOAUTH2Login(p_mailID,p_password);
  }
  // No authentication method present on the SMTP server
  // Try sending right away! Proceed with fingers crossed!!
  // BUT MOST LIKELY TO FAIL!!!
  return true;
}

bool
SMTPConnection::DoBasicLogin(XString& p_mailID,XString& p_password)
{
  // No user ID en password present. Do not use login
  if(!p_mailID.GetLength() || !p_password.GetLength())
  {
    // No username and password given. Try to send the email without a login
    theApp.Log(LOGLEVEL_TERSE,g_message[MESS_NO_LOGIN][g_lang]);
    return true;
  }
  // Request login logic
  XString sBuf(_T("AUTH LOGIN\r\n"));
  if(!SendAndResponse(sBuf,334,MESS_FAIL_AUTH,MESS_UNEX_AUTH))
  {
    return false;
  }
  if(m_lastCommandResponse.Mid(4) != _T("VXNlcm5hbWU6"))
  {
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_AUTH][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return false;
  }
  //<Send m_mailid in Base64 form>
#ifdef _UNICODE
  CStringA mailID(p_mailID);
#else
  CString mailID(p_mailID);
#endif


  int idSize = Base64::Base64BufferSize(p_mailID.GetLength()+1);
  BYTE* mailid_encoded = new BYTE[(unsigned)idSize+3];
  Base64::EncodeBase64((BYTE*)mailID.GetString(),p_mailID.GetLength(),mailid_encoded,idSize,&idSize);

  if(!SendAndResponse(mailid_encoded,334,MESS_FAIL_MAILID,MESS_UNEX_MAILID))
  {
    delete [] mailid_encoded;
    return false;
  }
  delete [] mailid_encoded;

  if(m_lastCommandResponse.Mid(4) == _T("VXNlcm5hbWU6"))
  {
    // No MAIL-ID or unacceptable mail-id for the server,
    // Still asking for a mail id
    // MESS_NO_MAILID "No mail-id at login time given or unacceptable mail-id for the server"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_NO_MAILID][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return false;
  }
  if(m_lastCommandResponse.Mid(4) != _T("UGFzc3dvcmQ6"))
  {
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_MAILID][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return false;
  }
  // <Send m_password in Base64 form>
#ifdef _UNICODE
  CStringA password(p_password);
#else
  CString password(p_password);
#endif

  idSize = Base64::Base64BufferSize(p_password.GetLength()+1);
  BYTE* passwd_encoded = new BYTE[(unsigned)idSize+3];
  Base64::EncodeBase64((const BYTE*)password.GetString(),password.GetLength(),passwd_encoded,idSize,&idSize);

  if(!SendAndResponse(passwd_encoded,235,MESS_FAIL_PASSWORD,MESS_UNEX_PASSWORD))
  {
    delete [] passwd_encoded;
    return false;
  }
  delete [] passwd_encoded;
  
  return true;
}

bool    
SMTPConnection::DoXOAUTH2Login(XString& p_mailID,XString& p_password)
{
  // No user ID en password present. Do not use login
  if(!p_mailID.GetLength() || !p_password.GetLength())
  {
    // No username and password given. Try to send the email without a login
    theApp.Log(LOGLEVEL_TERSE,g_message[MESS_NO_LOGIN][g_lang]);
    return true;
  }
  int userNameLen    = p_mailID.GetLength();
  int accessTokenLen = p_password.GetLength();
  int totalLen = userNameLen + accessTokenLen + 20;

  CStringA mailID(p_mailID);
  CStringA password(p_password);

  // Token buffer "user=<username><CTRL-A>auth=Bearer <password><CTRL-A><CTRL-A>"
  // See: https://developers.google.com/gmail/imap/xoauth2-protocol
  char* token = new char[totalLen + 1];

  // Build the O-AUTH Version 2 token
  memcpy_s(token,totalLen,"user=",5);
  memcpy_s(token + 5,totalLen - 5,mailID.GetString(),userNameLen);
  token[5 + userNameLen] = 0x01; // Control-A
  memcpy_s(token +  6 + userNameLen,totalLen -  6 - userNameLen,"auth=Bearer ",12);
  memcpy_s(token + 18 + userNameLen,totalLen - 18 - userNameLen,password.GetString(),accessTokenLen);
  token[18 + userNameLen + accessTokenLen] = 0x01; // Control-A
  token[19 + userNameLen + accessTokenLen] = 0x01; // Control-A
  token[totalLen] = 0;

  // Build SMTP authentication line
  SMPTBase64Encode encode;
  encode.Encode(token,ATL_BASE64_FLAG_NOCRLF);
  CStringA tosend = CStringA("AUTH XOAUTH2 ") + CStringA(encode.Result()) + "\r\n";

  // Go send it to the server
  bool result = SendAndResponse((BYTE*)tosend.GetString(),235,MESS_FAIL_PASSWORD,MESS_UNEX_PASSWORD);
  if(result == false)
  {
    // Explicitly answer with an empty line: see xoauth2 documentation!!
    SendAndResponse((BYTE*)"\r\n",-1,MESS_FAIL_PASSWORD,MESS_UNEX_PASSWORD);
  }
  delete[] token;

  return result;
}

bool
SMTPConnection::DoNTLMLogin(XString& p_mailID,XString& p_password)
{
  if(p_mailID.IsEmpty() || p_password.IsEmpty())
  {
    // Try single-signon with no username/password
    theApp.Log(LOGLEVEL_TERSE,g_message[MESS_SINGLE_SIGNON][g_lang]);
    p_mailID.Empty();
    p_password.Empty();
  }
  if(!InitNTLMAuthentication())
  {
    return false;
  }
  SECURITY_STATUS ss = NTLMAuthenticate(p_mailID,p_password);
  if(SEC_SUCCESS(ss))
  {
    return true;
  }
  return false;
}

bool
SMTPConnection::InitNTLMAuthentication()
{
  bool reset = false;
  //Set our credentials handles to default values
  memset(&m_credentials, 0, sizeof(m_credentials));
  memset(&m_context, 0, sizeof(m_context));
  m_sspiBufferSize = 16384;

  m_secur32 = LoadLibrary(_T("secur32.dll"));
  if (m_secur32)
  {
    m_lpfnCompleteAuthToken         = reinterpret_cast<COMPLETE_AUTH_TOKEN_FN>        (GetProcAddress(m_secur32,"CompleteAuthToken"));
    m_lpfnFreeCredentialsHandle     = reinterpret_cast<FREE_CREDENTIALS_HANDLE_FN>    (GetProcAddress(m_secur32,"FreeCredentialsHandle"));
    m_lpfnDeleteSecurityContext     = reinterpret_cast<DELETE_SECURITY_CONTEXT_FN>    (GetProcAddress(m_secur32,"DeleteSecurityContext"));
    m_lpfnInitializeSecurityContext = reinterpret_cast<INITIALIZE_SECURITY_CONTEXT_FN>(GetProcAddress(m_secur32,"InitializeSecurityContextA"));
    m_lpfnAcquireCredentialsHandle  = reinterpret_cast<ACQUIRE_CREDENTIALS_HANDLE_FN> (GetProcAddress(m_secur32,"AcquireCredentialsHandleA"));

    // Note we allow "CompleteAuthToken" to be not implemented. 
    // This gives us at least a runtime chance of using NTLM authentication on Win 9x
    if (m_lpfnFreeCredentialsHandle     == nullptr || 
        m_lpfnDeleteSecurityContext     == nullptr || 
        m_lpfnInitializeSecurityContext == nullptr || 
        m_lpfnAcquireCredentialsHandle  == nullptr )
    {
      reset = true;
    } 
  }
  else
  {
    reset = true;
  }

  // Reset everything
  if(reset)
  {
    m_lpfnCompleteAuthToken         = nullptr;
    m_lpfnFreeCredentialsHandle     = nullptr;
    m_lpfnDeleteSecurityContext     = nullptr;
    m_lpfnInitializeSecurityContext = nullptr;
    m_lpfnAcquireCredentialsHandle  = nullptr;
  }
  return !reset;
}

void
SMTPConnection::FreeSecurityLibrary()
{
  // Free the handle on the security library
  if(m_secur32)
  {
    FreeLibrary(m_secur32);
    m_secur32 = nullptr;
  }
}

void
SMTPConnection::NTLMRelease()
{
  // Free up the security context if valid
  if (m_context.dwLower != 0 || m_context.dwUpper != 0) 
  {
    ASSERT(m_lpfnDeleteSecurityContext);
    m_lpfnDeleteSecurityContext(&m_context);
    memset(&m_context, 0, sizeof(m_context));
  }

  // Free up the credentials handle if valid
  if (m_credentials.dwLower != 0 || m_credentials.dwUpper != 0) 
  {
    ASSERT(m_lpfnFreeCredentialsHandle);
    m_lpfnFreeCredentialsHandle(&m_credentials);
    memset(&m_credentials, 0, sizeof(m_credentials));
  }

}

SECURITY_STATUS SMTPConnection::NTLMAuthenticate(LPCTSTR pszUserName, LPCTSTR pszPassword)
{
  //Note we do the check for these 2 function pointers at the start, as otherwise we could (very unlikely
  //but possible) end up with resource handles but with no means to deallocate them!
  if ((m_lpfnDeleteSecurityContext == NULL) || (m_lpfnFreeCredentialsHandle == NULL))
  {
    return SEC_E_UNSUPPORTED_FUNCTION;
  }
  //allow "UserName" to be of the format DomainName\UserName
  LPCTSTR pszDomain = _T("");
  XString sUserName(pszUserName);
  int nSlashSeparatorOffset = sUserName.Find(_T('\\'));
  XString sDomain;
  XString sUserNameWithoutDomain;
  if (nSlashSeparatorOffset != -1)
  {
    sDomain = sUserName.Left(nSlashSeparatorOffset);
    pszDomain = sDomain.operator LPCTSTR();
    sUserNameWithoutDomain = sUserName.Mid(nSlashSeparatorOffset + 1);
    pszUserName = sUserNameWithoutDomain.operator LPCTSTR();
  }

  //Release the handles before we try to authenticate (we do this here to ensure any previous calls
  //to NTLMAuthenticate which throw exceptions are cleaned up prior to any new calls to DoNTLMAuthentication)
  NTLMRelease();

  //Call the helper function which does all the work
  SECURITY_STATUS ss = DoNTLMAuthentication(pszUserName, pszPassword, pszDomain);

  //Now free up the handles now that we are finished the authentication (note it is not critical that this code is
  //called since the various NTLMAuthPhase(*) functions may throw exceptions
  NTLMRelease();

  return ss;
}

SECURITY_STATUS SMTPConnection::DoNTLMAuthentication(LPCTSTR p_userName, LPCTSTR p_password, LPCTSTR p_domain)
{
  //Allocate some heap space to contain the in and out buffers for SSPI
  ATL::CHeapPtr<BYTE> inBuf;
  if (!inBuf.Allocate(m_sspiBufferSize))
  {
    return SEC_E_INSUFFICIENT_MEMORY;
  }
  ATL::CHeapPtr<BYTE> outBuf;
  if (!outBuf.Allocate(m_sspiBufferSize))
  {
    return SEC_E_INSUFFICIENT_MEMORY;
  }
  DWORD cbMaxMessage = m_sspiBufferSize;
  DWORD cbOut = cbMaxMessage;
  BOOL bDone = FALSE;
  SECURITY_STATUS ss = GenClientContext(NULL, 0, outBuf.m_pData, &cbOut, &bDone, p_userName, p_password, p_domain);
  if (!SEC_SUCCESS(ss))
  {
    return ss;
  }
  ss = NTLMAuthPhase1(outBuf.m_pData, cbOut);
  if (!SEC_SUCCESS(ss))
  {
    return ss;
  }
  DWORD cbIn = 0;
  while (!bDone) 
  {
    ss = NTLMAuthPhase2(inBuf.m_pData, m_sspiBufferSize, &cbIn);
    if (!SEC_SUCCESS(ss))
    {
      return ss;
    }
    cbOut = cbMaxMessage;

    ss = GenClientContext(inBuf.m_pData, cbIn, outBuf.m_pData, &cbOut, &bDone, p_userName, p_password, p_domain);
    if (!SEC_SUCCESS(ss))
    {
      return ss;
    }
    ss = NTLMAuthPhase3(outBuf.m_pData, cbOut);
    if (!SEC_SUCCESS(ss))
    {
      return ss;
    }
  }
  return ss;
}

SECURITY_STATUS 
SMTPConnection::GenClientContext(BYTE*   pIn
                                ,DWORD   cbIn
                                ,BYTE*   pOut
                                ,DWORD*  pcbOut
                                ,BOOL*   pfDone
                                ,LPCTSTR pszUserName
                                ,LPCTSTR pszPassword
                                ,LPCTSTR pszDomain)
{
  TimeStamp Lifetime;
  SECURITY_STATUS ss;
  if (NULL == pIn)  
  {   
    if (m_lpfnAcquireCredentialsHandle)
    {
      SEC_WINNT_AUTH_IDENTITY authInfo;
      authInfo.Domain   = NULL;
      authInfo.User     = NULL;
      authInfo.Password = NULL;
      void* pvLogonID   = NULL;

      if ((pszUserName != NULL) && (lstrlen(pszUserName)))
      {
        authInfo.UserLength     = lstrlen(pszUserName);
        authInfo.DomainLength   = lstrlen(pszDomain);
        authInfo.PasswordLength = lstrlen(pszPassword);
#ifdef _UNICODE
        authInfo.User     = reinterpret_cast<unsigned short*>(const_cast<LPTSTR>(pszUserName));
        authInfo.Domain   = reinterpret_cast<unsigned short*>(const_cast<LPTSTR>(pszDomain));
        authInfo.Password = reinterpret_cast<unsigned short*>(const_cast<LPTSTR>(pszPassword));
#else
        authInfo.User     = reinterpret_cast<unsigned char*>(const_cast<LPTSTR>(pszUserName));
        authInfo.Domain   = reinterpret_cast<unsigned char*>(const_cast<LPTSTR>(pszDomain));
        authInfo.Password = reinterpret_cast<unsigned char*>(const_cast<LPTSTR>(pszPassword));
#endif
        authInfo.Flags    = SEC_WINNT_AUTH_IDENTITY_ANSI;
        pvLogonID         = &authInfo;
      }
      ss = m_lpfnAcquireCredentialsHandle(NULL, _T("NTLM"), SECPKG_CRED_OUTBOUND, NULL, pvLogonID, NULL, NULL, &m_credentials, &Lifetime);
      if (!SEC_SUCCESS(ss))
      {
        return ss;
      }
    }
    else
    {
      return SEC_E_UNSUPPORTED_FUNCTION;
    }
  }
  //Prepare the buffers
  SecBufferDesc OutBuffDesc;
  OutBuffDesc.ulVersion = 0;
  OutBuffDesc.cBuffers  = 1;
  SecBuffer OutSecBuff;
  OutBuffDesc.pBuffers  = &OutSecBuff;
  OutSecBuff.cbBuffer   = *pcbOut;
  OutSecBuff.BufferType = SECBUFFER_TOKEN;
  OutSecBuff.pvBuffer   = pOut;

  //The input buffer is created only if a message has been received from the server.
  SecBufferDesc InBuffDesc;
  SecBuffer InSecBuff;
  if (pIn)   
  {
    InBuffDesc.ulVersion = 0;
    InBuffDesc.cBuffers  = 1;
    InBuffDesc.pBuffers  = &InSecBuff;
    InSecBuff.cbBuffer   = cbIn;
    InSecBuff.BufferType = SECBUFFER_TOKEN;
    InSecBuff.pvBuffer   = pIn;

    ULONG ContextAttributes;
    if (m_lpfnInitializeSecurityContext)
    {
      ss = m_lpfnInitializeSecurityContext(&m_credentials, &m_context, NULL, 0, 0, SECURITY_NATIVE_DREP, &InBuffDesc, 0, &m_context, &OutBuffDesc, &ContextAttributes, &Lifetime);
    }
    else
    {
      return SEC_E_UNSUPPORTED_FUNCTION;
    }
  }
  else
  {
    ULONG ContextAttributes;
    if (m_lpfnInitializeSecurityContext)
    {
      ss = m_lpfnInitializeSecurityContext(&m_credentials, NULL, NULL, 0, 0, SECURITY_NATIVE_DREP, NULL, 0, &m_context, &OutBuffDesc, &ContextAttributes, &Lifetime);
    }
    else
    {
      return SEC_E_UNSUPPORTED_FUNCTION;
    }
  }

  if (!SEC_SUCCESS(ss))
  {
    return ss;
  }
  //If necessary, complete the token.
  if ((SEC_I_COMPLETE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss))  
  {
    //Check if CompleteAuthToken is available at runtime
    if (m_lpfnCompleteAuthToken)
    {
      ss = m_lpfnCompleteAuthToken(&m_context, &OutBuffDesc);
      if (!SEC_SUCCESS(ss))  
      {
        return ss;
      }
    }
    else
    {
      return SEC_E_UNSUPPORTED_FUNCTION;
    }
  }
  *pcbOut = OutSecBuff.cbBuffer;
  *pfDone = !((SEC_I_CONTINUE_NEEDED == ss) || (SEC_I_COMPLETE_AND_CONTINUE == ss));

  return ss;
}

SECURITY_STATUS 
SMTPConnection::NTLMAuthPhase1(PBYTE p_buffer, DWORD p_length)
{
  //Send the AUTH NTLM command with the initial data
  SMPTBase64Encode encode;
  encode.Encode(p_buffer,p_length,ATL_BASE64_FLAG_NOCRLF);

  XString buffer;
  buffer.Format(_T("AUTH NTLM %s\r\n"),encode.Result());
  
  theApp.Log(LOGLEVEL_CLIENT,buffer);
  if(SendPartial(buffer) == SOCKET_ERROR)
  {
    // We did not anticipate this on the success path
    // Tell we failed and give the last response from server
    SetLastError(ERROR_BAD_COMMAND);
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_LOGON][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return SEC_E_UNSUPPORTED_FUNCTION;
  }
  return SEC_E_OK;
}

SECURITY_STATUS 
SMTPConnection::NTLMAuthPhase2(PBYTE pBuf, DWORD cbBuf, DWORD* pcbRead)
{
  //check the response to the AUTH NTLM command
  if (!ReadCommandResponse(334))
  {
    // We did not anticipate this on the success path
    // Tell we failed and give the last response from server
    SetLastError(ERROR_BAD_COMMAND);
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_LOGON][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return SEC_E_UNSUPPORTED_FUNCTION;
  }
  //Decode the last response
  SMPTBase64Encode encode;
  XString sLastCommandString(m_lastCommandResponse);
  sLastCommandString = sLastCommandString.Right(sLastCommandString.GetLength() - 4);
  CStringA sAsciiLastCommandString(sLastCommandString);
  encode.Decode(sAsciiLastCommandString);

  //Store the results in the output parameters
  *pcbRead = encode.ResultSize();
  if (*pcbRead >= cbBuf)
  {
    return SEC_E_INSUFFICIENT_MEMORY;
  }
  memcpy_s(pBuf, cbBuf, encode.Result(), *pcbRead);

  return SEC_E_OK;
}

SECURITY_STATUS 
SMTPConnection::NTLMAuthPhase3(PBYTE p_buffer, DWORD p_length)
{
  //send base64 encoded version of the data
  SMPTBase64Encode encode;
  encode.Encode(p_buffer, p_length, ATL_BASE64_FLAG_NOCRLF);

  XString buffer;
  buffer.Format(_T("%s\r\n"),encode.Result());

  theApp.Log(LOGLEVEL_CLIENT,buffer);
  if(SendPartial(buffer) == SOCKET_ERROR)
  {
    SetLastError(ERROR_BAD_COMMAND);
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_UNEX_LOGON][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return SEC_E_UNSUPPORTED_FUNCTION;
  }

  // check if authentication is successful
  if (!ReadCommandResponse(235))
  {
    // NOT AUTHORISED!!
    SetLastError(ERROR_ACCESS_DENIED);
    theApp.WideMessageBox(NULL,g_message[MESS_NO_AUTHOR][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_NO_AUTHOR][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return SEC_E_INVALID_TOKEN;
  }
  return SEC_E_OK;
}

BOOL 
SMTPConnection::SendMessage(SMTPMessage& Message,RelayDlg* relay)
{
  // parameter validity checking
  // Must be connected to send a message
  if(!m_connected)
  {
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_INTERN_SOCK][g_lang]);
  }

  if(!Message.m_from.m_emailAddress.GetLength())
  {
    //MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_NOADDRESS][g_lang]);
  }

  XString sBuf;
  if(relay)
  {
    //RELAY
    sBuf.Format(_T("Mail from: %s"),Message.m_from.m_emailAddress.GetString());
    relay->Increment();
    relay->SetText(sBuf);
  }

  //Send the MAIL command
  if(m_dsn && Message.GetNotify() != NOTIFY_NEVER)
  {
    // OK, we are capable of DSN handling and we want it.
    XString retFormat;
    if(Message.GetNotify() & NOTIFY_FULL)
    {
      retFormat = _T("FULL");
    }
    if(Message.GetNotify() & NOTIFY_HEADER)
    {
      retFormat = _T("HDRS");
    }
    sBuf.Format(_T("MAIL FROM:<%s>"),Message.m_from.m_emailAddress.GetString());

    if(!retFormat.IsEmpty())
    {
      // RCPT wil specify DSN's, so specify return format
      sBuf.AppendFormat(_T(" RET=%s"),retFormat.GetString());
    }
    sBuf += _T("\r\n");
  }
  else
  {
    // Simple: NO DSN's, so don't specify return headers/full
    sBuf.Format(_T("MAIL FROM:<%s>\r\n"),Message.m_from.m_emailAddress.GetString());
  }

  // MESS_FAIL_MAIL "Failed in call to send 'MAIL FROM' command"
  // MESS_UNEX_MAIL "Error in mail-senders address: an unexpected MAIL response was received"
  if(!SendAndResponse(sBuf,250,MESS_FAIL_MAIL,MESS_UNEX_MAIL))
  {
    return FALSE;
  }

  // Send the RCPT command, one for each recipient (includes the TO, CC & BCC recipients)
  // Must be sending to someone
  if((Message.GetNumberOfRecipients(TO) + 
      Message.GetNumberOfRecipients(CC) + 
      Message.GetNumberOfRecipients(BCC)) == 0)
  {
    // Not a minimum of 1 sender address given (TO:)
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_NORCPT][g_lang]);
    return FALSE;
  }

  //First the "To" recipients
  for (int i=0; i<Message.GetNumberOfRecipients(TO); i++)
  {
    SMTPAddress recipient = Message.GetRecipient(i, TO);
    if (!SendRCPTForRecipient(recipient,m_dsn,Message.GetNotify(),_T("TO"),relay))
    {
      return FALSE;
    }
  }
  //Then the "CC" recipients
  for (int i=0; i<Message.GetNumberOfRecipients(CC); i++)
  {
    SMTPAddress recipient = Message.GetRecipient(i, CC);
    if (!SendRCPTForRecipient(recipient,m_dsn,Message.GetNotify(),_T("CC"),relay))
    {
      return FALSE;
    }
  }
  //Then the "BCC" recipients
  for (int i=0; i<Message.GetNumberOfRecipients(BCC); i++)
  {
    SMTPAddress recipient = Message.GetRecipient(i, BCC);
    if (!SendRCPTForRecipient(recipient,m_dsn,Message.GetNotify(),_T("BCC"),relay))
    {
      return FALSE;
    }
  }
  if(relay)
  {
    // RELAY
    relay->Increment();
    relay->SetText(_T("Sending message"));
  }

  //Send the DATA command
  XString pszDataCommand(_T("DATA\r\n"));

  // RFC 5321: Lower timeout from 5 to 2 minutes for the DATA command
  // m_socket->SetRecvTimeoutSeconds(2 * 60);    

  //MESS_FAIL_DATA "Failed in call to send MAIL command"
  //MESS_UNEX_DATA "Error in the body of the mail: an unexpected DATA response was received"
  if(!SendAndResponse(pszDataCommand,354,MESS_FAIL_DATA,MESS_UNEX_DATA))
  {
    return FALSE;
  }

  // RFC 5321: Set timeout to 3 minutes for each data part
  // m_socket->SetRecvTimeoutSeconds(3 * 60);

  // See if we have an RTF or HTML body 
  bool doRTF = false;
  bool doHTML = false;
  XString sContentType;

  if(Message.m_body.Left(6) == _T("{\\rtf1"))
  {
    doRTF = true;
    sContentType = _T("application/ms-tnef");
  }
  if(Message.m_body.Left(6).CompareNoCase(_T("<html>")) == 0 || !Message.m_htmlBody.IsEmpty())
  {
    doHTML = true;
    sContentType = _T("text/html");
  }

  //Send the Header
  XString messHeader = Message.GetHeader(sContentType,doRTF);
  theApp.Log(LOGLEVEL_CLIENT,messHeader);
  if(SendPartial(messHeader) == SOCKET_ERROR)
  {
    XString errMsg = theApp.WindowsError();
    // MESS_FAIL_HEADER "Failed in call to send the header"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_HEADER][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    return FALSE;
  }

  //Send the Mime Header for the body
  if(Message.m_attachments.GetSize() || doRTF)
  {
    XString bodyHeader;
    if(doRTF)
    {
      bodyHeader = _T("\r\n--#BOUNDARY#\r\n")
                   _T("Content-Type: application/rtf; name=\"Message.rtf\"\r\n")
                   _T("Content-Transfer-Encoding: binary\r\n\r\n");
    }
    else if(doHTML)
    {
      bodyHeader = _T("\r\n--#BOUNDARY#\r\n")
                   _T("Content-Type: text/html; charset=us-ascii\r\n")
                   _T("Content-Transfer-Encoding: 8bit\r\n\r\n");
    }
    else if(m_utf8)
    {
      bodyHeader = _T("\r\n--#BOUNDARY#\r\n")
                   _T("Content-Type: text/plain; charset=utf-8\r\n")
                   _T("Content-Transfer-Encoding: 8bit\r\n\r\n");
    }
    else
    {
      bodyHeader = _T("\r\n--#BOUNDARY#\r\n")
                   _T("Content-Type: text/plain; charset=us-ascii\r\n")
                   _T("Content-Transfer-Encoding: 8bit\r\n\r\n");
    }
    theApp.Log(LOGLEVEL_CLIENT,bodyHeader);
    if(SendPartial(bodyHeader) == SOCKET_ERROR)
    {
      XString errMsg = theApp.WindowsError();
      //MESS_FAIL_BODYHEADER "Failed in call to send the body header"
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_BODYHEADER][g_lang]);
      theApp.Log(LOGLEVEL_ERROR,errMsg);
      return FALSE;
    }
  }

  //Send the body (HTML of PLAIN text)
  XString messBody;

  if(Message.m_htmlBody.IsEmpty())
  {
    if(m_utf8)
    {
      theApp.Log(LOGLEVEL_CLIENT,Message.m_body);
      messBody = Message.m_body;
    }
    else
    {
      messBody = Message.m_body;
      theApp.Log(LOGLEVEL_CLIENT,Message.m_body);
    }
  }
  else
  {
    messBody = Message.m_htmlBody;
    theApp.Log(LOGLEVEL_CLIENT,messBody);
  }

  if(SendMsg(messBody) == SOCKET_ERROR)
  {
    XString errMsg = theApp.WindowsError();
    //MESS_FAIL_BODY "Failed in call to send the body of the message"
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_BODY][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    return FALSE;
  }

  //Send all the attachments
  for (int i=0; i<Message.m_attachments.GetSize(); i++)
  {
    SMTPAttachment* pAttachment = Message.m_attachments.GetAt(i);

    if(relay)
    {
      // RELAY
      sBuf.Format(_T("Attachment: %s"),pAttachment->GetFilename().GetString());
      relay->Increment();
      relay->SetText(sBuf);
    }
    //First send the Mime header for each attachment
    XString contentHeader;
    contentHeader.Format(_T("\r\n\r\n--#BOUNDARY#\r\n")
                    _T("Content-Type: application/octet-stream; name=%s\r\n")
                    _T("Content-Transfer-Encoding: base64\r\n")
                    _T("Content-Disposition: attachment; filename=%s\r\n\r\n")
                   ,pAttachment->GetTitle().GetString()
                   ,pAttachment->GetTitle().GetString());

    theApp.Log(LOGLEVEL_CLIENT,contentHeader);
    if (SendPartial(contentHeader) == SOCKET_ERROR)
    {
      XString errMsg = theApp.WindowsError();
      //MESS_FAIL_MIMEHEADER "Failed in call to send Mime attachment header"
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_MIMEHEADER][g_lang]);
      theApp.Log(LOGLEVEL_ERROR,errMsg);
      return FALSE;
    }

    // Then send the encoded attachment
    XString msg;
    msg.Format(_T("Sending attachment of [%d] bytes"),pAttachment->GetEncodedSize());
    theApp.Log(LOGLEVEL_CLIENT,msg);

    if (m_socket->SendMsg(pAttachment->GetEncodedBuffer(), pAttachment->GetEncodedSize()) == SOCKET_ERROR)
    {
      XString errMsg = theApp.WindowsError();
      //MESS_FAIL_ATTACH "Failed in call to send the attachment"
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_ATTACH][g_lang]);
      theApp.Log(LOGLEVEL_ERROR,errMsg);
      return FALSE;
    }
  }

  //Send the final mime boundary
  if(Message.m_attachments.GetSize() || doRTF)
  {
    XString finalBoundary = _T("\r\n--#BOUNDARY#--");
    theApp.Log(LOGLEVEL_CLIENT,finalBoundary);
    if(SendPartial(finalBoundary) == SOCKET_ERROR)
    {
      XString errMsg = theApp.WindowsError();
      //MESS_FAIL_MIMEFOOTER "Failed in call to send MIME attachment footer"
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_FAIL_MIMEFOOTER][g_lang]);
      theApp.Log(LOGLEVEL_ERROR,errMsg);
      return FALSE;
    }
  }

  //Send the end of message indicator
  XString pszEOM(_T("\r\n.\r\n"));

  // RFC 5321: Set timeout to 10 minutes for last data part, to complete the storage of the email
  m_socket->SetRecvTimeoutSeconds(10 * 60);

  // MESS_FAIL_EOM "Failed in call to send end-of-message indicator"
  // MESS_UNEX_EOM "Error at the end of the mail body: An unexpected end of message response was received"
  if(!SendAndResponse(pszEOM,250,MESS_FAIL_EOM,MESS_UNEX_EOM))
  {
    return FALSE;
  }

  // RFC 5321: Reset the timeout to the normative 5 minutes
  m_socket->SetRecvTimeoutSeconds(5 * 60);

  return TRUE;
}

BOOL 
SMTPConnection::SendRCPTForRecipient(SMTPAddress& recipient
                                    ,bool         do_notify
                                    ,NOTIFY_TYPE  notify
                                    ,XString      type
                                    ,RelayDlg*    relay)
{
  // must have an email address for this recipient
  if(!recipient.m_emailAddress.GetLength())
  {
    //MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_NOADDRESS][g_lang]);
  }
  XString sBuf;
  if(do_notify)
  {
    XString notification;
    if(notify == NOTIFY_NEVER)  
    {
      notification = _T("NEVER");
      sBuf.Format(_T("RCPT TO:<%s> NOTIFY=%s\r\n")
                ,recipient.m_emailAddress.GetString()
                ,notification.GetString());
    }
    else 
    {
      if(notify & NOTIFY_SUCCESS) 
      {
        notification = _T("SUCCESS");
      }
      if(notify & NOTIFY_FAILURE) 
      {
        if(!notification.IsEmpty()) notification += _T(",");
        notification += _T("FAILURE");
      }
      if(notify & NOTIFY_DELAY) 
      {
        if(!notification.IsEmpty()) notification += _T(",");
        notification += _T("DELAY");
      }
      sBuf.Format(_T("RCPT TO:<%s>"),recipient.m_emailAddress.GetString());
      if(!notification.IsEmpty())
      {
        sBuf.AppendFormat(_T(" NOTIFY=%s"),notification.GetString());
      }
      sBuf.AppendFormat(_T(" ORCPT=%s\r\n"),recipient.GetOriginalRecepientsFormat().GetString());
    }
  }
  else
  {
    sBuf.Format(_T("RCPT TO:<%s>\r\n"),recipient.m_emailAddress.GetString());
  }

  if(relay)
  {
    // RELAY
    XString rel;
    rel.Format(_T("%s: %s"),type.GetString(),recipient.m_emailAddress.GetString());
    relay->Increment();
    relay->SetText(rel);
  }
  // MESS_FAIL_RCPT "Failed in call to send RCPT TO command"
  // MESS_UNEX_RCPT "Recipients address not accepted:"
  if(!SendAndResponse(sBuf,250,MESS_FAIL_RCPT,MESS_UNEX_RCPT))
  {
    return FALSE;
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////
//
// LOW LEVEL READING THE SOCKET CHANNEL
//
//////////////////////////////////////////////////////////////

bool
SMTPConnection::SendAndResponse(XString buffer
                               ,int     expected
                               ,MESSAGE fail_mess
                               ,MESSAGE unex_mess
                               ,int     extra /* = 0*/)
{
  // Log as outgoing message
  theApp.Log(LOGLEVEL_CLIENT,buffer);

  if(SendPartial(buffer) == SOCKET_ERROR)
  {
    // Failure to send a command. Our error text + OS error text
    XString errMsg = theApp.WindowsError();
    theApp.Log(LOGLEVEL_ERROR,g_message[fail_mess][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    return false;
  }
  if(!ReadCommandResponse(expected,extra))
  {
    // We did not anticipate this on the success path
    // Tell we failed and give the last response from server
    SetLastError(ERROR_BAD_COMMAND);
    theApp.Log(LOGLEVEL_ERROR,g_message[unex_mess][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return false;
  }
  return true;
}

bool
SMTPConnection::SendAndResponse(BYTE*   buffer
                               ,int     expected
                               ,MESSAGE fail_mess
                               ,MESSAGE unex_mess
                               ,int     extra /* = 0*/)
{
#ifdef _UNICODE
  CStringW buf(CStringA((char*)buffer));
  theApp.Log(LOGLEVEL_CLIENT,buf.GetString());
  int nCmdLength = (int)buf.GetLength();
#else
  theApp.Log(LOGLEVEL_CLIENT,(LPCTSTR)buffer);
  int nCmdLength = (int)_tcslen((char*)buffer);
#endif

  if(m_socket->SendPartial(buffer,nCmdLength) == SOCKET_ERROR)
  {
    // Failure to send a command. Our error text + OS error text
    XString errMsg = theApp.WindowsError();
    theApp.Log(LOGLEVEL_ERROR,g_message[fail_mess][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    return false;
  }
  if(!ReadCommandResponse(expected,extra))
  {
    // We did not anticipate this on the success path
    // Tell we failed and give the last response from server
    SetLastError(ERROR_BAD_COMMAND);
    theApp.Log(LOGLEVEL_ERROR,g_message[unex_mess][g_lang]);
    theApp.Log(LOGLEVEL_ERROR,m_lastCommandResponse);
    return false;
  }
  return true;
}

int
SMTPConnection::SendPartial(XString p_toSend)
{
#ifdef _UNICODE
  BYTE* buffer = nullptr;
  int   length = 0;
  TryCreateNarrowString(p_toSend,_T("utf-8"),false,&buffer,length);
  int result = m_socket->SendPartial(buffer,length);
  delete[] buffer;
  return result;
#else
  XString send = EncodeStringForTheWire(p_toSend);
  return m_socket->SendPartial(send.GetString(),send.GetLength());
#endif
}

int
SMTPConnection::SendMsg(XString p_message)
{
#ifdef _UNICODE
  BYTE* buffer = nullptr;
  int   length = 0;
  TryCreateNarrowString(p_message,_T("utf-8"),false,&buffer,length);
  int result = m_socket->SendMsg(buffer,length);
  delete[] buffer;
  return result;
#else
  XString send = EncodeStringForTheWire(p_message);
  return m_socket->SendMsg(send.GetString(),send.GetLength());
#endif
}

const int nBufferSize   = 1024;
const int nBufferGrowBy = 4096;

bool
SMTPConnection::ReadCommandResponse(int nExpectedCode,int nExtraCode /*=0*/)
{
  char sBuf[4 * nBufferSize + 10];
  ZeroMemory(sBuf,sizeof(sBuf));

  if(ReadResponse(sBuf,4 * nBufferSize,"\r\n"))
  {
    // determine if the response is an error. 
    // The SMTP protocol always begin with a 3 digit number!
    char sCode[4];
    strncpy_s(sCode,4,sBuf,_TRUNCATE);
    sCode[3] = '\0';
    m_lastCommandResponseCode = atoi(sCode);

    if(m_lastCommandResponseCode != nExpectedCode)
    {
      if(nExtraCode && m_lastCommandResponseCode == nExtraCode)
      {
        return true;
      }
      // Unexpected network error = unexpected in SMTP answer
      SetLastError(ERROR_UNEXP_NET_ERR);
      // Connection now in error state
      m_errors = true;
      return false;
    }
    return true;
  }
  // Connection now in error state
  m_errors = true;
  return false;
}

bool
SMTPConnection::ReadResponse(LPSTR p_buffer
                            ,int   p_initialBufSize
                            ,LPSTR p_terminator)
{
  USES_CONVERSION;
  LPSTR pszOverFlowBuffer = NULL;

  //must have been created first
  if(!m_connected)
  {
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_INTERN_SOCK][g_lang]);
  }

  //The local variables which will receive the data
  LPSTR pszRecvBuffer = p_buffer;
  int nBufSize = p_initialBufSize;

  // Must have a receive buffer
  if(pszRecvBuffer == NULL)
  {
    return false;
  }

  if(!m_lastReceivedBuffer.IsEmpty())
  {
    // Left over from last call. Eat this first!
    if(m_lastReceivedBuffer.GetLength() > p_initialBufSize)
    {
      strncpy_s(pszRecvBuffer,p_initialBufSize,m_lastReceivedBuffer.Left(p_initialBufSize),p_initialBufSize);
      m_lastReceivedBuffer = m_lastReceivedBuffer.Mid(p_initialBufSize);
    }
    else
    {
      strncpy_s(pszRecvBuffer,p_initialBufSize,m_lastReceivedBuffer,p_initialBufSize);
    }
  }
  else
  {
    // retrieve the response using until we
    // get the terminator or a timeout occurs
    BOOL  bFoundTerminator = FALSE;
    int   nReceived = 0;

    while(!bFoundTerminator)
    {

      //check the socket for readability
      bool bReadible;
      if(!m_socket->IsReadible(bReadible))
      {
        pszRecvBuffer[nReceived] = '\0';
        //MESS_LOST_CONNECTION "SMTP Server channel was not accessible or suffered from lost connection"
        theApp.Log(LOGLEVEL_ERROR,g_message[MESS_LOST_CONNECTION][g_lang]);
        //MESS_LASTBUFFER "Last known received buffer:"
        theApp.Log(LOGLEVEL_ERROR,g_message[MESS_LASTBUFFER][g_lang]);
        theApp.Log(LOGLEVEL_ERROR,A2T(pszRecvBuffer));
        return FALSE;
      }
      else if(!bReadible) //no data to receive, just loop around
      {
        //Sleep for a while before we loop around again
        Sleep(SLEEP_WAIT_ON_SERVER);
        continue;
      }

      //receive the data from the socket
      int nBufRemaining = nBufSize - nReceived - 1; //Allows allow one space for the NULL terminator
      if(nBufRemaining < 0)
      {
        nBufRemaining = 0;
      }
      int nData = m_socket->RecvPartial(pszRecvBuffer + nReceived,nBufRemaining);

      if(nData)
      {

        //Increment the count of data received
        nReceived += nData;
      }

      // If an error occurred receiving the data
      // Also come here in case of a timeout of the socket
      if(nData == SOCKET_ERROR)
      {
        //NULL terminate the data received
        if(pszRecvBuffer)
        {
          p_buffer[nReceived] = '\0';
        }
        // MESS_LOST_SOCKET "SMTP Server channel : socket error, connection lost"
        theApp.Log(LOGLEVEL_ERROR,g_message[MESS_LOST_SOCKET][g_lang]);
        // MESS_LASTBUFFER "Last known received buffer:"
        theApp.Log(LOGLEVEL_ERROR,g_message[MESS_LASTBUFFER][g_lang]);
        theApp.Log(LOGLEVEL_ERROR,A2T(pszRecvBuffer));
        return FALSE;
      }
      else
      {
        // NULL terminate the data received
        if(pszRecvBuffer)
        {
          pszRecvBuffer[nReceived] = '\0';
        }
        if(nBufRemaining - nData == 0) //No space left in the current buffer
        {
          // Allocate the new receive buffer
          nBufSize += nBufferGrowBy; //Grow the buffer by the specified amount
          LPSTR pszNewBuf = new char[nBufSize];

          // copy the old contents over to the new buffer and assign 
          // the new buffer to the local variable used for retrieving 
          // from the socket
          if(pszRecvBuffer)
          {
            strcpy_s(pszNewBuf,nBufSize,pszRecvBuffer);
          }
          pszRecvBuffer = pszNewBuf;

          // delete the old buffer if it was allocated
          if(pszOverFlowBuffer && *pszOverFlowBuffer)
          {
            delete[] pszOverFlowBuffer;
          }
          // Remember the overflow buffer for the next time around
          pszOverFlowBuffer = pszNewBuf;
        }
      }
      // Check to see if the terminator character(s) have been found
      if(pszRecvBuffer)
      {
        bFoundTerminator = (strstr(pszRecvBuffer,p_terminator) != NULL);
      }
    }
    // If overflow buffer was used, now delete it. 
    // We lose the rest of the protocol!!!
    if(pszOverFlowBuffer)
    {
      strncpy_s(p_buffer,p_initialBufSize,pszOverFlowBuffer,p_initialBufSize);
      delete[] pszOverFlowBuffer;
      pszRecvBuffer = p_buffer;
    }
  }
  // Find first and last terminator
  char* firstTerminator = strstr(pszRecvBuffer,p_terminator);
  char* lastTerminator = strrstr(pszRecvBuffer,p_terminator);
  // Terminate it right away
  if(firstTerminator)
  {
    *firstTerminator = 0;
  }
  if(firstTerminator != lastTerminator)
  {
     // OK Received multiple lines of input buffer from socket
    firstTerminator += 2;
    m_lastReceivedBuffer = firstTerminator;
  }
  else
  {
    m_lastReceivedBuffer = "";
  }
  // Hive away the last command response
  m_lastCommandResponse = A2T(pszRecvBuffer);

  // Now log it as a server answer
  theApp.Log(LOGLEVEL_SERVER,m_lastCommandResponse);
  return true;
}

// Poor man's implementation of finding the last occurrence of a substring
// BEWARE: Always ANSI implementation
char*
SMTPConnection::strrstr(char* p_total,char* p_part)
{
  char* found1 = strstr(p_total,p_part);
  char* found2 = found1;
  while(found1 && found2)
  {
    found2 = found1;
    ++found2;
    found2 = strstr(found2,p_part);
    if(found2)
    {
      found1 = found2;
    }
  }
  return found1;
}
