/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPMessage.cpp
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
#include "SMTPMessage.h"
#include "PostMail.h"
#include "PostMailDlg.h"
#include "Message.h"
#include "Profiles.h"
#include "SMTPConnection.h"
#include "GRAPHConnection.h"
#include "ChooseProfileDlg.h"
#include "Version.h"
#include <IsUnicodeUTF8.h>
#include <Crypto.h>
#include <windns.h>
#include <WinFile.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 6284)

SMTPMessage::SMTPMessage() 
            :m_priority(MAILPRIORITY_NORMAL)
            ,m_showErrors(true)
            ,m_showDialog(true)
            ,m_showNotify(false)
            ,m_mdn(false)
            ,m_progress(true)
            ,m_dsn(NOTIFY_FAILURE)
            ,m_delete(true)
            ,m_editSubject(false)
            ,m_editBody(false)
            ,m_sendNotify(false)
            ,m_doLogin(false)
            ,m_isRTF(false)
            ,m_debugNoPWDCrypt(false)
            ,m_timeout(DEFAULT_TIMEOUT * CLOCKS_PER_SEC)
            ,m_writeChanges(false)
            ,m_port(DEFAULT_SMTP_PORT)
            ,m_encoding(Encoding::UTF8)
{
}

SMTPMessage::~SMTPMessage()
{
  Reset();
}

void
SMTPMessage::Reset()
{
  m_ToRecipients.RemoveAll();
  m_CCRecipients.RemoveAll();
  m_BCCRecipients.RemoveAll();
  for(int i=0; i < m_attachments.GetSize(); ++i)
  {
    SMTPAttachment* attach = m_attachments.GetAt(i);
    delete attach;
  }
  m_attachments.RemoveAll();
  m_doLogin = false;
  m_body.Empty();
  m_mailID.Empty();
  m_htmlBody.Empty();
  m_password.Empty();
  m_passwordScram.Empty();
}

// Check the host against all profiles
void
SMTPMessage::CheckHost()
{
  Profiles& profiles = theApp.GetProfiles();
  if(m_host.IsEmpty())
  {
    // No host set, use the first standard mail server host
    m_host = profiles.GetDefaultMailServer();
  }
  else
  {
    // Check that the host is in the settings
    profiles.CheckMailServer(m_host);
  }
}

void
SMTPMessage::SetPriority(int priority)
{
  if(priority >= 0 && priority <= 2)
  {
    m_priority = priority;
    LogPriorityCode();
  }
}

XString
SMTPMessage::GetPriority()
{
  switch(m_priority)
  {
    case 0: return _T("low");
    case 2: return _T("high");
    case 1: // Fall through
    default:return _T("normal");
  }
}

int SMTPMessage::GetNumberOfRecipients(RECIPIENT_TYPE RecipientType) const
{
  int nSize = 0;
  switch (RecipientType)
  {
    case TO:  nSize = (int) m_ToRecipients.GetSize();  break;
    case CC:  nSize = (int) m_CCRecipients.GetSize();  break;
    case BCC: nSize = (int) m_BCCRecipients.GetSize(); break;
  }

  return nSize;
}

int SMTPMessage::AddRecipient(SMTPAddress& recipient, RECIPIENT_TYPE RecipientType)
{
  int nIndex = -1;
  switch (RecipientType)
  {
    case TO:  nIndex = (int) m_ToRecipients.Add(recipient);  break;
    case CC:  nIndex = (int) m_CCRecipients.Add(recipient);  break;
    case BCC: nIndex = (int) m_BCCRecipients.Add(recipient); break;
    case FROM:m_from = recipient; break;
  }

  return nIndex;
}

SMTPAddress
SMTPMessage::GetRecipient(int nIndex, RECIPIENT_TYPE RecipientType) const
{
  SMTPAddress address;

  switch (RecipientType)
  {
    case TO:  if(m_ToRecipients.GetCount() > nIndex)
              {
                address = m_ToRecipients.GetAt(nIndex);  
              }
              break;
    case CC:  if(m_CCRecipients.GetCount() > nIndex)
              {
                address = m_CCRecipients.GetAt(nIndex);
              }
              break;
    case BCC: if(m_BCCRecipients.GetCount() > nIndex)
              {
                address = m_BCCRecipients.GetAt(nIndex);
              }
              break;
  }
  return address;
}

SMTPAddress*
SMTPMessage::GetRecipient(XString& p_adres,RECIPIENT_TYPE p_recipientType /*=TO*/)
{
  SMTPAddress* address = NULL;

  if(p_recipientType == TO)
  {
    for(int ind = 0;ind < m_ToRecipients.GetSize(); ++ind)
    {
      if(m_ToRecipients[ind].GetRegularFormat().CompareNoCase(p_adres) == 0)
      {
        address = &(m_ToRecipients.GetAt(ind));
        break;
      }
    }
  }
  if(p_recipientType == CC)
  {
    for(int ind = 0;ind < m_CCRecipients.GetSize(); ++ind)
    {
      if(m_CCRecipients[ind].GetRegularFormat().CompareNoCase(p_adres) == 0)
      {
        address = &(m_CCRecipients.GetAt(ind));
        break;
      }
    }
  }
  if(p_recipientType == BCC)
  {
    for(int ind = 0;ind < m_BCCRecipients.GetSize(); ++ind)
    {
      if(m_BCCRecipients[ind].GetRegularFormat().CompareNoCase(p_adres) == 0)
      {
        address = &(m_BCCRecipients.GetAt(ind));
        break;
      }
    }
  }
  return address;
}

void
SMTPMessage::DeleteRecipient(XString& p_adres,RECIPIENT_TYPE p_recipientType/*=TO*/)
{
  if(p_recipientType == TO)
  {
    for(int ind = 0;ind < m_ToRecipients.GetSize(); ++ind)
    {
      if(m_ToRecipients[ind].GetRegularFormat().CompareNoCase(p_adres) == 0)
      {
        m_ToRecipients.RemoveAt(ind);
        break;
      }
    }
  }
  if(p_recipientType == CC)
  {
    for(int ind = 0;ind < m_CCRecipients.GetSize(); ++ind)
    {
      if(m_CCRecipients[ind].GetRegularFormat().CompareNoCase(p_adres) == 0)
      {
        m_CCRecipients.RemoveAt(ind);
        break;
      }
    }
  }
  if(p_recipientType == BCC)
  {
    for(int ind = 0;ind < m_BCCRecipients.GetSize(); ++ind)
    {
      if(m_BCCRecipients[ind].GetRegularFormat().CompareNoCase(p_adres) == 0)
      {
        m_BCCRecipients.RemoveAt(ind);
        break;
      }
    }
  }
}

int 
SMTPMessage::AddAttachment(SMTPAttachment* pAttachment)
{
  if(!pAttachment->GetFilename().GetLength())
  {
    // MESS_NOFILENAME An empty filename for an attachment is not allowed
    theApp.Log(3,g_message[MESS_NOFILENAME][g_lang]);
  }
  return (int) m_attachments.Add(pAttachment);
}

