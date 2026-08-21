/////////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
//
// SHELL RETURN CODES DOCUMENTATION
//
// Program returns an integer value to the command shell
// Internally this is the "m_returnValue" attribute
//
// 0   = Mail was sent / function accomplished (or configuration done)
// 1   = Cancel button was hit
// 2   = Mail viewer ended correctly
// 3   = Errors in the email found / not sent
// 4   = SMTP Transmit error in sending the mail
// 5   = No default mail server in 'PostMail.Mailservers.ini'
// 11  = Mail sent OK / First profile was used
// 12  = Mail sent OK / Second profile was used
// ..  =
// 10+n  The number of the profile is found by subtracting 10
//
#include "stdafx.h"
#include "PostMail.h"

#include "SMTPAddress.h"
#include "SMTPAttachment.h"
#include "SMTPConnection.h"
#include "SMTPMessage.h"
#include "OutBox.h"
#include "OutBoxDlg.h"
#include "Message.h"
#include "RelayDlg.h"
#include "ChooseProfileDlg.h"
#include "Profiles.h"
#include "ProfileManagamentDlg.h"
#include "Version.h"
#include "Logging.h"
#include "Office365.h"
#include "EncryptDlg.h"
#include <ExecuteProcess.h>
#include <ServiceReporting.h>
#include <io.h>
#include <time.h>
#include <sys/timeb.h>
#include <Base64.h>
#include <Crypto.h>

#pragma warning(disable: 6284)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// My identity
TCHAR WhoAmI[] = _T(WHOAMI);

// The one and only PostMail App object
PostMail theApp;

// CPostMailApp
BEGIN_MESSAGE_MAP(PostMail, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// CPostMailApp construction
PostMail::PostMail()
{
  m_logLevel        = 0;
  m_server          = false;
  m_text            = false;
  m_viewer          = false;
  m_config          = false;
  m_doProfile       = false;
  m_doHelp          = false;
  m_doOutbox        = false;
  m_defaultProfile  = 0;
  m_useIDCache      = false;
  m_mailID          = 0;
  m_returnValue     = 0;
  m_logAppend       = false;
  m_stackErrors     = false;
  m_doLogfile       = false;
  m_doHexlog        = false;
  m_doEncrypt       = false;
  m_logfile         = nullptr;
  m_currentProfile  = nullptr;
}

// CPostMailApp initialization
BOOL PostMail::InitInstance()
{
  INITCOMMONCONTROLSEX InitCtrls;
  InitCtrls.dwSize = sizeof(InitCtrls);
  InitCtrls.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&InitCtrls);

  // Body of the email is potentially a RTF text
  AfxInitRichEdit2();
  AfxEnableControlContainer();

  CWinApp::InitInstance();

  // Set service name
  _tcscpy_s(g_svcname,SERVICE_NAME_LENGTH,APPLICATION);

  // DEBUG: Waiting for a DEBUG connection
  // Sleep(20000);

	// Initialize sockets for SMTP traffic
	if(!AfxSocketInit())
  {
    // MESS_WINSOCK: Cannot load the MS-WIndows socket libraries
    theApp.WideMessageBox(NULL
                         ,g_message[MESS_WINSOCK][g_lang]
                         ,WhoAmI
                         ,MB_OK | MB_TASKMODAL | MB_ICONERROR | MB_SETFOREGROUND);
    return FALSE;
  }

  SetRegistryKey(PRODUCT_REGISTRY);

  // Get the parameter for this run
  GetWorkingDir();

  // Getting all options
  GetOptionsFromCommandline();
  // Check if from server process
  IsUserInteractive();

  // Switching to the correct language
  switch(g_lang)
  {
    case LANGUAGE_NEDERLANDS: SetStyleFxLanguage(StyleLanguage::SL_DUTCH);  break;
    case LANGUAGE_ENGLISH:    SetStyleFxLanguage(StyleLanguage::SL_ENGLISH);break;
    case LANGUAGE_FRANCAIS:   SetStyleFxLanguage(StyleLanguage::SL_FRENCH); break;
    case LANGUAGE_DEUTSCH:    SetStyleFxLanguage(StyleLanguage::SL_GERMAN); break;
  }

  // Show help page and exit: no valid options on commandline
  if(m_doHelp)
  {
    DoHelpPage();
    exit(4);
  }

  // Only encryption asked of us.
  if(m_doEncrypt)
  {
    EncryptDlg dlg;
    dlg.DoModal();
    return FALSE;
  }

  // First thing to do after reading the command line
  // is starting or appending a logfile if so requested
  if(m_doLogfile)
  {
    OpenLogfile(m_doHexlog ? LOGLEVEL_TRACE : LOGLEVEL_SERVER);
  }
  // Connect logging functions of SSLSocket to PostMail
  printing = PostMailLogString;

  // Is there an OFFICE-365 configuration?
  bool graph = CheckJSONConfig();
  if(graph)
  {
    // Do we have a correct JSON MS-GRAPH configuration
    if(!CheckGraph())
    {
      return FALSE;
    }
  }
  else
  {
    // Read the mailservers.ini file
    m_profiles.ReadMailServers();

    // Do we have a correct legacy SMTP configuration
    if(!CheckSMTP())
    {
      return FALSE;
    }
  }

  if(m_doOutbox && !m_server)
  {
    if(graph)
    {
      ShellExecute(nullptr,_T("open"),OFFICE365_MAIL_INBOX,_T(""),_T(""),SW_SHOW);
      return TRUE;
    }
    else
    {
      OutBox outbox;
      OutBoxDlg dlg(NULL,outbox);
      dlg.DoModal();
      return FALSE;
    }
  }

  // Create a message on the stack
  SMTPMessage mail;
  bool showResults = true;

  if(m_useIDCache)
  {
    // Handle mail ID
    showResults = HandleIDCache(mail,graph);
  }
  else
  {
    // Do the mailfile parameter
    HandleMailFile(mail,graph);
  }

  if(showResults)
  {
    // Show send results and/or errors
    NotifyResults(mail);
  }
  CloseLogfile();

  if(showResults)
  {
    // Show any logfile results?
    DisplayLogfile();
  }

  // Show in the central logfile?
  if(!graph)
  {
    if(m_profiles.GetHasCentralLogfile())
    {
      LogInCentralLogfile(mail);
    }
  }

  // Status exit with 0 -> ALL OK;
  // ExitInstance delivers m_returnValue to the OS!
  return FALSE;
}

