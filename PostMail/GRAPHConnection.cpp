/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: GRAPHConnection.cpp
//
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#include "stdafx.h"
#include "GRAPHConnection.h"
#include "RelayDlg.h"
#include "PostMail.h"
#include "Message.h"
#include "Version.h"
#include "Office365.h"
#include "ADSI.h"
#include <LogAnalysis.h>
#include <CrackURL.h>

#define SECURITY_WIN32
#include <sspi.h>
#include <secext.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

GRAPHConnection::GRAPHConnection(ODataConfig& p_config)
                :m_config(p_config)
{
  FindEmailAdressOfUPN();
}

GRAPHConnection::~GRAPHConnection()
{
}

// Connect to the server
bool
GRAPHConnection::Connect(RelayDlg* p_relay)
{
  if(p_relay)
  {
    p_relay->SetText(_T("Connecting to: ") + m_config.GetType());
  }

  XString scope = m_config.GetAppScope();
  if(scope.IsEmpty())
  {
    scope = scope_ms_graph;
  }
  XString url = m_cache.CreateTokenURL(m_config.GetTokenServer().GetString(), m_config.GetAzureTenant().GetString());
  m_session   = m_cache.CreateClientCredentialsGrant(url
                                                    ,m_config.GetAppIdentity().GetString()
                                                    ,m_config.GetAppSecret().GetString()
                                                    ,scope.GetString());

  // Connect to the client
  m_client.SetOAuth2Cache(&m_cache);
  m_client.SetOAuth2Session(m_session);

  if(theApp.GetLoglevel() >= LOGLEVEL_TRACE)
  {
    // Create Logfile
    XString filename;
    LogAnalysis* logfile = LogAnalysis::CreateLogfile(_T("PostMail via MS-Graph"));
    logfile->SetLogLevel(HLL_TRACEDUMP);
    filename.GetEnvironmentVariable(_T("TMP"));
    filename += _T("\\LogfileGraphEmail.txt");
    logfile->SetLogFilename(filename);
    logfile->SetLogRotation(true);

    // TRANSFER
    m_client.SetLogging(logfile,true);
    m_client.SetLogLevel(HLL_TRACEDUMP);
  }

  return m_session > 0;
}

// Vergeet de OAuth2 sessie
bool
GRAPHConnection::DisConnect()
{
  m_cache.EndSession(m_session);
  m_client.SetOAuth2Cache(nullptr);
  m_client.SetOAuth2Session(m_session = 0);
  return true;
}

bool
GRAPHConnection::TestMessageTooBig(SMTPMessage& p_message)
{
  // Add all sizes together
  m_totalSize = 0;

  // Add all attachment sizes
  for(int i = 0; i < p_message.m_attachments.GetSize(); i++)
  {
    m_totalSize += p_message.m_attachments.GetAt(i)->GetEncodedSize();
  }
  // All body text sizes
  m_totalSize += p_message.GetBody().GetLength();
  m_totalSize += p_message.GetBodyHTML().GetLength();
  // Space for headers and multipart dividers
  m_totalSize += (10 * 1024); // General headers

  // If total size bigger than allowed max message size
  // We cannot begin to send the message: The mailserver will never accept it!
  return (m_totalSize > OFFICE365_MAX_MESSAGE_SIZE);
}