void 
SMTPMessage::RemoveAttachment(int nIndex)
{
  if(nIndex >= 0 && nIndex < m_attachments.GetCount())
  {
    SMTPAttachment* attach = m_attachments[nIndex];
    delete attach;
    m_attachments.RemoveAt(nIndex);
  }
}

SMTPAttachment* 
SMTPMessage::GetAttachment(int nIndex) const
{
  if(nIndex >= 0 && nIndex < m_attachments.GetCount())
  {
     return m_attachments[nIndex];
  }
  return NULL;
}

int 
SMTPMessage::GetNumberOfAttachments() const
{
  return (int) m_attachments.GetSize();
}

XString 
SMTPMessage::GetHeader(XString p_contentType,bool p_doRTF)
{
  //Form the Timezone info which will form part of the Date header
  int nTZBias = 0;
  TIME_ZONE_INFORMATION tzi;
  ZeroMemory(&tzi, sizeof(TIME_ZONE_INFORMATION));
  
  switch(GetTimeZoneInformation(&tzi))
  {
    case TIME_ZONE_ID_DAYLIGHT: nTZBias = tzi.Bias + tzi.DaylightBias;
                                break;
    case TIME_ZONE_ID_STANDARD: nTZBias = tzi.Bias;
                                break;
    case TIME_ZONE_ID_UNKNOWN:  // Fall through
    default:                    nTZBias = 0;
                                break;
  }
  XString sTZBias;
  sTZBias.Format(_T("%+.2d%.2d"), -nTZBias/60, nTZBias%60);

  //Create the "Date:" part of the header
  CTime now(CTime::GetCurrentTime());
  XString sDate(now.Format(_T("%a, %d %b %Y %H:%M:%S ")));
  sDate += sTZBias;

  //Create the "To:" part of the header
  XString sTo;
  for (int i=0; i<GetNumberOfRecipients(TO); i++)
  {
    SMTPAddress recipient = GetRecipient(i, TO);
    if (i)
    {
      sTo += _T(",");
    }
    sTo += recipient.GetRegularFormat();
  }

  //Create the "Cc:" part of the header
  XString sCc;
  for (int i=0; i<GetNumberOfRecipients(CC); i++)
  {
    SMTPAddress recipient = GetRecipient(i, CC);
    if (i)
    {
      sCc += _T(",");
    }
    sCc += recipient.GetRegularFormat();
  }

  //Stick everything together
  XString sBuf;
  XString sEbuf;

  sBuf.Format(_T("From: %s\r\n")
              _T("To: %s\r\n")
             ,m_from.GetRegularFormat().GetString()
             ,sTo.GetString());
  if (sCc.GetLength())
  {
    sEbuf.Format(_T("Cc: %s\r\n"),sCc.GetString());
    sBuf += sEbuf;
  }

  sEbuf.Format(_T("Subject: %s\r\n")
               _T("Date: %s\r\n")
               _T("X-Mailer: %s\r\n")
               _T("Importance: %s\r\n"), 
                m_subject.GetString(),
                sDate.GetString(),
                WhoAmI,
                GetPriority().GetString());
  sBuf += sEbuf;

  if(m_mdn)
  {
    sEbuf.Format(_T("Disposition-Notification-To: %s\r\n"),m_from.m_emailAddress.GetString());
    sBuf += sEbuf;
  }

  //Add the optional fields if attachments are included
  if (m_attachments.GetSize() || p_doRTF)
  {
    sBuf += _T("MIME-Version: 1.0\r\n")
            _T("Content-Type: multipart/mixed; boundary=\"#BOUNDARY#\"\r\n");
  }
  else
  {
    // avoid long textual message being automatically converted by the server:
    sEbuf.Format(_T("MIME-Version: 1.0\r\n")
                 _T("Content-type: %s; charset=\"%s\"\r\n")
                ,p_contentType.GetString()
                ,CodepageToCharset((int)m_encoding).GetString());
    sBuf += sEbuf;
  }
  sBuf += _T("\r\n");

  //Return the result
  return sBuf;
}

void 
SMTPMessage::AddBody(XString& sBody)
{
  //Fix the case of a single dot on a line in the message body
  sBody.Replace(_T("\n."),_T("\n ."));
  m_body += sBody;
}

void 
SMTPMessage::AddHtmlBody(const XString& p_htmlBody)
{
  m_htmlBody += p_htmlBody;
}

BOOL SMTPMessage::AddMultipleRecipients(const XString& p_recipients, RECIPIENT_TYPE p_recipientType)
{
  if(!p_recipients.GetLength())
  {
    //MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(3,g_message[MESS_NOADDRESS][g_lang]);
  }	
  //Loop through the whole string, adding recipients as they are encountered
  int length = p_recipients.GetLength();
  TCHAR* buf = new TCHAR[length + 1];	// Allocate a work area (don't touch parameter itself)
  _tcscpy_s(buf,length+1,p_recipients);
  for (int pos=0, start=0; pos<=length; pos++)
  {
    //Valid separators between addresses are ',' or ';'
    if ((buf[pos] == _T(',')) || (buf[pos] == _T(';')) || (buf[pos] == 0))
    {
      buf[pos] = 0;	//Redundant when at the end of string, but who cares.
      XString sTemp(&buf[start]);

      // Now divide the substring into friendly names and e-mail addresses
      SMTPAddress To;
      int nMark = sTemp.Find(_T('<'));
      if (nMark >= 0)
      {
        To.m_friendlyName = sTemp.Left(nMark);
        int nMark2 = sTemp.Find(_T('>'));
        if (nMark2 < nMark)
        {
          //An invalid string was sent in, fail the call
          // MESS_MAILADDRESS: "An error occurred while parsing the recipients string"
          delete[] buf;
          SetLastError(ERROR_INVALID_DATA);
          theApp.Log(3,g_message[MESS_MAILADDRESS][g_lang]);
          return FALSE;
        }
        // End of mark at closing bracket or end of string
        // nMark2 > -1 ? nMark2 = nMark2 : nMark2 = sTemp.GetLength() - 1
        if(nMark2 < 0)
        {
          nMark2 = sTemp.GetLength() - 1;
        }
        To.m_emailAddress = sTemp.Mid(nMark + 1, nMark2 - (nMark + 1));
      }
      else
      {
        To.m_emailAddress = sTemp;
        To.m_friendlyName = _T("");
      }

      //Finally add the new recipient to the array of recipients
      To.m_emailAddress.TrimRight();
      To.m_emailAddress.TrimLeft();
      To.m_friendlyName.TrimRight();
      To.m_friendlyName.TrimLeft();
      if (To.m_emailAddress.GetLength())
      {
        AddRecipient(To, p_recipientType);
      }
      //Move on to the next position
      start = pos + 1;
    }
  }
  delete[] buf;
  return TRUE;
}

bool
SMTPMessage::ReReadFile()
{
  Reset();
  return ReadFromFile(m_filename);
}

