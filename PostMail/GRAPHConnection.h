/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: GRAPHConnection.h
//
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#pragma once
#include "SMTPMessage.h"
#include "OdataConfig.h"
#include <HTTPClient.h>
#include <JSONMessage.h>
#include <OAuth2Cache.h>

class RelayDlg;
class JSONMessage;

class GRAPHConnection
{
public:
  GRAPHConnection(ODataConfig& p_config);
 ~GRAPHConnection();

  bool    Connect(RelayDlg* p_relay);
  bool    DisConnect();
  bool    TestMessageTooBig(SMTPMessage& p_message);
  bool    SendMessage(SMTPMessage& p_message,RelayDlg* p_relay);
  void    SetSender(const XString& p_sender) { m_sender = p_sender; }

private:
  void    FindEmailAdressOfUPN();
  XString FindMailID(JSONMessage& p_msg);
  XString ComposeMail(const XString& p_mesageID);
  void    SetRobotAccount(SMTPAddress& p_sender);
  void    HandleError(const XString& p_melding);

  // Adding the parts of the message
  void    AddFrom       (JSONobject& p_object,const XString& p_emailAddress);
  void    AddBody       (JSONobject& p_object,SMTPMessage& p_message);
  void    AddSubject    (JSONobject& p_object,SMTPMessage& p_message);
  void    AddImportance (JSONobject& p_object,SMTPMessage& p_message);
  void    AddDelivery   (JSONobject& p_object,SMTPMessage& p_message);
  void    AddReadReceipt(JSONobject& p_object,SMTPMessage& p_message);
  void    AddRecipients (JSONobject& p_object,Recipients&  p_recipients,const XString& p_type);
  void    AddRecipient  (JSONvalue&  p_value, SMTPAddress& p_address);
  void    AddAttachments(XString p_messageID,SMTPMessage& p_message,RelayDlg* p_relay);
  void    AddAttachment (XString p_messageID,SMTPAttachment* p_attachment);

  bool    CreateUploadSession(const XString& p_messageID,const XString& p_name,int p_size);
  void    UploadAttachment(SMTPAttachment* p_attachment);
  bool    CancelUploadSession();
  void    DetachOauth2();
  void    RestoreOAuth2();

  // DATA
  int          m_session   { 0 };   // OAuth2 session number in the OAuth2 cache
  unsigned     m_totalSize { 0 };   // Total size of the e-mail including attachments
  ODataConfig& m_config;            // Configuration 'postmail.json' as read from file
  OAuth2Cache  m_cache;             // OAuth2 caching for bearer/refresh tokens
  HTTPClient   m_client;            // HTTP client to send out messages
  XString      m_sender;            // AD User-principal-name of the sender
  XString      m_userPrincipalName; // Default user of this process
  XString      m_uploadSession;     // Temporary upload session for an attachment
  XString      m_errors;
};