int
PostMail::ExitInstance()
{
  // Run the notification commands for our ERP application
  NotifyERPFramework();

  // Any changes made to the profiles must be written here
  // before we hit the XTOR or the Terminate Process below
  m_profiles.WriteProfiles();

  // Currently we have much havoc from MS-Windows WARP service for 3D rasterization
  // the "d3d10warp.dll" keeps hanging in the DetachDLL function on non-existing
  // threads and events
  // This occurs since the 1709 build of Windows 10
  TerminateProcess(GetCurrentProcess(),m_returnValue);

  // Simply return it
  return m_returnValue;
}

// Checking the SMTP configuration (profiles, mailservers)
bool
PostMail::CheckSMTP()
{
  // Check mail servers. Errors to the logfile
  CheckMailServers();

  // Read all profiles
  m_profiles.ReadProfiles();

  // Potentially default profile from the commandline
  if(m_defaultProfile)
  {
    if(m_profiles.SetChosenProfile(m_defaultProfile) == false)
    {
      WriteProfileInt(_T(""),_T("LastProfile"),0);
      ChooseProfile();
    }
    else
    {
      m_currentProfile = m_profiles.GetProfile(m_defaultProfile);
    }
  }
  else if(!m_profile.IsEmpty())
  {
    if(m_profiles.SetChosenProfile(m_profile) == false)
    {
      WriteProfileInt(_T(""),_T("LastProfile"),0);
      ChooseProfile();
    }
    else
    {
      m_currentProfile = m_profiles.GetProfile(m_profile);
    }
  }
  else
  {
    if(m_profiles.GetNumberOfProfiles() == 1)
    {
      // Special case: exact 1 profile, never a choice
      m_defaultProfile = 1;
      m_profiles.SetChosenProfile(m_defaultProfile);
    }
    else if(m_profiles.GetNumberOfProfiles() > 1)
    {
      if(m_server)
      {
        // In case of a server backend: never a choice menu
        if(m_defaultProfile == 0 && m_profiles.GetNumberOfProfiles() > 1)
        {
          Profile* stdprof = m_profiles.GetStandardProfile();
          m_defaultProfile = stdprof ? stdprof->m_number : 0;
        }
      }
      else
      {
        // Let the user decide
        ChooseProfile();
      }
    }
  }
  if(m_config && !m_server)
  {
    ProfileManagamentDlg dlg(NULL,m_profiles);
    dlg.DoModal();
    m_profiles.WriteProfiles();
    return FALSE;
  }
  return TRUE;
}

bool
PostMail::CheckGraph()
{
  bool result = m_odataConfig.ReadConfig();
  if(!result)
  {
    Log(LOGLEVEL_TERSE,g_message[MESS_MANUAL365][g_lang]);
  }
  return result;
}

// Check mail servers
void
PostMail::CheckMailServers()
{
  if(m_profiles.GetMailServers().empty())
  {
    // Error already seen. Configure default mail server first
    Log(LOGLEVEL_ERROR,g_message[MESS_DEFSERVER][g_lang]);
    // No filled 'PostMail.Mailservers.ini'
    exit(5);
  }
}