// READ AN EXTERNAL DEFINITION FILE
// This constitutes a mail message
bool             
SMTPMessage::ReadFromFile(XString& p_fileName,bool p_total /*=false*/)
{
  USES_CONVERSION;

  bool    bodySeen = false;
  bool    htmlSeen = false;
  XString body;
  XString htmlBody;
  XString separator(_T('-'),72);
  XString buffer;

  try
  {
    WinFile file(p_fileName);
    if(file.Open(winfile_read | FFlag::open_shared_read | FFlag::open_trans_text) == false)
    {
      XString errMsg;
      if(GetLastError() == ERROR_FILE_NOT_FOUND)
      {
        errMsg = theApp.WindowsError();
        if(p_total == false)
        {
          return true;
        }
      }
      // MESS_MAILTXT "Cannot read the mail message file: " 
      XString errorMsg = g_message[MESS_MAILTXT][g_lang] + p_fileName;
      theApp.Log(3,errorMsg);
      theApp.Log(3,errMsg);
      return false;
    }
    // Remember our file
    m_filename = p_fileName;

    theApp.Log(LOGLEVEL_TERSE,m_filename);
    theApp.Log(LOGLEVEL_TERSE,separator);

    while(file.Read(buffer))
    {
      if(buffer.GetAt(0) == _T('#'))
      {
        continue;
      }
      // Some applications add multiple <CR> and/or <CR><LF> characters
      // to eache line, through incorrect text-translation modes
      buffer.TrimRight(_T("\n"));
      buffer.TrimRight(_T("\r"));

      // Log the line in the log buffer
      theApp.Log(LOGLEVEL_TERSE,buffer,false);
      theApp.Log(LOGLEVEL_TERSE,_T("<<"));

      // Scan for end of parameters and reading of a body of text
      if (buffer.Left(6).CompareNoCase(_T("<body>")) == 0 && !htmlSeen)
      {
        bodySeen = true;
        htmlSeen = false;
        continue;
      }
      else if (buffer.Left(10).CompareNoCase(_T("<htmlbody>")) == 0)
      {
        htmlSeen = true;
        bodySeen = false;
        continue;
      }

      // If already reading BODY/HTMLBODY append to that
      if(bodySeen)
      {
        // Add to the body
        AddParameter(_T("body"), buffer);

        // Check for RTF
        if(!m_isRTF)
        {
          if(buffer.Left(6).Compare(_T("{\\rtf1")) == 0)
          {
            m_isRTF = true;
          }
        }
        // Check for HTML
        if (!htmlSeen)
        {
          continue;
        }
      }
      else if (htmlSeen)
      {
        if (buffer.Left(11).CompareNoCase(_T("</htmlbody>")) == 0)
        {
          // OK, continue with other parts (body, parameters)
          htmlSeen = false;
        }
        else
        {
          // Add to the HTML body
          AddParameter(_T("htmlbody"), buffer);
        }
        continue;
      }

      // Process the parameters
      int pos = buffer.Find(_T(':'));
      if(pos > 0)
      {
        // <CODE>:<line> parameter
        XString parameter = buffer.Left(pos);
        XString valueline = buffer.Mid(pos + 1);

        AddParameter(parameter,valueline);
      }
      else
      {
        // Boolean parameter without value
        AddParameter(buffer,_T(""));
      }
    }
    // Remember our encoding
    m_encoding = file.GetEncoding();

    if(m_encoding == Encoding::EN_ACP)
    {
      if(IsTextUnicodeUTF8((LPCTSTR)m_body.GetString(),m_body.GetLength()))
      {
        m_encoding = Encoding::UTF8;
      }
      if(IsTextUnicodeUTF8((LPCTSTR)m_htmlBody.GetString(),m_htmlBody.GetLength()))
      {
        m_encoding = Encoding::UTF8;
      }
    }

    file.Close();
    theApp.Log(LOGLEVEL_TERSE,separator);
  }
  catch(StdException& exp)
  {
    XString errorMsg = g_message[MESS_MAILTXT][g_lang] + p_fileName;
    errorMsg += _T("\n") + exp.GetErrorMessage();
    theApp.Log(3,errorMsg);
    theApp.Log(3,errorMsg);
  }
  return true;
}

bool
SMTPMessage::CheckAfterRead(bool p_viewer,bool p_graph)
{
  // Record viewer mode overrides show/delete status
  if(p_viewer)
  {
    m_showDialog = true;
    m_delete     = false;
  }

  // Add yourself to the BCC list to show who send the mail
  if(m_showNotify)
  {
    AddMultipleRecipients(m_from.m_emailAddress,BCC);
  }

  // Check profile (host, sender etc)
  if(!p_graph)
  {
    CheckProfile();
  }
  // If we want to be able to login, we will need a
  // user id and a password at the least.
  if(!CheckLogin())
  {
    return false;
  }

  // Further consistency checks
  if(!ConsistencyCheck(p_graph))
  {
    return false;
  }

  // Now list it to the logfile to see what we did read in.
  ListMessageToLog();

  return true;
}

bool
SMTPMessage::CheckLogin()
{
  if(m_doLogin)
  {
    if(m_mailID.IsEmpty() ||
      (m_password.IsEmpty() && m_passwordScram.IsEmpty()))
    {
      // MESS_LOGIN_IDPWD "Needs a login: but no mail-id and password given"
      theApp.Log(3,g_message[MESS_LOGIN_IDPWD][g_lang]);
      return false;
    }
    if(m_debugNoPWDCrypt == false && m_password.IsEmpty() && !m_passwordScram.IsEmpty())
    {
      m_password = Unscramble(m_passwordScram);
    }
  }
  else
  {
    // Dispose of critical security info as quickly as possible
    m_mailID   = _T("");
    m_password = _T("");
  }
  return true;
}

void
SMTPMessage::CheckProfile()
{
  // Check on host/sender
  if(m_host.IsEmpty() || m_from.m_emailAddress.IsEmpty() || !m_profileName.IsEmpty())
  {
    // Work to do
    Profiles& prof  = theApp.GetProfiles();
    int      gekozen = prof.GetChosenProfile();
    Profile* profile = prof.GetProfile(gekozen);

    if(profile == NULL && !m_profileName.IsEmpty())
    {
      profile = prof.GetProfile(m_profileName);
    }
    if(profile == NULL)
    {
      if(prof.HasACompleteProfile())
      {
        profile = prof.GetProfile(0);
      }
      else
      {
        // No profile or multiple profiles
        // See if there is a standard profile
        profile = prof.GetStandardProfile();

        if(profile == NULL && theApp.GetServer() == false)
        {
          // No profile and no standard profile
          // Make one, or choose one
          ChooseProfileDlg dlg(NULL,prof);
          if(dlg.DoModal() == IDOK)
          {
            gekozen = prof.GetChosenProfile();
            profile = prof.GetProfile(gekozen);
            if(!profile)
            {
              // No profile chosen
              profile = prof.GetStandardProfile();
            }
          }
        }
      }
    }
    if(profile)
    {
      // Try to copy as much as possible from the profile
      if(m_from.m_emailAddress.IsEmpty())
      {
        m_from.m_emailAddress  = profile->m_emailAddress;
      }
      if(m_host.IsEmpty())
      {
        m_host = profile->m_smtpServer;
      }
      if(m_doLogin == false && profile->m_useLogin)
      {
        m_doLogin = true;
      }
      if(m_doLogin)
      {
        if(m_mailID.IsEmpty() || m_password.IsEmpty())
        {
          m_mailID   = profile->m_loginUser;
          m_password = profile->m_password;
        }
      }
    }
  }

}

