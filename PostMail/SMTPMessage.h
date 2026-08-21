/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPMessage.h
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
#include "SMTPAddress.h"
#include "SMTPAttachment.h"
#include "Profiles.h"
#include "RelayDlg.h"
#include "Commands.h"
#pragma once

#define MIN_TIMEOUT 2
#define MAX_TIMEOUT 20;


////////////////// Forward declaration
class SMTPConnection;

typedef int NOTIFY_TYPE;

typedef enum RECIPIENT_TYPE 
{ 
  TO
 ,CC
 ,BCC
 ,FROM 
}
RECIPIENT_TYPE;

// Container types
typedef CArray<SMTPAddress,     SMTPAddress&>     Recipients;
typedef CArray<SMTPAttachment*, SMTPAttachment*&> Attachments;

//Encapsulation of an SMTP message
class SMTPMessage
{
public:
  // Constructors / Destructors
  SMTPMessage();
 ~SMTPMessage();

  // Primary commands
  void             Reset();
  bool             DisplayMessage();
  bool             SendMessage();
  bool             ReadFromFile(XString& p_fileName,bool p_total = false);
  bool             SaveToFile(XString p_fileName);
  bool             ReReadFile();

  //Recipient support
  int              GetNumberOfRecipients(RECIPIENT_TYPE p_recipientType = TO) const;
  int              AddRecipient(SMTPAddress& p_recipient,RECIPIENT_TYPE p_recipientType = TO);
  SMTPAddress      GetRecipient   (int      p_index,RECIPIENT_TYPE p_recipientType = TO) const;
  SMTPAddress*     GetRecipient   (XString& p_adres,RECIPIENT_TYPE p_recipientType = TO);
  void             DeleteRecipient(XString& p_adres,RECIPIENT_TYPE p_recipientType = TO);
  void             SetSender(XString& p_email);

  //Attachment support
  int              GetNumberOfAttachments() const;
  int              AddAttachment(SMTPAttachment* pAttachment);
  void             RemoveAttachment(int nIndex);
  SMTPAttachment*  GetAttachment(int nIndex) const;
  bool             CheckAttachments() const;
  bool             EncodeAllAttachments(RelayDlg* p_relay);
  bool             ReadInAllAttachments(RelayDlg* p_relay);

  // Miscellaneous methods
  void             AddBody(XString& sBody);
  void             AddHtmlBody(const XString& p_htmlBody);
  BOOL             AddMultipleRecipients(const XString& sRecipients, RECIPIENT_TYPE RecipientType);
  int              CalculateTransmissionParts();
  void             LogRecipients();
  void             CheckHost();
  bool             AddParameter(XString p_parameter,XString p_text);
  bool             CheckAfterRead(bool p_viewer,bool p_graph);
  void             ResetHTMLBodyAfterEdit();
  void             UseProfile(Profile* p_profile);

  XString          GetHeader(XString p_contentType = _T("plain"),bool p_doRTF = false);
  XString          GetFilename();
  bool             GetShowErrors();
  bool             GetShowDialog();
  XString          GetHost();
  int              GetPort();
  XString          GetMailID();
  XString          GetPassword();
  bool             GetProgress();
  int              GetTimeout();
  bool             GetDelete();
  NOTIFY_TYPE      GetNotify();
  bool             GetSendNotify();
  SMTPAddress*     GetSender();
  XString          GetSubject();
  XString          GetBody();
  XString          GetBodyHTML();
  bool             GetDisposition();
  bool             GetDoLogin();
  bool             GetIsRTF();
  XString          GetErrors();
  Encoding         GetEncoding();

  // Priority
  void             SetPriority(int priority);
  XString          GetPriority();
  int              GetPriorityCode();

  // Setters
  void             SetHost(XString p_host);
  void             SetPort(int p_port);
  void             SetDoLogin(bool p_login);
  void             SetMailID(XString p_mailid);
  void             SetPassword(XString p_password);
  void             SetDisposition(bool p_disposition);
  void             SetNotify(NOTIFY_TYPE p_notify);
  void             SetEditSubject(bool p_edit);
  void             SetEditBody(bool p_edit);
  void             SetEncoding(Encoding p_encoding);

  //Data Members
	SMTPAddress      m_from;
	XString          m_subject;

protected:
  bool    SendMessageBySMTP (RelayDlg* p_relay);
  bool    SendMessageByGRAPH(RelayDlg* p_relay);

  void    ListMessageToLog();
  void    FindHostFromSender();
  XString Scramble  (const XString& p_password) const;
  XString Unscramble(const XString& p_password) const;
  bool    ConsistencyCheck(bool p_graph);
  void    CheckProfile();
  bool    CheckLogin();
  void    ProcessChanges(XString p_subject,XString p_body);
  void    SaveParamIfSet(CStdioFile& p_file,XString p_parameter,XString p_value);
  void    SaveRecipients(CStdioFile& p_file,RECIPIENT_TYPE p_type,Recipients& p_recipients);
  void    SaveRecipient (CStdioFile& p_file,XString p_type,SMTPAddress& p_address);
  void    ProcessHost();
  void    ProcessProfile();
    
