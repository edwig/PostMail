/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: GRAPHConnection.h
//
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#pragma once

// Currently 150MB for Office-365 Since 15-4-2015 
// Total size of message plus all attachments
#define OFFICE365_MAX_MESSAGE_SIZE (150 * 1024 * 1024) 
// OFFICE-365 maximum for inline attachments (3 MB)
// Above this size we must do the 'upload session workflow'
#define OFFICE365_INLINE_MAX       (3 * 1024 * 1024)

// Office-365 links for a browser
#define OFFICE365_MAIL_INBOX   _T("https://outlook.office.com")
#define OFFICE365_MAIL_COMPOSE _T("https://outlook.office365.com/mail/compose/")
#define OFFICE365_MAIL_DRAFTS  _T("https://outlook.office365.com/mail/drafts/")
// Where we create the concept email message and send it away
#define OFFICE365_MAIL_CREATE  _T("https://graph.microsoft.com/v1.0/users/%s/messages")
#define OFFICE365_MAIL_SEND    _T("https://graph.microsoft.com/v1.0/users/%s/messages/%s/send")
#define OFFICE365_MAIL_INLINE  _T("https://graph.microsoft.com/v1.0/users/%s/messages/%s/attachments")
#define OFFICE365_MAIL_UPLOAD  _T("https://graph.microsoft.com/v1.0/users/%s/messages/%s/attachments/createUploadSession")