void
PostMail::ChooseProfile()
{
  // Last chosen profile
  int profile = GetProfileInt(_T(""),_T("LastProfile"),0);

  if(profile == 0)
  {
    // Not yet chosen. Do that now
    ChooseProfileDlg dlg(NULL,m_profiles);
    dlg.DoModal();
    m_defaultProfile = m_profiles.GetChosenProfile();
  }
  else
  {
    m_defaultProfile = profile;
    m_profiles.SetChosenProfile(m_defaultProfile);
  }
  m_currentProfile = m_profiles.GetProfile(m_defaultProfile);
}

void
PostMail::HandleMailFile(SMTPMessage& p_mail,bool p_graph)
{
  if(!m_mailFilename.IsEmpty())
  {
    if(p_mail.ReadFromFile(m_mailFilename,true))
    {
      p_mail.CheckAfterRead(m_viewer,p_graph);
    }  
  }
  else
  {
    p_mail.SetEditSubject(true);
    p_mail.SetEditBody(true);
  }

  // If a profile is chosen, use it for this mail
  if(m_currentProfile) 
  {
    p_mail.UseProfile(m_currentProfile);
  }

  // No show or accepted message in the dialog
  if(p_mail.DisplayMessage())
  {
    if(m_viewer)
    {
      m_returnValue = 2;
    }
    else
    {
      // Last check against profiles and SEND IT!
      p_mail.CheckHost();

      // From here: stack all errors until showing notifications
      DoStackErrors(true);

      bool sent = true;
      if(p_mail.SendMessage() == false)
      {
        // Tried to send, but received an error
        // Connect error, transmit error etc.
        m_returnValue = 4;
        sent = false;
      }
      OutBox box;
      box.SaveInOutbox(p_mail,sent);

      // Show errors again
      DoStackErrors(false);

      if(!m_server)
      {
        box.FullOutboxWarning();
      }
    }
  }
  else
  {
    // Return status 1 : Cancel button pressed
    m_returnValue = 1;
  }
}

// Handle mail ID cache
bool
PostMail::HandleIDCache(SMTPMessage& p_mail,bool p_graph)
{
  // If explicit ID given, handle it
  if(m_mailID)
  {
    // Handle one mail, read status sofar
    m_cache.Read(p_mail,m_mailID);

    if(m_mailParam.IsEmpty())
    {
      // Send this mail
      p_mail.CheckAfterRead(m_viewer,p_graph);
      // No show or accepted message in the dialog
      if(p_mail.DisplayMessage())
      {
        if(m_viewer)
        {
          m_returnValue = 2;
        }
        else
        {
          bool sent = false;
          p_mail.CheckHost();
          if(p_mail.SendMessage())
          {
            sent = true;
          }
          else
          {
            // Tried to send, but received an error
            m_returnValue = 4;
          }
          OutBox box;
          if(box.SaveInOutbox(p_mail,sent))
          {
            m_cache.RemoveEmailFile(m_mailID);
          }
        }
      }
      else
      {
        // Return status 1 : Cancel button pressed
        m_returnValue = 1;
      }
    }
    else
    {
      // Add parameter to this Mail
      XString fileName = m_cache.GetFilenameForMailID(m_mailID);
      p_mail.AddParameter(m_mailParam,m_parameter);
      p_mail.SaveToFile(fileName);
      // Do no logging
      return false;
    }
  }
  else
  {
    // No ID found, Flush the cache
    m_cache.ReadAllMail();
    m_cache.Flush(m_viewer,p_graph);
  }
  // Must do logging
  return true;
}