bool
SMTPMessage::ConsistencyCheck(bool p_graph)
{
  // MULTIPLE CONSISTENCY CHECKS
  // 
  // 1) Host must be given
  if(p_graph == false && m_host.IsEmpty() && theApp.GetProfiles().GetDefaultMailServer().IsEmpty())
  {
    // MESS_NOHOST: "No mail-host given for the mail to sent (HOST:)"
    theApp.Log(3,g_message[MESS_NOHOST][g_lang]);
    return false;
  }
  // 2) From address must be given
  if(m_from.m_emailAddress.IsEmpty())
  {
    // MESS_NOSENDER "No sender for the mail (FROM:)"
    theApp.Log(3,g_message[MESS_NOSENDER][g_lang]);
    return false;
  }
  // 3) At least one delivery address
  int numTo = 0;
  for (int i=0; i<GetNumberOfRecipients(TO); i++)
  {
    SMTPAddress recipient = GetRecipient(i, TO);
    if(!recipient.m_emailAddress.IsEmpty())
    {
      ++numTo;
    }
  }
  if(!numTo)
  {
    // MESS_NORCPT "No minimum of 1 addressed person (TO:)"
    theApp.Log(3,g_message[MESS_NORCPT][g_lang]);
    return false;
  }
  // 4) A subject must be given. 
  // Technically not a requirement, but it looks odd in a mail list!
  if(m_subject.IsEmpty())
  {
    // MESS_NOSUBJECT "NO subject in the mail! (SUBJECT:)"
    theApp.Log(3,g_message[MESS_NOSUBJECT][g_lang]);
    return false;
  }
  // 5) A body must be given.
  // Technically not a requirement, but it looks odd in a mail list!
  if(m_body.IsEmpty() && m_htmlBody.IsEmpty())
  {
    // MESS_NOBODY "De mail contains no message! (<BODY>)"
    theApp.Log(3,g_message[MESS_NOBODY][g_lang]);
    return false;
  }
  return true;
}

void
SMTPMessage::ListMessageToLog()
{
  XString line;
  line = _T("INTERNAL STATE OF THE MESSAGE AS READ:");
  theApp.Log(LOGLEVEL_TERSE,line);
  theApp.Log(LOGLEVEL_TERSE,_T("   "));

  // Log the mail host
  LogHost();

  // Log the mail ID
  LogMailID();

  // Log the password
  LogPassword();

  // Log who send the email
  LogFrom();

  // Log all the recipients
  LogRecipients();

  line.Format(_T("Subject: %s"),m_subject.GetString());
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Show dialog: %s"), m_showDialog ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Show progress: %s"), m_progress ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Show errors at end: %s"), m_showErrors ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Show message with send status: %s"), m_sendNotify ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Notify sender in BCC list: %s"), m_showNotify ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Delete after successful send: %s"), m_delete ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);

  // Log the priority code
  LogPriorityCode();
  // Log the notification
  LogNotify();
  // Log m_mdn to logfile
  LogMailDisposition();

  line.Format(_T("Timeout for SMTP protocol: %d miliseconds"),m_timeout);
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Edit subject in dialog: %s"),m_editSubject ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);
  line.Format(_T("Edit message in dialog: %s"),m_editBody ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE,line);

  theApp.Log(LOGLEVEL_TERSE,_T("ATTACHMENTS:"));
  for (int i=0; i<m_attachments.GetSize(); ++i)
  {
    SMTPAttachment* pAttachment = m_attachments.GetAt(i);
    line.Format(_T("File: %s"),pAttachment->GetFilename().GetString());
    theApp.Log(LOGLEVEL_TERSE,line);
  }
  theApp.Log(LOGLEVEL_TERSE,_T("  "));
  theApp.Log(LOGLEVEL_TERSE,_T("THE MESSAGE BODY:"));
  theApp.Log(LOGLEVEL_TERSE,_T("  "));
  theApp.Log(LOGLEVEL_TERSE,m_body);
  if(!m_htmlBody.IsEmpty())
  {
    theApp.Log(LOGLEVEL_TERSE,_T("  "));
    theApp.Log(LOGLEVEL_TERSE,_T("THE HTML VARIANT OF THE MESSAGE:"));
    theApp.Log(LOGLEVEL_TERSE,_T("  "));
    theApp.Log(LOGLEVEL_TERSE,m_htmlBody);
  }
}

// Show the message on screen in an MFC dialog box
bool
SMTPMessage::DisplayMessage()
{
  if(!m_showDialog || theApp.GetServer() ||   
     theApp.GetODataConfig().GetHasBeenRead())
  {
    // No show dialog requested, so always send the message
    return true;
  }

  bool viewer = theApp.GetViewer();
  // START THE DIALOG
  PostMailDlg dlg(this
                 ,viewer
                 ,m_editSubject && !viewer
                 ,m_editBody    && !viewer
                 ,theApp.GetProfiles());

  // Display the mail message
  theApp.m_pMainWnd = &dlg;
  INT_PTR response = dlg.DoModal();
  theApp.m_pMainWnd = nullptr;

  if(response == IDOK)
  {
    // Process the changes in subject/body?
    ProcessChanges(dlg.GetSubject(),dlg.GetBody());
    // OK to send this message
    return true;
  }
  return false;
}

// Save new Subject and Message Body
// Old style PostMail 3 & 5 "filename.chng" changes
void
SMTPMessage::ProcessChanges(XString p_subject,XString p_body)
{
  if(p_subject != m_subject)
  {
    m_subject = p_subject;
    theApp.Log(LOGLEVEL_TERSE,_T("SUBJECT EDITTED IN THE DIALOG:"));
    theApp.Log(LOGLEVEL_TERSE,m_subject);
    if(m_writeChanges)
    {
      theApp.WriteChanges(_T("SUBJECT"),m_subject);
    }
  }
  if(p_body != m_body)
  {
    m_body = p_body;
    theApp.Log(LOGLEVEL_TERSE,_T("BODY EDITTED IN THE DIALOG:"));
    theApp.Log(LOGLEVEL_TERSE,m_body);
    if(m_writeChanges)
    {
      theApp.WriteChanges(_T("BODY"),m_body);
    }
  }
}