  // Logging sub-functions
  void    LogMailDisposition();
  void    LogPriorityCode();
  void    LogNotify();
  void    LogFrom();
  void    LogHost();
  void    LogPort();
  void    LogMailID();
  void    LogPassword();

  XString m_filename;       // TXT file containing this message
  XString m_host;           // Host of my  SMTP server
  int     m_port;           // Port of the STMP server
  XString m_body;           // Text of the message body
  XString m_htmlBody;       // HTML Version of the message body
  bool    m_isRTF;          // Text is in RTF format
  int     m_priority;       // low/normal/high importance
  bool    m_showErrors;     // Show errors (if any)
  bool    m_showDialog;     // Show message in a dialog
  bool    m_showNotify;     // Notify end-user that mail has been sent
  bool    m_mdn;            // Mail disposition notification
  bool    m_progress;       // Show progress meter
  bool    m_doLogin;        // Use mail-id/password
  bool    m_writeChanges;   // Old style file.chng file with changes only
  XString m_profileName;    // Name mail profile
  XString m_password;       // For administrators with paranoia
  XString m_mailID;         // For administrators with paranoia
  XString m_passwordScram;  // For administrators with more paranoia
  NOTIFY_TYPE m_dsn;        // Delivery Status Notification
  int     m_timeout;        // Timeout for the protocol
  bool    m_delete;         // Delete definition file after send
  bool    m_editSubject;
  bool    m_editBody;
  bool    m_sendNotify;     // Notification message of send-status
  bool    m_debugNoPWDCrypt;
  // Lists
  Recipients  m_ToRecipients;
  Recipients  m_CCRecipients;
  Recipients  m_BCCRecipients;
  Attachments m_attachments;
  // Errors
  XString  m_errors;
  Encoding m_encoding;

  friend class SMTPConnection;
  friend class GRAPHConnection;
};

inline bool
SMTPMessage::GetShowErrors()
{
  return m_showErrors;
}

inline bool
SMTPMessage::GetShowDialog()
{
  return m_showDialog;
}

inline XString
SMTPMessage::GetHost()
{
  return m_host;
}

inline int
SMTPMessage::GetPort()
{
  return m_port;
}

inline NOTIFY_TYPE
SMTPMessage::GetNotify()
{
  return m_dsn;
}

inline bool
SMTPMessage::GetProgress()
{
  return m_progress;
}

inline int
SMTPMessage::GetTimeout()
{
  return m_timeout;
}

inline bool
SMTPMessage::GetDelete()
{
  return m_delete;
}

inline bool
SMTPMessage::GetSendNotify()
{
  return m_sendNotify;
}

inline XString
SMTPMessage::GetMailID()
{
  return m_mailID;
}

inline XString 
SMTPMessage::GetPassword()
{
  return m_password;
}

inline SMTPAddress*
SMTPMessage::GetSender()
{
  return &m_from;
}

inline XString
SMTPMessage::GetSubject()
{
  return m_subject;
}

inline XString
SMTPMessage::GetBody()
{
  return m_body;
}

inline bool
SMTPMessage::GetDisposition()
{
  return m_mdn;
}

inline void
SMTPMessage::SetDisposition(bool p_disposition)
{
  m_mdn = p_disposition;
  LogMailDisposition();
}

inline int
SMTPMessage::GetPriorityCode()
{
  return m_priority;
}

inline void
SMTPMessage::SetHost(XString p_host)
{
  m_host = p_host;
  LogHost();
}

inline void
SMTPMessage::SetPort(int p_port)
{
  m_port = p_port;
  LogPort();
}

inline bool
SMTPMessage::GetDoLogin()
{
  return m_doLogin;
}

inline void
SMTPMessage::SetDoLogin(bool p_login)
{
  m_doLogin = p_login;
}

inline void
SMTPMessage::SetMailID(XString p_mailid)
{
  m_mailID = p_mailid;
  LogMailID();
}

inline void
SMTPMessage::SetPassword(XString p_password)
{
  m_password = p_password;
  LogPassword();
}

inline void
SMTPMessage::SetNotify(NOTIFY_TYPE p_notify)
{
  m_dsn = p_notify;
  LogNotify();
}

inline void
SMTPMessage::SetEditSubject(bool p_edit)
{
  m_editSubject = p_edit;
}

inline void
SMTPMessage::SetEditBody(bool p_edit)
{
  m_editBody = p_edit;
}

inline bool
SMTPMessage::GetIsRTF()
{
  return m_isRTF;
}

inline XString
SMTPMessage::GetFilename()
{
  return m_filename;
}

inline XString
SMTPMessage::GetBodyHTML()
{
  return m_htmlBody;
}

inline void
SMTPMessage::ResetHTMLBodyAfterEdit()
{
  m_htmlBody.Empty();
}

inline XString
SMTPMessage::GetErrors()
{
  return m_errors;
}

inline void
SMTPMessage::SetEncoding(Encoding p_encoding)
{
  m_encoding = p_encoding;
}

inline Encoding
SMTPMessage::GetEncoding()
{
  return m_encoding;
}