void
PostMail::NotifyResults(SMTPMessage& p_mail)
{
  if(m_totalErrorState.IsEmpty())
  {
    if(p_mail.GetSendNotify() && !m_viewer)
    {
      // MESS_CONFIRM: "Mail message%s sent to: %s"
      // MESS_NOT:     "NOT "
      XString mesg;
      mesg.Format(g_message[MESS_CONFIRM][g_lang]
                 ,m_returnValue==1? g_message[MESS_NOT][g_lang]:_T("")
                 ,p_mail.GetRecipient(0,TO).GetRegularFormat().GetString());
      this->WideMessageBox(NULL
                          ,mesg
                          ,WhoAmI
                          ,MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
    }
    if(p_mail.GetDelete())
    {
      DeleteFile(m_mailFilename);
    }
    if(m_doProfile)
    {
      int gekozen   = m_profiles.GetChosenProfile();
      bool nietmeer = m_profiles.GetDoNotAskAgain();
      if(gekozen >= 0 && nietmeer)
      {
        // MAGIC: return 10 + profile number back to the shell (profile = 1-based)
        m_returnValue = 11 + gekozen;
      }
    }
  }
  else
  {
    if(p_mail.GetShowErrors())
    {
      // Add graph errors
      if(!p_mail.GetErrors().IsEmpty())
      {
        m_totalErrorState += p_mail.GetErrors();
      }

      this->WideMessageBox(NULL
                          ,m_totalErrorState
                          ,WhoAmI
                          ,MB_OK | MB_TASKMODAL | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
      m_returnValue = 3;
    }
  }
}

void
PostMail::GetWorkingDir()
{
  TCHAR buffer[_MAX_PATH];

  GetModuleFileName(GetModuleHandle(NULL), buffer, _MAX_PATH);
  XString applicatiePlusPad = buffer;

  int slashPositie = applicatiePlusPad.ReverseFind(_T('\\'));
  if (slashPositie == 0)
  {
    return;
  }
  m_workingDir = applicatiePlusPad.Left(slashPositie + 1);
  XString filename = applicatiePlusPad.Mid(slashPositie + 1);
  // Check for bad consultants!
  CheckRuntimer(filename);
}

void
PostMail::GetOptionsFromCommandline()
{
  for(int ind = 1;ind < __argc; ++ind)
  {
    XString line = __targv[ind];
    if(line.GetAt(0) == _T('/'))
    {
      int option = _totupper((TCHAR) line.GetAt(1));
      switch(option)
      {
        case 'N': g_lang = LANGUAGE_NEDERLANDS;
                  break;
        case 'E': g_lang = LANGUAGE_ENGLISH;
                  break;
        case 'F': g_lang = LANGUAGE_FRANCAIS;
                  break;
        case 'D': g_lang = LANGUAGE_DEUTSCH;
                  break;
        case 'L': m_doLogfile = true;
                  break;
        case 'X': m_doLogfile = true;
                  m_doHexlog  = true;
                  break;
        case 'V': m_viewer = true; // We are a viewer
                  break;
        case 'C': m_config = true; // We do configuration
                  break;
        case 'S': m_server = true; // We are a server process
                  break;
        case 'T': m_text   = true; // We are in text mode instead of HTML mode
                  break;
        case '?': // Fall Through
        case 'H': m_doHelp = true; // Do a help page
                  break;
        case 'O': m_doOutbox = true;
                  break;
        case 'K': m_doEncrypt = true;
                  break;
        case 'P': m_doProfile = true; // we do profiles
                  GetDefaultProfile(line);
                  break;
        case 'I': GetIDOption(line);
                  break;
        case 'A': GetLogFile(line);
                  break;
        case 'U': GetReconnectCommand(line);
                  break;
        case 'R': GetReadyCommand(line);
                  break;
        case 'Q': GetPingCommand(line);
                  break;
        case 'Z': GetProfileCommand(line);
                  break;
        default:  m_doHelp = true;
                  break;
      }
    }
    else // Option without a /, must be the mailfile.txt
    {
      m_mailFilename = line;
      m_chngFilename = m_mailFilename + _T(".chng");
    }
  }
}

void    
PostMail::GetDefaultProfile(XString& p_line)
{
  if((p_line.GetLength() > 2) && (p_line.GetAt(2) == _T(':')))
  {
    if(_istdigit((TCHAR) p_line.GetAt(3)))
    {
      m_defaultProfile = _ttoi(p_line.Mid(3));
    }
    else
    {
      m_profile = p_line.Mid(3);
    }
    return;
  }
  // Only "/P" given: Reset last profile
  WriteProfileInt(_T(""),_T("LastProfile"),0);
}

// Get the Parameter file ID option
void
PostMail::GetIDOption(XString& p_line)
{
  // Option must be "/ID"
  if (_totupper((TCHAR) p_line.GetAt(2)) != _T('D'))
  {
    p_line = p_line.Mid(3);
    p_line.TrimLeft();
    m_doHelp = true;
  }
  // ID cache setting 
  m_useIDCache = true;

  // Next option
  p_line = p_line.Mid(3);

  // Check for ID number
  if (p_line.GetAt(0) == _T(':'))
  {
    m_mailID = _ttoi(p_line.Mid(1));

    // Get remaining p_line
    int pos = p_line.Find(_T('/'));
    if (pos >= 0)
    {
      p_line = p_line.Mid(pos);
    }

    // Check if we have a mail parameter
    if(p_line.GetAt(0) == _T('/'))
    {
      pos = p_line.Find(_T(':'));
      if(pos < 0)
      {
        m_mailParam = p_line.Mid(1);
      }
      else
      {
        m_mailParam = p_line.Mid(1,pos-1);
        m_parameter = p_line.Mid(pos+1);
      }
    }
  }
}

void
PostMail::GetLogFile(XString& p_line)
{
  if(p_line.GetLength() > 4 && p_line.GetAt(2) == _T(':'))
  {
    m_logFilename = p_line.Mid(3);
    m_logAppend = true;
  }
}

void
PostMail::GetReconnectCommand(const XString& p_line)
{
  // Using the current program directory as our passkey
  TCHAR currentDirectory[MAX_PATH + 1];
  GetCurrentDirectory(MAX_PATH,currentDirectory);
  XString passkey(currentDirectory);
  passkey.MakeLower();

  // Getting the command to decode
  if(p_line.GetLength() > 4 && p_line.GetAt(2) == _T(':'))
  {
    m_reconnectCommand = p_line.Mid(3);
  }

  // Try to decrypt the resulting command
  Crypto crypt;
  m_reconnectCommand = crypt.Decryption(m_reconnectCommand,passkey);

  // If nothing decrypted: Maybe it was not encrypted after all.
  if(m_reconnectCommand.IsEmpty())
  {
    m_reconnectCommand = p_line;
  }
}

void
PostMail::GetReadyCommand(XString& p_line)
{
  if(p_line.GetLength() > 4 &&  p_line.GetAt(2) == _T(':'))
  {
    m_readyCommand = p_line.Mid(3);
  }
}

void
PostMail::GetPingCommand(XString& p_line)
{
  if(p_line.GetLength() > 4 &&  p_line.GetAt(2) == _T(':'))
  {
    m_pingCommand = p_line.Mid(3);
  }
}

void
PostMail::GetProfileCommand(XString& p_line)
{
  if(p_line.GetLength() > 4 &&  p_line.GetAt(2) == _T(':'))
  {
    m_profileCommand = p_line.Mid(3);
  }
}

// General MS-Windows Error. Gets the last error
// In case the buffer is not big enough: show a message right away
XString
PostMail::WindowsError()
{
  // Calling GetLastError right away
  DWORD errorNum = GetLastError();
  TCHAR  buffer1[2048 + 1];
  TCHAR  buffer2[2048 + 1];

  buffer1[0] = 0;
  buffer2[0] = 0;
  SetLastError(0);
  if(!errorNum)
  {
    return XString(_T(""));
  }
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM     | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorNum,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &buffer1,
                2048,
                NULL 
                );
  _sntprintf_s(buffer2,2048,_T("\r\nMS-Windows: %d\r\n%s\r\n"),errorNum,buffer1);
  return XString(buffer2);
}

// Open the logfile for writing
bool 
PostMail::OpenLogfile(int p_loglevel)
{
  // Check if already open
  if(m_logfile && m_logfile->GetIsOpen())
  {
    return true;
  }
  // Record the log level
  m_logLevel = p_loglevel;

  // Use levels 1 and 2 for tracing
  if(p_loglevel >= LOGLEVEL_TRACE)
  {
    SetSocketLogging(SOCK_LOGGING_FULLTRACE);
  }

  // Getting the directory for the logfile
  if(m_logFilename.GetEnvironmentVariable(_T("TMP")) == FALSE)
  {
    // Fallback to old-school function and paths
    // TMP -> TEMP -> USERPROFILE -> Windows system TEMP
    TCHAR filename[MAX_PATH + 1];
    GetTempPath(MAX_PATH,filename);
    m_logFilename = filename;
  }
  if(m_logFilename.Right(1) != _T("\\"))
  {
    m_logFilename += _T("\\");
  }
  
  // Construct full logfile name
  XString file = _T("Logfile_");
  file += WhoAmI;
  file.Replace(_T(" "),_T(""));
  file.Replace(_T("."),_T(""));
  m_logFilename += file;
  m_logFilename += _T(".txt");
    
  m_logfile = LogAnalysis::CreateLogfile(WhoAmI);
  
  m_logfile->SetLogFilename(m_logFilename);
  m_logfile->SetLogLevel(HLL_LOGGING);
  m_logfile->SetLogRotation(true);
  m_logfile->SetDoLogging(true);
  if(m_server)
  {
    m_logfile->SetDoEvents(true);
  }

  // Header to get the log started
  int language = g_lang < 0 ? 0 : g_lang;
  m_logfile->AnalysisLog(_T(__FUNCTION__),LogType::LOG_INFO,false,g_message[MESS_STARTLOG][language]);
  return m_logfile->GetIsOpen();
}

void
PostMail::CloseLogfile()
{
  if(m_logfile && m_logfile->GetIsOpen())
  {
    // "End of the logfile for PostMail"
    XString line(_T("---------------------------------------------------------------------"));
    m_logfile->AnalysisLog(_T(__FUNCTION__),LogType::LOG_INFO,false,line);
    m_logfile->AnalysisLog(_T(__FUNCTION__),LogType::LOG_INFO,false,g_message[MESS_ENDLOG][g_lang]);
  }
  if(m_logfile)
  {
    m_logfile->Reset();
    LogAnalysis::DeleteLogfile(m_logfile);
    m_logfile = nullptr;
  }
}

// LOGLEVELS
// 0 - Log terse-mode
// 1 - Log silent on debug (protocol) Client request
// 2 - Log silent on debug (protocol) Server answer
// 3 - Log always client&server and log hexadecimal dumps
// 4 - Log always and show all in message boxes 
//
void 
PostMail::Log(int p_level,const XString& p_text,bool p_newline /*=true*/)
{
  if(p_level == LOGLEVEL_ERROR)
  {
    m_totalErrorState += p_text + _T("\n");
    if(!p_text.IsEmpty() && !m_stackErrors && !m_server)
    {
      theApp.WideMessageBox(NULL,p_text,WhoAmI,MB_OK|MB_ICONERROR);
    }
    else
    {
      if(OpenLogfile(LOGLEVEL_TERSE) == false)
      {
        // Be sure we have the error (even if no logfile)
        SvcReportErrorEvent(0,false,_T(__FUNCTION__),p_text);
      }
    }
  }
  if(m_logfile && m_logfile->GetIsOpen() && ((p_level <= m_logLevel) || (p_level == LOGLEVEL_ERROR)))
  {
    XString text;
    LogType type(LogType::LOG_INFO);
    switch(p_level)
    {
      case LOGLEVEL_CLIENT: text = _T("CLIENT: ") + p_text;
                            break;
      case LOGLEVEL_SERVER: text = _T("SERVER: ") + p_text;
                            break;
      case LOGLEVEL_ERROR:  text = _T("ERROR: ") + p_text;
                            type = LogType::LOG_ERROR;
                            break;
      default:              text = p_text;
                            break;
    }
    if(p_newline)
    {
      text += _T("\n");
    }
    m_logfile->AnalysisLog(WhoAmI,type,false,text);
  }
  // Show live debug feed in Visual Studio
#ifdef _DEBUG
  OutputDebugString(p_text);
  if(p_newline && p_text.GetLength()>1 && p_text.Right(1) != _T("\n"))
  {
    OutputDebugString(_T("\n"));
  }
#endif
}

void
PostMail::LogString(LPCTSTR p_string)
{
  XString string(p_string);
  if(m_logfile)
  {
    m_logfile->BareStringLog(string);
  }
#ifdef _DEBUG
  OutputDebugString(p_string);
#endif
}

void
PostMail::LogHexString(LPCTSTR p_string)
{
  m_logfile->AnalysisHex(WhoAmI,_T("TLS"),(void*)p_string,sizeof(p_string) * sizeof(TCHAR));
}

void __stdcall PostMailLogString(LPCTSTR lpOutputString)
{
  switch(SSL_socket_logging)
  {
    default:
    case SOCK_LOGGING_OFF:      // Fall through
    case SOCK_LOGGING_ON:       // Fall through
    case SOCK_LOGGING_TRACE:    theApp.LogString(lpOutputString);
                                break;
    case SOCK_LOGGING_FULLTRACE:theApp.LogHexString(lpOutputString);
                                break;
  }
}

void
PostMail::DisplayLogfile()
{
  // Never for a server process in the background (m_server)
  // Never if we append to another programs logfile (m_logAppend)
  // Never if we did not set any logfile, so nothing to show
  if(m_server || m_logAppend || m_logFilename.IsEmpty())
  {
    // Nothing to display
    return;
  }
  // Automatic open a *.txt file with the default TXT viewer (e.g. notepad.exe)
  ShellExecute(NULL,_T("open"),m_logFilename,NULL,NULL,SW_SHOW);
}

int
PostMail::WideMessageBox(HWND p_hwnd ,LPCTSTR p_message ,LPCTSTR p_title ,int p_buttons /*= MB_OK*/)
{
  // Stack the error
  if(m_stackErrors && (p_buttons & MB_ICONERROR))
  {
    m_totalErrorState += p_message;
  }
  // Background processing
  if(m_server)
  {
    // Server variant: Cannot show any message box
    if(OpenLogfile(LOGLEVEL_TERSE))
    {
      int loglevel = (p_buttons & MB_ICONERROR) ? LOGLEVEL_ERROR : LOGLEVEL_TERSE;
      Log(loglevel,p_message);
    }
    else
    {
      // Be sure we have the error (even if no logfile)
      SvcReportErrorEvent(0,false,_T(__FUNCTION__),p_message);
    }
    return IDNO;
  }

  if(!m_stackErrors)
  {
    if(p_hwnd)
    {
      SetForegroundWindow(p_hwnd);
    }
    else if(m_pMainWnd)
    {
      SetForegroundWindow(m_pMainWnd->GetSafeHwnd());
      p_hwnd = m_pMainWnd->GetSafeHwnd();
    }
    return (int)::StyleMessageBox(CWnd::FromHandle(p_hwnd),p_message,p_title,p_buttons | MB_SETFOREGROUND);
  }
  // Answering message boxes will always be false
  // Stacking errors does not require a return value
  return IDNO;
}

void
PostMail::WriteChanges(const XString& p_type,const XString& p_text)
{
  if(m_chngFilename.IsEmpty())
  {
    return;
  }
  WinFile file(m_chngFilename);
  if(file.Open(winfile_append,FAttributes::attrib_archive,Encoding::UTF8))
  {
    file.Format(_T("<%s>\n"),p_type.GetString());
    file.Format(_T("%s\n"),  p_text.GetString());
    file.Close();
  }
}

// Results for this e-mail are written in the central logfile
// Can only be called if m_profiles.GetHasCentralLogfile() returns "true"!
void
PostMail::LogInCentralLogfile(SMTPMessage& p_mail)
{
  extern XString GetLoginName();

  for(int retry = 0; retry < 3; ++retry)
  {
    WinFile file(m_profiles.GetCentralLogfile());
    if(file.Open(winfile_append,FAttributes::attrib_none,Encoding::UTF8))
    {
      XString timing;
      __timeb64 now;
      struct tm today;

      _ftime64_s(&now);
      _localtime64_s(&today,&now.time);
      timing.Format(_T("%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%03d")
                    ,today.tm_year + 1900
                    ,today.tm_mon  + 1
                    ,today.tm_mday
                    ,today.tm_hour
                    ,today.tm_min
                    ,today.tm_sec
                    ,now.millitm);

      // Get data from the email
      XString from    = p_mail.GetSender()->m_emailAddress;
      XString mailto  = p_mail.GetRecipient(0,TO).m_emailAddress;
      XString subject = p_mail.GetSubject();
      subject.Replace(_T("\r\n"),_T("_"));

      // Calculate the status
      XString status;
      switch(m_returnValue)
      {
        default:      // 11 and bigger, mailed by profile
        case 0:       status = _T("Mailed"); break;
        case 1:       status = _T("CANCEL"); break;
        case 2:       status = _T("Viewed"); break;
        case 3:       // Errors in email
        case 4:       // Errors in SMTP transmit
        case 5:       // reserved
        case 6:       // reserved
        case 7:       // reserved
        case 8:       // reserved
        case 9:       status = _T("FAILED"); break;
        case 10:      status = _T("PROFIL"); break;
      }

      // Print line in the logfile
      file.Format(_T("%s %s By:%s "),timing.GetString(),status.GetString(),GetLoginName().GetString());
      if(!from.IsEmpty())
      {
        file.Format(_T("From:%s "),from.GetString());
      }
      if(!mailto.IsEmpty())
      {
        file.Format(_T("TO:%s "),mailto.GetString());
      }
      if(!subject.IsEmpty())
      {
        file.Format(_T("Subject:%s"),subject.GetString());
      }
      file.Write(XString(_T("\n")));
      if(file.Close())
      {
        _flushall();
        return;
      }
    }
  }
  // Cannot write to the central logfile <filename>
  theApp.WideMessageBox(NULL,g_message[MESS_WRITELOG][g_lang] + m_profiles.GetCentralLogfile(),WhoAmI,MB_OK|MB_ICONERROR);
}

void
PostMail::NotifyERPFramework()
{
  if(m_returnValue == 0 || m_returnValue >= 10)
  {
    // We DID sent and email
    if(!m_pingCommand.IsEmpty() && 
       !m_readyCommand.IsEmpty() && 
       !m_reconnectCommand.IsEmpty())
    {
      // We have the commands!
      int result = RunERPCommand(m_pingCommand);
      if(result < 1)
      {
        // Silence: We need to login
        result = RunERPCommand(m_reconnectCommand);
      }
      // Login or ping gotten: Send the email status
      if(result > 0)
      {
        RunERPCommand(m_readyCommand);
      }

      // See if we have chosen another profile
      if((m_returnValue > 10) && !m_profileCommand.IsEmpty())
      {
        m_profileCommand.AppendFormat(_T(" %d"),(m_returnValue - 10));
        RunERPCommand(m_profileCommand);
      }
    }
  }
}

int
PostMail::RunERPCommand(XString p_commando)
{
  XString error;
  XString program;
  return ExecuteProcess(program,p_commando,true,error,SW_HIDE,true);
}

bool
PostMail::CheckJSONConfig()
{
  XString pad = m_profiles.GetExePath() + _T("PostMail.json");
  return _taccess(pad,4) == 0;
}

void
PostMail::DoHelpPage()
{
  XString help =
    _T("PostMail program to send e-mail to the mail recipients\n")
    _T("Written by W.E. Huisman (2006-2025)\n")
    _T("\n")
    _T("Usage:\n")
    _T("POSTMAIL [options] {[parameter-file] | [mail-part]}\n")
    _T("\n")
    _T("Where:\n")
    _T("The parameter file is a text file with parameters AND the intended e-mail.\n")
    _T("See the manual \'PostMail.pdf\' for more details about this text file.\n")
    _T("\n")
    _T("The options can be one or more of the following:\n")
    _T("/N\t\tAll dialogs are in Dutch (N=Nederlands)\n")
    _T("/E\t\tAll dialogs are in English (E=English)\n")
    _T("/F\t\tAll dialogs are in French (F=Français)\n")
    _T("/D\t\tAll dialogs are in German (D=Deutsch)\n")
    _T("/S\t\tProgram started by a non-dedicated server (S=Server)\n")
    _T("/T\t\tProgram started in legacy text mode instead of HTML\n")
    _T("/L\t\tProgram will log all activity to logfile\n")
    _T("/A:\"file\"\t\tAppend loginfo to this absolute \"filename\"\n")
    _T("/X\t\tExtend logging info to Hexadecimal dumping\n")
    _T("/O\t\tShow outbox with already sent e-mails\n")
    _T("/P\t\tChoose a profile from the profilelist\n")
    _T("/P[:<n>]\t\tProgram will choose profile <n> from profilelist\n")
    _T("/P[:<name>]\tProgram will use profile with this name\n")
    _T("/V\t\tProgram is a mailviewer only (V=Viewer)\n")
    _T("/C\t\tShow configuration dialog only (C=Configuration)\n")
    _T("/K\t\tEncrypt a text for passwords or config files\n")
    _T("/U:login\t\tUser login command for the ERP application\n")
    _T("/R:ready\t\tCommand to send when email sent ready\n")
    _T("/Q:ping\t\tQuery a ping command to test connection\n")
    _T("/Z:newprof\tCommand to set chosen profile\n")
    _T("\n")
    _T("The mailpart consists of an ID and one optional parameter per single call\n")
    _T("\n")
    _T("/ID:<n>\t\tID of the mail to sent\n")
    _T("/ID:<n>/CC:name\tCC of the mail with ID <n>\n")
    _T("/ID:<n>/...\tEvery mail parameter\n")
    _T("/ID\t\tEmpty the Mail ID cache and sent all mails\n");

  theApp.WideMessageBox(NULL,help,WhoAmI,MB_OK);
}

void
PostMail::DoHelpFile()
{
  XString file = _T("PostMail_");
  XString directory = m_profiles.GetExePath();

  switch(g_lang)
  {
    case LANGUAGE_NEDERLANDS: file += _T("NL"); break;
    case LANGUAGE_ENGLISH:    file += _T("EN"); break;
    case LANGUAGE_FRANCAIS:   file += _T("FR"); break;
    case LANGUAGE_DEUTSCH:    file += _T("DE"); break;
  }
  XString path = directory + file + _T(".pdf");
  if(_taccess(path,0))
  {
    theApp.WideMessageBox(NULL,g_message[MESS_NOMANUAL][g_lang] + path,WhoAmI,MB_OK|MB_ICONERROR);
    return;
  }
  ShellExecute(NULL,_T("open"),path,_T(""),directory,SW_SHOW);
}

XString
SystemError()
{
  TCHAR buffer[MAX_LINELEN];
  int ErrorCode = GetLastError();

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM     | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                ErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR)buffer,
                MAX_LINELEN,
                NULL);
  XString message;
  message.Format(_T("%d: %s"),ErrorCode,buffer);
  message.TrimRight(_T("\n\r"));
  return message;
}