// Copy the essential information from the profile
void
SMTPMessage::UseProfile(Profile* p_profile)
{
  // Connection information  
  m_host      = p_profile->m_smtpServer;
  m_port      = p_profile->m_smtpPort;
  m_mailID    = p_profile->m_loginUser;
  m_password  = p_profile->m_password;
  m_doLogin   = p_profile->m_useLogin;
  // Mail address information
  m_from.m_emailAddress = p_profile->m_emailAddress;
  AddMultipleRecipients(p_profile->m_bccAddress,BCC);
}

void
SMTPMessage::FindHostFromSender()
{
  int pos = m_from.m_emailAddress.Find(_T('@'));
  if(pos >= 0)
  {
    XString maildomain = m_from.m_emailAddress.Mid(pos + 1);
    if(!maildomain.IsEmpty())
    {
      PDNS_RECORD pdr = NULL;
      DNS_STATUS result = DnsQuery(maildomain,DNS_TYPE_MX,DNS_QUERY_STANDARD,0,&pdr,0);
      if(result == 0 && pdr)
      {
        m_host = pdr->Data.MX.pNameExchange;
      }
      DnsRecordListFree(pdr,DnsFreeRecordList);
    }
  }
}

bool
SMTPMessage::SendMessage()
{
  // Ensure we have a sender
  if(m_from.m_emailAddress.IsEmpty() || m_host.IsEmpty())
  {
    Profile* profile = theApp.GetProfiles().GetStandardProfile();
    if(profile)
    {
      if((m_from.m_emailAddress.IsEmpty() && !profile->m_emailAddress.IsEmpty()) ||
         (m_host.IsEmpty() && !profile->m_smtpServer.IsEmpty()))
      {
        UseProfile(profile);
      }
    }
  }

  // Pointer to relay dialog
  RelayDlg* relay = nullptr;

  // See if we are a server process
  bool server = theApp.GetServer();

  // Building a RelayDlg to show the relaying process..
  if(!server && GetProgress())
  {
    // Make a relay dialog and tell we are connecting
    relay = new RelayDlg(NULL,true);
    relay->DoTheInit();
    relay->SetMax(CalculateTransmissionParts());
    relay->SetText(_T("Encoding all attachments before transmitting"));
  }
  bool result = false;
  if(theApp.GetODataConfig().GetHasBeenRead())
  {
    result = SendMessageByGRAPH(relay);
  }
  else
  {
    result = SendMessageBySMTP(relay);
  }

  // Clear out the relay dialog
  if(relay)
  {
    delete relay;
  }

  return result;
}

bool
SMTPMessage::SendMessageBySMTP(RelayDlg* p_relay)
{
  bool result = false;

  // Ensure we have a SMTP host server
  // Last resort is the MX record of the DNS
  if(m_host.IsEmpty())
  {
    FindHostFromSender();
  }

  // Read-in all attachments and encode in Base64 for the mail server
  if(EncodeAllAttachments(p_relay) == false)
  {
    return false;
  }

  // Create the SMTP connection
  // And set the timeout on the connection
  SMTPConnection smtp;
  smtp.SetTimeout(GetTimeout());

  // Connect to the server
  if(p_relay)
  {
    p_relay->SetText(_T("Connecting to: ") + GetHost());
  }
  if(smtp.Connect(GetHost(),GetMailID(),GetPassword(),GetDoLogin(),GetPort()))
  {
    // Test size of the message
    if(smtp.TestMessageTooBig(*this))
    {
      // The message (including attachments) is too big for the mailserver
      theApp.Log(LOGLEVEL_ERROR,g_message[MESS_TOO_BIG][g_lang]);
      smtp.Disconnect();
      return false;
    }

    // Send the message
    if(!smtp.SendMessage(*this,p_relay))
    {
      XString sResponse = smtp.GetLastCommandResponse();
      // MESS_FAILSEND: "Failed to send the SMTP message"
      theApp.Log(3,g_message[MESS_FAILSEND][g_lang]);
      theApp.Log(3,sResponse.GetString());
    }
    else
    {
      if(p_relay)
      {
        p_relay->SetAtEnd();
        p_relay->SetText(_T("Disconnecting"));
        p_relay->Increment();
      }
      // Disconnect from the server. All went well.
      smtp.Disconnect();
      result = true;
    }
  }
  else
  {
    XString sResponse = smtp.GetLastCommandResponse();
    XString errMsg    = theApp.WindowsError();

    // MESS_FAILCONNECT: ": Failed to connect to SMTP server: ";
    XString error = XString(WhoAmI) + g_message[MESS_FAILCONNECT][g_lang] + GetHost();
    theApp.Log(3,error);
    theApp.Log(3,sResponse);
    theApp.Log(3,errMsg);
  }
  return result;
}

bool
SMTPMessage::SendMessageByGRAPH(RelayDlg* p_relay)
{
  bool result = false;
  GRAPHConnection graph(theApp.GetODataConfig());

  // Read-in all attachments and encode in Base64 for the mailserver
  if(ReadInAllAttachments(p_relay) == false)
  {
    XString errMsg = theApp.WindowsError();
    // MESS_OPENFILE: ": Failed to open file to be attached: %s.\nIt probably doesn't exist, or is still open in a viewer.
    XString error = XString(WhoAmI) + g_message[MESS_OPENFILE][g_lang];
    error.Replace(_T("%s"),_T("(every attachment)"));
    theApp.Log(LOGLEVEL_ERROR,error);
    theApp.Log(LOGLEVEL_ERROR,errMsg);
    return false;
  }

  if(graph.TestMessageTooBig(*this))
  {
    // Message (including attachments) is too big for the mailserver
    theApp.Log(LOGLEVEL_ERROR,g_message[MESS_TOO_BIG][g_lang]);
    return false;
  }

  if(graph.Connect(p_relay))
  {
    // Send the message
    if(!graph.SendMessage(*this,p_relay))
    {
      // MESS_FAILSEND: "Failed to send the mail message"
      theApp.Log(3,g_message[MESS_FAILSEND][g_lang]);
    }
    else
    {
      if(p_relay)
      {
        p_relay->SetAtEnd();
        p_relay->SetText(_T("Disconnecting"));
        p_relay->Increment();
      }
      // Disconnect from the server. All went well.
      result = true;
    }

  }
  else
  {
    XString errMsg = theApp.WindowsError();
    // MESS_FAILCONNECT: ": Failed to connect to SMTP server: ";
    XString error = XString(WhoAmI) + g_message[MESS_FAILCONNECT][g_lang] + GetHost();
    theApp.Log(3,error);
    theApp.Log(3,errMsg);
  }

  // Always successful
  graph.DisConnect();
  return result;
}

// Size of the gouge in the RelayDlg
int
SMTPMessage::CalculateTransmissionParts()
{
  // CONNECT / MAILFROM / MESSAGE / DISCONNECT
  int parts = 4; 
 
  parts += GetNumberOfRecipients(TO);
  parts += GetNumberOfRecipients(CC);
  parts += GetNumberOfRecipients(BCC);
  parts += (int)m_attachments.GetSize() * 2;

  return parts;
}