bool 
GRAPHConnection::SendMessage(SMTPMessage& p_message,RelayDlg* p_relay)
{
  XString text;
  bool result = false;
  m_sender = p_message.GetSender()->m_emailAddress;

  if(m_sender.IsEmpty())
  {
    // If no "FROM" given, try to use the robot account
    m_sender = m_config.GetRobotUsesAccount();
    if(m_sender.IsEmpty())
    {
      // If still no sender, use the current process user's UPN
      m_sender = m_userPrincipalName;
    }
  }

  // Override
  if(theApp.GetServer() == false && p_message.GetShowDialog())
  {
    // Must use current users UPN to be able to see the mail and change it
    m_sender = m_userPrincipalName;
  }

  m_sender = CrackedURL::EncodeURLChars(m_sender.GetString());

  if(m_userPrincipalName.IsEmpty())
  {
    XString melding(_T("Niet ingelogd op een Active Directory of Entra-ID. U kunt geen e-mail versturen naar Office-365!"));
    theApp.Log(LOGLEVEL_ERROR,melding);
    theApp.WideMessageBox(nullptr,melding,_T(WHOAMI),MB_OK);
    return false;
  }

  // Create the JSON message for "/me/messages" (Concept map)
  JSONMessage mail;
  XString url;
  url.Format(OFFICE365_MAIL_CREATE,m_sender.GetString());
  mail.SetContentType(_T("application/json"));
  mail.SetURL(url);
  mail.SetVerb(_T("POST"));

  // Make the JSON
  mail.GetValue().SetDatatype(JsonType::JDT_object);
  JSONobject& object = mail.GetValue().GetObject();

  // Filling in the message
  AddFrom       (object,m_sender);
  AddImportance (object,p_message);
  AddDelivery   (object,p_message);
  AddReadReceipt(object,p_message);
  AddRecipients (object,p_message.m_ToRecipients, _T("toRecipients"));
  AddRecipients (object,p_message.m_CCRecipients, _T("ccRecipients"));
  AddRecipients (object,p_message.m_BCCRecipients,_T("bccRecipients"));
  AddSubject    (object,p_message);
  AddBody       (object,p_message);

  // Show the sender delegation
  if(p_relay)
  {
    p_relay->Increment();
    p_relay->SetText(_T("Mail from: ") + m_sender);
  }

  mail.SetWhitespace(true);
  XString msg = mail.GetJsonMessage();

  // CREATING THE CONCEPT AND SENDING IT
  result = m_client.Send(&mail);
  if(result)
  {
    // Message was sent
    XString messageID = FindMailID(mail);
    if(!messageID.IsEmpty())
    {
      // Add the attachments to the concept message
      if(p_message.GetNumberOfAttachments())
      {
        AddAttachments(messageID,p_message,p_relay);
      }
      if(p_relay)
      {
        p_relay->Increment();
        p_relay->SetText(_T("Sending concept to recipients"));
      }

      if(p_message.GetShowDialog() == false || theApp.GetServer())
      {
        // sending to "/users/{UPN}/message/{id}/send"
        url.Format(OFFICE365_MAIL_SEND,m_sender.GetString(),messageID.GetString());
        HTTPMessage send(HTTPCommand::http_post,url);

        result = m_client.Send(&send);
        if(result && m_client.GetStatus() == HTTP_STATUS_ACCEPTED)
        {
          // All went well.
          result = true;
        }
        else
        {
          HandleError(_T("Mailconcept not sent to recipients!"));
        }
      }
      else if(!theApp.GetServer())
      {
        // We now have a concept in the folder 'concepts'
        result = true;

        // Show the user the concept in the default browser
        if(m_config.GetAppBrowser().IsEmpty() ||
           m_config.GetAppBrowser().CompareNoCase(_T("browse")) == 0)
        {
          XString webLink = ComposeMail(messageID);
          if(ShellExecute(nullptr,_T("open"),webLink,_T(""),_T(""),SW_SHOW) <= (HINSTANCE)32)
          {
            HandleError(_T("Kon geen concept e-mail openen in uw standaard browser."));
            return false;
          }
        }
        else if(m_config.GetAppBrowser().CompareNoCase(_T("msg")) == 0)
        {
          theApp.WideMessageBox(nullptr
                               ,_T("De e-mail is aangemaakt in uw map met 'Concepten'\n")
                                _T("U kunt de e-mail vanuit deze map nu verder bewerken en verzenden.\n")
                               ,APPLICATION
                               ,MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND);
        }
      }
      else
      {
        // Server leaves a concept in the email mailbox in the folder 'concepts'
        result = true;
      }
    }
  }
  else
  {
    // Failed to create concept message
    HandleError(_T("Kon geen concept e-mail aanmaken in de concepten map."));
  }

  if(!m_errors.IsEmpty())
  {
    p_message.m_errors = m_errors;
    p_message.m_showErrors = true;
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//
// PRIVATE
//
//////////////////////////////////////////////////////////////////////////

void
GRAPHConnection::FindEmailAdressOfUPN()
{
  // Request default email through the ADSI service
  m_userPrincipalName = GetSender();
}

void
GRAPHConnection::DetachOauth2()
{
  m_client.SetOAuth2Cache(nullptr);
  m_client.SetOAuth2Session(0);
}

void
GRAPHConnection::RestoreOAuth2()
{
  m_client.SetOAuth2Cache(&m_cache);
  m_client.SetOAuth2Session(m_session);
}

XString
GRAPHConnection::FindMailID(JSONMessage& p_msg)
{
  XString id;
  JSONvalue* value = p_msg.FindValue(_T("id"));
  if(value)
  {
    id = value->GetString();
  }
  // getting the id
  if(id.IsEmpty())
  {
    HandleError(_T("Office-365 did not return a message ID for the concept e-mail."));
  }
  return id;
}

XString
GRAPHConnection::ComposeMail(const XString& p_mesageID)
{
  XString compose;
  compose = OFFICE365_MAIL_COMPOSE + p_mesageID;
  if(compose.Right(2) == _T("=="))
  {
    compose = compose.TrimRight(_T('='));
    compose += _T("%3D%3D");
  }
  else if(compose.Right(1) == _T("="))
  {
    compose = compose.TrimRight(_T('='));
    compose += _T("%3D");
  }
  return CrackedURL::EncodeURLChars(compose).GetString();
}

void
GRAPHConnection::HandleError(const XString& p_melding)
{
  // Showing the error message
  XString error;
  error.Format(_T("Fout bij het afhandelen van Office-365 integratie. Status [%d]\n")
               _T("Resultaat: %s")
               ,m_client.GetStatus(),p_melding.GetString());
  theApp.WideMessageBox(nullptr,error,_T(WHOAMI),MB_OK | MB_ICONERROR);

  if(!m_errors.IsEmpty())
  {
    m_errors += _T("\n");
  }
  m_errors += error;

  // Showing the response body in the log
  BYTE* response = nullptr;
  unsigned length = 0;
  m_client.GetResponse(response,length);
#ifdef _UNICODE
  XString body;
  bool foundBOM(false);
  TryConvertNarrowString(response,length,_T(""),body,foundBOM);
#else
  XString body((const char*)response);
#endif
  JSONMessage json(body);
  json.SetWhitespace(true);
  body = json.GetJsonMessage();
  theApp.Log(LOGLEVEL_ERROR,body.GetString());
}

void
GRAPHConnection::AddFrom(JSONobject& p_object,const XString& p_emailAddress)
{
  JSONpair recip;
  recip.m_name = _T("from");
  recip.m_value.SetDatatype(JsonType::JDT_object);

  SMTPAddress from;
  from.m_emailAddress = p_emailAddress;
  if(from.m_emailAddress.IsEmpty())
  {
    SetRobotAccount(from);
  }
  AddRecipient(recip.m_value,from);
  p_object.push_back(recip);
}

void
GRAPHConnection::AddSubject(JSONobject& p_object,SMTPMessage& p_message)
{
  JSONpair subject;
  subject.m_name = _T("subject");
  subject.m_value.SetValue(p_message.GetSubject());

  p_object.push_back(subject);
}

void
GRAPHConnection::AddBody(JSONobject& p_object,SMTPMessage& p_message)
{
  JSONpair body;
  body.m_name = _T("body");
  body.m_value.SetDatatype(JsonType::JDT_object);
  JSONobject& list = body.m_value.GetObject();

  JSONpair type;
  JSONpair content;
  type.m_name = _T("contentType");
  content.m_name = _T("content");

  XString contentType;

  if(!p_message.GetBodyHTML().IsEmpty())
  {
    contentType = _T("HTML");
    content.m_value.SetValue(p_message.GetBodyHTML());
  }
  else
  {
    XString text = p_message.GetBody();
    if(text.Left(6).CompareNoCase(_T("<HTML>")) == 0)
    {
      contentType = _T("HTML");
    }
    else
    {
      contentType = _T("TEXT");
    }
    content.m_value.SetValue(text);
  }
  type.m_value.SetValue(contentType);

  // Put in body
  list.push_back(type);
  list.push_back(content);

  // Put in message
  p_object.push_back(body);
}

void
GRAPHConnection::AddImportance(JSONobject& p_object,SMTPMessage& p_message)
{
  JSONpair importance;
  importance.m_name = _T("importance");
  importance.m_value.SetValue(p_message.GetPriority());

  p_object.push_back(importance);
}

// We assume that OFFICE-365 does not need a NOTIFY_FAILURE
// because this happens anyway, and thus only for success/delay a notification is needed.
void
GRAPHConnection::AddDelivery(JSONobject& p_object,SMTPMessage& p_message)
{
  JSONpair importance;
  importance.m_name = _T("isDeliveryReceiptRequested");
  importance.m_value.SetValue(p_message.GetNotify() >= NOTIFY_SUCCESS ? JsonConst::JSON_TRUE : JsonConst::JSON_FALSE);

  p_object.push_back(importance);
}

void
GRAPHConnection::AddReadReceipt(JSONobject& p_object,SMTPMessage& p_message)
{
  JSONpair importance;
  importance.m_name = _T("isReadReceiptRequested");
  importance.m_value.SetValue(p_message.GetDisposition() ? JsonConst::JSON_TRUE : JsonConst::JSON_FALSE);

  p_object.push_back(importance);
}

void
GRAPHConnection::AddRecipients(JSONobject& p_object,Recipients& p_recipients,const XString& p_type)
{
  // Check if something to do
  if(p_recipients.GetCount() == 0)
  {
    return;
  }

  JSONpair recip;
  recip.m_name = p_type;
  recip.m_value.SetDatatype(JsonType::JDT_array);

  for(int ind = 0;ind < p_recipients.GetCount(); ++ind)
  {
    JSONvalue total;
    total.SetDatatype(JsonType::JDT_object);

    AddRecipient(total,p_recipients[ind]);
    recip.m_value.GetArray().push_back(total);
  }

  p_object.push_back(recip);
}

void
GRAPHConnection::AddRecipient(JSONvalue& p_value,SMTPAddress& p_address)
{
  JSONpair adr;
  adr.m_name = _T("emailAddress");
  adr.m_value.SetDatatype(JsonType::JDT_object);

  JSONpair email;
  email.m_name = _T("address");
  email.m_value.SetValue(p_address.m_emailAddress);
  adr.m_value.GetObject().push_back(email);

  if(!p_address.m_friendlyName.IsEmpty())
  {
    JSONpair alias;
    alias.m_name = _T("name");
    alias.m_value.SetValue(p_address.m_friendlyName);
    adr.m_value.GetObject().push_back(alias);
  }

  p_value.GetObject().push_back(adr);
}

void
GRAPHConnection::SetRobotAccount(SMTPAddress& p_sender)
{
  // Are we the robot account, use the robot account settings
  p_sender.m_emailAddress = m_config.GetRobotUsesAccount();
  p_sender.m_friendlyName = m_config.GetRobotUsesAlias();
}

void
GRAPHConnection::AddAttachments(XString p_messageID,SMTPMessage& p_message,RelayDlg* p_relay)
{
  XString text;

  for(int ind = 0;ind < p_message.GetNumberOfAttachments(); ++ind)
  {
    SMTPAttachment* attachment = p_message.GetAttachment(ind);
    if(p_relay)
    {
      text.Format(_T("Sending attachment [%d] : %s"),ind,attachment->GetTitle().GetString());
      p_relay->SetText(text);
      p_relay->Increment();
    }
    if(attachment->GetFileSize() < OFFICE365_INLINE_MAX)
    {
      AddAttachment(p_messageID,attachment);
    }
    else
    {
      if(CreateUploadSession(p_messageID,attachment->GetTitle(),attachment->GetFileSize()))
      {
        DetachOauth2();
        UploadAttachment(attachment);
        CancelUploadSession();
      }
      else
      {
        HandleError(_T("Cannot create upload session for large attachment!"));
      }
      RestoreOAuth2();
    }
  }
}

void
GRAPHConnection::AddAttachment(XString p_messageID,SMTPAttachment* p_attachment)
{
  XString url;
  XString sender = CrackedURL::EncodeURLChars(m_sender.GetString());
  url.Format(OFFICE365_MAIL_INLINE,sender.GetString(),p_messageID.GetString());

  HTTPMessage attach(HTTPCommand::http_post,url);
  attach.SetContentType(_T("application/json"));
  XString body(_T("{ \"@odata.type\": \"#microsoft.graph.fileAttachment\",\n"));
  body.AppendFormat(_T(" \"name\" : \"%s\",\n"),p_attachment->GetTitle().GetString());
  body += _T(" \"contentBytes\" : \"");
#ifdef _UNICODE
  const BYTE* encoded = p_attachment->GetEncodedBuffer();
  int length = (int)strlen((char*)encoded);
  XString text;
  bool foundBom = false;
  TryConvertNarrowString(p_attachment->GetEncodedBuffer(),length,_T(""),text,foundBom);
  body += text;
#else
  body += XString((char*)p_attachment->GetEncodedBuffer());
#endif
  body += _T("\"\n}\n");
  attach.SetBody(body);

  bool result = m_client.Send(&attach);
  if(result)
  {
    if(m_client.GetStatus() != HTTP_STATUS_CREATED)
    {
      HandleError(_T("Unexpected result when appending attachment to the concept e-mail message."));
    }
  }
  else
  {
    HandleError(_T("Attachment NOT appended to the concept e-mail message!"));
  }
}

bool
GRAPHConnection::CreateUploadSession(const XString& p_messageID,const XString& p_name,int p_size)
{
  XString url;
  XString what(_T("file"));
  XString sender = CrackedURL::EncodeURLChars(m_sender.GetString());
  url.Format(OFFICE365_MAIL_UPLOAD,sender.GetString(),p_messageID.GetString());

  JSONMessage upload;
  upload.SetContentType(_T("application/json"));
  upload.SetURL(url);

  // Create attachment message
  upload.GetValue().SetDatatype(JsonType::JDT_object);
  JSONpair item;
  item.m_name = _T("AttachmentItem");
  item.m_value.SetDatatype(JsonType::JDT_object);
  upload.GetValue().GetObject().push_back(item);
  JSONvalue& itemval = upload.GetValue().GetObject().front().m_value;

  JSONpair type;
  JSONpair name;
  JSONpair size;

  type.m_name = _T("attachmentType");
  name.m_name = _T("name");
  size.m_name = _T("size");
  type.m_value.SetValue(what);
  name.m_value.SetValue(p_name);
  size.m_value.SetValue(p_size);

  itemval.GetObject().push_back(type);
  itemval.GetObject().push_back(name);
  itemval.GetObject().push_back(size);

  // Request upload session id
  if(m_client.Send(&upload))
  {
    JSONvalue* value = upload.FindValue(_T("uploadUrl"));
    if(value)
    {
      m_uploadSession = value->GetString();
      m_uploadSession.Replace(_T("@"),_T("%40"));
      return true;
    }
  }
  HandleError(_T("Kan geen upload sessie aanmaken voor een e-mail attachment!"));
  return false;
}

void
GRAPHConnection::UploadAttachment(SMTPAttachment* p_attachment)
{
  int size  = p_attachment->GetFileSize();
  int parts = 1 + (size / OFFICE365_INLINE_MAX);

  for(int ind = 0;ind < parts; ++ind)
  {
    // Calculate sizes and ranges
    XString range;
    int begin  = ind   * OFFICE365_INLINE_MAX;
    int ending = begin + OFFICE365_INLINE_MAX - 1;
    int length = OFFICE365_INLINE_MAX;
    if(ending >= size)
    {
      length = size - begin;
      ending = size - 1;
    }
    range.Format(_T("bytes %d-%d/%d"),begin,ending,size);

    // Create upload message
    HTTPMessage upload(HTTPCommand::http_put,m_uploadSession.GetString());
    upload.SetContentType(_T("application/octet-stream"));
    upload.AddHeader(_T("Content-Range"),range);
    upload.SetContentLength(length);
    // Add binary buffer as body part
    upload.GetFileBuffer()->AddBuffer((uchar*)&(p_attachment->GetEncodedBuffer()[begin]),length);

    // Upload the part of the attachment
    if(m_client.Send(&upload))
    {
      if(m_client.GetStatus() != HTTP_STATUS_OK &&
         m_client.GetStatus() != HTTP_STATUS_CREATED)
      {
        HandleError(_T("Upload attachment interrupted with error!"));
        return;
      }
    } 
    else
    {
      HandleError(_T("Upload attachment interrupted with error!"));
      return;
    }
  }
}

bool
GRAPHConnection::CancelUploadSession()
{
  HTTPMessage cancel(HTTPCommand::http_delete,m_uploadSession.GetString());
  if(m_client.Send(&cancel))
  {
    if(m_client.GetStatus() == HTTP_STATUS_NO_CONTENT)
    {
      return true;
    }
  }
  HandleError(_T("Cannot cancel the attachment upload session."));
  return false;
}