void
PostMail::CheckRuntimer(XString p_runtimer)
{
  // Getting the name of the runtimer and main version ("PostMail")
  XString check(WhoAmI);
  int pos = check.Find(_T(' '));
  if(pos > 0)
  {
    check  = check.Left(pos);
    check += _T(".exe");
  }
  p_runtimer.MakeLower();
  check.MakeLower();

  // CRC Check on WhoAmI name
  unsigned __int64 crc = 0L;
  for(int ind = 0;ind < check.GetLength(); ++ind)
  {
    crc *= 128;
    crc += (int) check.GetAt(ind);
  }
  crc = 0xFFFFFFFF - crc;

  // Someone tried to tamper with the version information
  if(crc != WHOAMI_CRC)
  {
    // Version information in the program has been tampered with. The program will be closed.
    theApp.WideMessageBox(NULL,g_message[MESS_VERSION_TAMPERED][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    ExitProcess(0xFFFF);
  }

  // Name on the file system does not match the internal name
  if(check.Compare(p_runtimer))
  {
    // The program has been renamed. This is not allowed. The program will be closed.
    theApp.WideMessageBox(NULL,g_message[MESS_RUNTIME_RENAMED][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    ExitProcess(0xFFFF);
  }
}


BOOL 
PostMail::IsUserInteractive()
{
  BOOL bIsUserInteractive = TRUE;

  HWINSTA hWinStation = GetProcessWindowStation();
  if(hWinStation != NULL)
  {
    USEROBJECTFLAGS uof = {0};
    if(GetUserObjectInformation(hWinStation,UOI_FLAGS,&uof,sizeof(USEROBJECTFLAGS),NULL) && ((uof.dwFlags & WSF_VISIBLE) == 0))
    {
      m_server = true;
      bIsUserInteractive = FALSE;
    }
  }
  return bIsUserInteractive;
}