///////////////////////////////////////////////////////////////////////////////
//
// Encrypts a password
//
XString
SMTPMessage::Scramble(const XString& p_password) const
{
  if(m_password.Find(':') >= 0)
  {
    theApp.Log(LOGLEVEL_TERSE,_T("Passwords may not contain the ':' character!\n"));
    return _T("");
  }
  Crypto crypt;
  CString encrypted;
  CString reverse(p_password);
  reverse.MakeReverse();
  XString value = reverse + _T(":");
  value += p_password;
  return crypt.Encryption(value,POSTMAIL_PASSWORD);
}

///////////////////////////////////////////////////////////////////////////////
//
// Decodes a scrambled password
//
XString 
SMTPMessage::Unscramble(const XString& p_password) const
{
  int pos = 0;
  Crypto crypt;
  XString passPhrase(POSTMAIL_PASSWORD);

  XString decrypted = crypt.Decryption(p_password,passPhrase);

  pos = decrypted.Find(_T(':'));
  if(pos > 0)
  {
    return decrypted.Mid(pos + 1);
  }
  return _T("");
}

void
SMTPMessage::SetSender(XString& p_email)
{
  m_from.m_emailAddress = p_email;
  m_from.m_friendlyName.Empty();
  LogFrom();
}

void
SMTPMessage::LogMailDisposition()
{
  XString line;
  line.Format(_T("Mail disposition notification: %s"), m_mdn ? _T("yes") : _T("no"));
  theApp.Log(LOGLEVEL_TERSE, line);
}

void
SMTPMessage::LogPriorityCode()
{
  XString line;
  line.Format(_T("Importance: %s"), m_priority == 2 ? _T("high") : (m_priority == 1 ? _T("low") : _T("normal")));
  theApp.Log(LOGLEVEL_TERSE, line);
}

void
SMTPMessage::LogNotify()
{
  XString line;
  line = _T("Delivery status notification: ");
  if (m_dsn == NOTIFY_NEVER)
  {
    line += _T("Never");
  }
  else
  {
    line += (m_dsn & NOTIFY_FULL) ? _T("Full") : _T("Headers");
    if (m_dsn & NOTIFY_FAILURE)
    {
      line += _T(", ");
      line += _T("Failure");
    }
    if (m_dsn & NOTIFY_SUCCESS)
    {
      line += _T(", ");
      line += _T("Success");
    }
    if (m_dsn & NOTIFY_DELAY)
    {
      line += _T(", ");
      line += _T("delay");
    }
  }
theApp.Log(LOGLEVEL_TERSE,line);
}

void
SMTPMessage::LogFrom()
{
  XString line;
  line.Format(_T("FROM: %s"),m_from.GetRegularFormat().GetString());
  theApp.Log(LOGLEVEL_TERSE,line);
}

void
SMTPMessage::LogHost()
{
  XString line;
  line.Format(_T("Mailserver is host  : %s"),m_host.GetString());
  theApp.Log(LOGLEVEL_TERSE,line);
}

void
SMTPMessage::LogPort()
{
  XString line;
  line.Format(_T("Mailserver talks on port : %d"),m_port);
  theApp.Log(LOGLEVEL_TERSE,line);
}

void
SMTPMessage::LogMailID()
{
  XString line;
  line.Format(_T("Login on mailhost as: %s"),m_mailID.GetString());
  theApp.Log(LOGLEVEL_TERSE,line);
}

void
SMTPMessage::LogPassword()
{
  XString line;
#ifdef DEBUG
  line.Format(_T("Used authentication : %s"),m_debugNoPWDCrypt ? m_password.GetString() : m_passwordScram.GetString());
#else
  line.Format(_T("Used authentication : %s"),m_passwordScram.GetString());
#endif
  theApp.Log(LOGLEVEL_TERSE,line);
#ifdef DEBUG
  if(m_debugNoPWDCrypt)
  {
    theApp.Log(LOGLEVEL_TERSE,_T("WARNING: NO PASSWORD ENCRYPTION USED!"));
  }
#endif
  theApp.Log(LOGLEVEL_TERSE,_T("   "));
}

void
SMTPMessage::LogRecipients()
{
  XString line;

  // Make the list of recipients (TO)
  for(int i = 0; i < GetNumberOfRecipients(TO); i++)
  {
    SMTPAddress recipient = GetRecipient(i,TO);
    line.Format(_T("TO: %s"),recipient.GetRegularFormat().GetString());
    theApp.Log(LOGLEVEL_TERSE,line);
  }
  // Make the list of recipients (CC)
  for(int i = 0; i < GetNumberOfRecipients(CC); i++)
  {
    SMTPAddress recipient = GetRecipient(i,CC);
    line.Format(_T("CC: %s"),recipient.GetRegularFormat().GetString());
    theApp.Log(LOGLEVEL_TERSE,line);
  }
  // Make the list of recipients (BCC)
  for(int i = 0; i < GetNumberOfRecipients(BCC); i++)
  {
    SMTPAddress recipient = GetRecipient(i,BCC);
    line.Format(_T("BCC: %s"),recipient.GetRegularFormat().GetString());
    theApp.Log(LOGLEVEL_TERSE,line);
  }
}

bool
SMTPMessage::AddParameter(XString p_parameter,XString p_text)
{
  bool notFound(false);

  // Quick scan for most appearing lines
  if(p_parameter.CompareNoCase(_T("body")) == 0)
  {
    p_text += _T("\r\n");
    AddBody(p_text);
    return true;
  }
  if(p_parameter.CompareNoCase(_T("htmlbody")) == 0)
  {
    p_text += _T("\r\n");
    AddHtmlBody(p_text);
    return true;
  }

  // Trimmed variant of the parameter
  XString trimmed(p_text);
  trimmed.Trim();

  // Find command on this line
  PMCType datatype(PMCType::PMC_NoType);
  CommandNR cmd = FindMailCommand(p_parameter,datatype);

  // Prepare boolean
  bool boolean = false;
  if(datatype == PMCType::PMC_Boolean)
  {
    int fb = FindMailBoolean(trimmed);
    if(fb == 1)
    {
      boolean = true;
    }
    if(fb < 0)
    {
      return false;
    }
  }

  // Prepare notify / priority / integer / language / strings
  int notify   = 0;
  int priority = 0;
  int integer  = 0;
  int language = 0;

  switch(datatype)
  {
    case PMCType::PMC_Notify:   notify = FindDeliverStatus(trimmed);
                                if(notify < 0)
                                {
                                  return false;
                                }
                                break;
    case PMCType::PMC_Priority: priority = FindMailPriority(trimmed);
                                if(priority < 0)
                                {
                                  return false;
                                }
                                break;
    case PMCType::PMC_Integer:  integer = _ttoi(trimmed);
                                break;
    case PMCType::PMC_Language: language = FindMailLanguage(trimmed);
                                if(language < 0)
                                {
                                  return false;
                                }
                                break;
    case PMCType::PMC_SString:  [[fallthrough]];
    case PMCType::PMC_MString:  [[fallthrough]];
    case PMCType::PMC_SHA256:   if(trimmed.IsEmpty())
                                {
                                  // Should have at least a string value
                                  return false;
                                }
                                break;
    case PMCType::PMC_NoType:   [[fallthrough]];
    default:                    break;
  }

  // Per command setting of a parameter
  switch(cmd)
  {
    case CommandNR::COM_HOST:       m_host = trimmed;
                                    ProcessHost();
                                    break;
    case CommandNR::COM_MAILID:     m_mailID = trimmed;
                                    break;
    case CommandNR::COM_PROFILE:    m_profileName = trimmed;
                                    ProcessProfile();
                                    break;
    case CommandNR::COM_SUBJECT:    m_subject = trimmed;
                                    break;
    case CommandNR::COM_FROM:       AddMultipleRecipients(trimmed,FROM);
                                    break;
    case CommandNR::COM_TO:         AddMultipleRecipients(trimmed,TO);
                                    break;
    case CommandNR::COM_CC:         AddMultipleRecipients(trimmed,CC);
                                    break;
    case CommandNR::COM_BCC:        AddMultipleRecipients(trimmed,BCC);
                                    break;
    case CommandNR::COM_NOPWDCRYPT: m_debugNoPWDCrypt = true;
                                    break;
    case CommandNR::COM_LOGIN:      m_doLogin = boolean;
                                    break;
    case CommandNR::COM_ERRORS:     m_showErrors = boolean;
                                    break;
    case CommandNR::COM_DIALOG:     m_showDialog = boolean;
                                    break;
    case CommandNR::COM_NOTIFY:     m_showNotify = boolean;
                                    break;
    case CommandNR::COM_READCONF:   m_mdn = boolean;
                                    break;
    case CommandNR::COM_PROGRESS:   m_progress = boolean;
                                    break;
    case CommandNR::COM_DELETE:     m_delete = boolean;
                                    break;
    case CommandNR::COM_EDITSUBJECT:m_editSubject = boolean;
                                    break;
    case CommandNR::COM_EDITBODY:   m_editBody = boolean;
                                    break;
    case CommandNR::COM_SENDCONF:   m_sendNotify = boolean;
                                    break;
    case CommandNR::COM_WRITE:      m_writeChanges = boolean;
                                    break;
    case CommandNR::COM_DELIVER:    m_dsn = notify;
                                    break;
    case CommandNR::COM_IMPORT:     m_priority = priority;
                                    break;
    case CommandNR::COM_LANGUAGE:   g_lang = language;
                                    break;
    case CommandNR::COM_LOGLEVEL:   if(integer >= LOGLEVEL_TERSE && integer <= LOGLEVEL_TRACE)
                                    {
                                      theApp.OpenLogfile(integer);
                                    }
                                    break;
    case CommandNR::COM_TIMEOUT:    if(integer >= 2 && integer <= 20)
                                    {
                                      // Timeout is in milliseconds
                                      m_timeout = integer * CLOCKS_PER_SEC;
                                    }
                                    break;
    case CommandNR::COM_PASSWORD:   if(m_debugNoPWDCrypt)
                                    {
                                      m_password = trimmed;
                                    }
                                    else
                                    {
                                      m_passwordScram = trimmed;
                                    }
                                    break;
    case CommandNR::COM_ATTACH:     { // It's an attachment to the mail
                                      auto attachment = new SMTPAttachment();
                                      if(attachment->Attachment(trimmed))
                                      {
                                        AddAttachment(attachment);
                                      }
                                      else
                                      {
                                        delete attachment;
                                        return false;
                                      }
                                    }
                                    break;
    default:                        notFound = true;
                                    break;
  }

  if(notFound)
  {
    // PARAMETER HAS NOT BEEN RECOGNIZED
    XString test = p_parameter;
    if(!p_text.IsEmpty())
    {
      test += XString(_T(":")) + p_text;
    }
    test.Trim();
    // EMPTY LINE IS EXPLICITLY ALLOWED
    if(!test.IsEmpty())
    {
      // MESS_UNKNOWNLINE Unknown line type in message file
      theApp.Log(3,g_message[MESS_UNKNOWNLINE][g_lang]);
      theApp.Log(3,test);
      return false;
    }
  }
  return true;
}

void
SMTPMessage::ProcessHost()
{
  if(m_host.Find(_T(':')) >= 0)
  {
    int pos = m_host.Find(_T(':'));
    XString port = m_host.Mid(pos + 1);
    m_port = _ttoi(port);
    m_host = m_host.Left(pos);
  }
}

void
SMTPMessage::ProcessProfile()
{
  if(!m_profileName.IsEmpty())
  {
    Profile* profile = theApp.GetProfiles().GetProfile(m_profileName);
    if(profile)
    {
      UseProfile(profile);
    }
  }
}

//////////////////////////////////////////////////////////////////////////
//
// Write Back to mail.txt file
//
//////////////////////////////////////////////////////////////////////////

bool
SMTPMessage::SaveToFile(XString p_fileName)
{
  bool result = false;
  CStdioFile file;

  try
  {
    // Open the file
    if(file.Open(p_fileName,CFile::modeCreate | CFile::typeText | CFile::modeWrite | CFile::shareDenyWrite) == FALSE)
    {
      return false;
    }

    SaveParamIfSet(file,_T("HOST"),             m_host);
    SaveParamIfSet(file,_T("LOGIN"),            m_doLogin ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("MAILID"),           m_mailID);
    SaveRecipient (file,_T("FROM"),             m_from);
    SaveRecipients(file, TO,                    m_ToRecipients);
    SaveRecipients(file, CC,                    m_CCRecipients);
    SaveRecipients(file, BCC,                   m_BCCRecipients);
    SaveParamIfSet(file,_T("PROFILE"),          m_profileName);
    SaveParamIfSet(file,_T("SUBJECT"),          m_subject);
    SaveParamIfSet(file,_T("ERRORS"),           m_showErrors   ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("DIALOG"),           m_showDialog   ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("READCONFIRMATION"), m_mdn          ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("PROGRESS"),         m_progress     ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("DELETE"),           m_delete       ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("EDITSUBJECT"),      m_editSubject  ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("EDITBODY"),         m_editBody     ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("NOTIFY"),           m_showNotify   ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("SENDCONFIRMATION"), m_showNotify   ? _T("yes") : _T("no"));
    SaveParamIfSet(file,_T("WRITECHANGES"),     m_writeChanges ? _T("yes") : _T("no"));

    if(m_dsn != NOTIFY_FAILURE)
    {
      XString notify;
      switch(m_dsn)
      {
        case NOTIFY_FAILURE: notify = _T("failure"); break;
        case NOTIFY_SUCCESS: notify = _T("success"); break;
        case NOTIFY_DELAY:   notify = _T("delayed"); break;
        case NOTIFY_NEVER:   notify = _T("never");   break;
        case NOTIFY_HEADER:  notify = _T("header");  break;
        case NOTIFY_FULL:    notify = _T("full");    break;
      }
      SaveParamIfSet(file,_T("DELIVERSTATUS"),notify);
    }

    // Save the password
    XString wachtwoord;
    if(m_debugNoPWDCrypt)
    {
      file.WriteString(_T("NOWPDCRYPT\n"));  
      wachtwoord = m_password;
    }
    else
    {
      wachtwoord = m_passwordScram;
    }
    SaveParamIfSet(file,_T("PASSWORD"),wachtwoord);

    // Save the attachments
    for(int ind = 0;ind < m_attachments.GetCount();++ind)
    {
      SMTPAttachment* attach = m_attachments.GetAt(ind);
      SaveParamIfSet(file,_T("ATTACH"),attach->GetFilename());
    }

    // Mail priority
    if(m_priority != MAILPRIORITY_NORMAL)
    {
      SaveParamIfSet(file,_T("IMPORTANCE"),m_priority == MAILPRIORITY_LOW ? _T("low") : _T("high"));
    }
    else
    {
      SaveParamIfSet(file,_T("IMPORTANCE"),_T("normal"));
    }
    // Language
    if(g_lang >= LANGUAGE_NEDERLANDS && g_lang <= LANGUAGE_DEUTSCH)
    {
      XString taalCode;
      switch(g_lang)
      {
        case LANGUAGE_NEDERLANDS: taalCode = _T("Nederlands");break;
        case LANGUAGE_ENGLISH:    taalCode = _T("English");   break;
        case LANGUAGE_FRANCAIS:   taalCode = _T("Francais");  break;
        case LANGUAGE_DEUTSCH:    taalCode = _T("Deutsch");   break;
      }
      SaveParamIfSet(file,_T("LANGUAGE"),taalCode);
    }
    // Timeout
    if(m_timeout != DEFAULT_TIMEOUT)
    {
      XString timeout;
      timeout.Format(_T("%d"),m_timeout /  CLOCKS_PER_SEC);
      SaveParamIfSet(file,_T("TIMEOUT"),timeout);
    }
    // Log level
    if(theApp.GetLoglevel())
    {
      XString loglevel;
      loglevel.Format(_T("%d"),theApp.GetLoglevel());
      SaveParamIfSet(file,_T("LOGLEVEL"),loglevel);
    }

    // Add the bodies
    if(!m_htmlBody.IsEmpty())
    {
      XString toWrite = m_htmlBody;
      toWrite.Remove(_T('\r'));
      file.WriteString(_T("<HTMLBODY>\n"));
      file.WriteString(toWrite);
      file.WriteString(_T("</HTMLBODY>\n"));
    }
    if(!m_body.IsEmpty())
    {
      XString toWrite = m_body;
      toWrite.Remove(_T('\r'));
      file.WriteString(_T("<BODY>\n"));
      file.WriteString(toWrite);
    }
    result = true;
  }
  catch(...)
  {
    // Something wrong in writing the message
    theApp.WideMessageBox(NULL
                         ,g_message[MESS_NOT_WRITTEN][g_lang]
                         ,WhoAmI
                         ,MB_OK | MB_SETFOREGROUND);
  }
  file.Close();
  return result;
}

void
SMTPMessage::SaveParamIfSet(CStdioFile& p_file,XString p_parameter,XString p_value)
{
  // Check that the parameter was set
  if(p_value.IsEmpty())
  {
    return;
  }
  XString line;
  line.Format(_T("%s:%s\n"),p_parameter.GetString(),p_value.GetString());
  p_file.WriteString(line);
}

void
SMTPMessage::SaveRecipients(CStdioFile& p_file,RECIPIENT_TYPE p_type,Recipients& p_recipients)
{
  // Quick check for emptiness
  if(p_recipients.GetCount() == 0)
  {
    return;
  }

  // Get the address type
  XString type;
  switch(p_type)
  {
    case TO:  type = _T("TO");  break;
    case CC:  type = _T("CC");  break;
    case BCC: type = _T("BCC"); break;
  }
  // Print the array of email addresses
  for(int ind = 0;ind < p_recipients.GetCount(); ++ind)
  {
    SMTPAddress& address = p_recipients.GetAt(ind);
    SaveRecipient(p_file,type,address);
  }
}

void
SMTPMessage::SaveRecipient(CStdioFile& p_file,XString p_type,SMTPAddress& p_address)
{
  XString smtpAddress;

  if(p_address.m_friendlyName.IsEmpty())
  {
    smtpAddress = p_address.m_emailAddress + _T("\n");
  }
  else
  {
    smtpAddress.Format(_T("%s <%s>\n")
                      ,p_address.m_friendlyName.GetString()
                      ,p_address.m_emailAddress.GetString());
  }
  p_type += _T(":");
  p_file.WriteString(p_type + smtpAddress);
}

//////////////////////////////////////////////////////////////////////////
//
// Check all attachments for reading
//
//////////////////////////////////////////////////////////////////////////

bool
SMTPMessage::CheckAttachments() const
{
  bool result = true;

  for(int index = 0;index < m_attachments.GetSize();++index)
  {
    SMTPAttachment* attach = m_attachments.GetAt(index);
    WinFile file(attach->GetFilename());
    if(file.Exists() == false)
    {
      // Cannot open an attachment. Stop here after a warning
      XString message = g_message[MESS_ATTACHMENT_IN_USE][g_lang];
      message += attach->GetFilename();
      theApp.WideMessageBox(NULL,message,WhoAmI,MB_OK|MB_ICONWARNING);
      result = false;
      break;
    }
  }
  return result;
}

bool
SMTPMessage::EncodeAllAttachments(RelayDlg* p_relay)
{
  for(int index = 0; index < m_attachments.GetSize(); ++index)
  {
    SMTPAttachment* attach = m_attachments.GetAt(index);

    if(p_relay)
    {
      XString text(_T("Encoding: "));
      text += attach->GetTitle();
      p_relay->SetText(text);
    }
    // Trigger the encoding by requesting the buffer
    if(attach->GetEncodedBuffer() == nullptr)
    {
      return false;
    }
    if(p_relay)
    {
      p_relay->Increment();
    }
  }
  return true;
}

bool
SMTPMessage::ReadInAllAttachments(RelayDlg* p_relay)
{
  for(int index = 0; index < m_attachments.GetSize(); ++index)
  {
    SMTPAttachment* attach = m_attachments.GetAt(index);

    if(p_relay)
    {
      XString text(_T("Reading: "));
      text += attach->GetTitle();
      p_relay->SetText(text);
    }
    // Trigger the encoding by requesting the buffer
    if(attach->ReadInBuffer() == nullptr)
    {
      XString nofile;
      // MESS_OPENFILE: ": Failed to open file to be attached: %s.\nIt probably doesn't exist, or is still open in a viewer.
      XString error = g_message[MESS_OPENFILE][g_lang];
      nofile.Format(error,attach->GetFilename().GetString());
      theApp.Log(LOGLEVEL_ERROR,nofile);
      return false;
    }
    if(p_relay)
    {
      p_relay->Increment();
    }
  }
  return true;
}
