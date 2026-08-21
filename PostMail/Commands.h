////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Commands.h
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

// Mail priority
#define MAILPRIORITY_LOW    0
#define MAILPRIORITY_NORMAL 1
#define MAILPRIORITY_HIGH   2

// Used to be a enum, now int + defines
#define NOTIFY_NEVER   0x00
#define NOTIFY_FAILURE 0x02
#define NOTIFY_SUCCESS 0x04
#define NOTIFY_DELAY   0x08
#define NOTIFY_HEADER  0x10
#define NOTIFY_FULL    0x20


enum class PMCType
{
  PMC_NoType = 0   // No type yet
 ,PMC_Present      // Must just be present
 ,PMC_SString      // Single string
 ,PMC_MString      // Multiple strings
 ,PMC_Boolean      // Boolean yes/no
 ,PMC_Integer      // Integer value
 ,PMC_Language     // English/Dutch/French/German
 ,PMC_SHA256       // Coded password
 ,PMC_Notify       // Notification code
 ,PMC_Priority     // Priority code
};

typedef struct _command
{
  XString m_english;
  XString m_dutch;
  XString m_french;
  XString m_german;
  PMCType m_type;
}
Command;

// These command numbers correspond with the 
// order in the 'commands[]' array
enum class CommandNR
{
  COM_HOST = 0
 ,COM_LOGIN
 ,COM_MAILID
 ,COM_FROM
 ,COM_TO
 ,COM_CC
 ,COM_BCC
  ,COM_SUBJECT
  ,COM_PROFILE
 ,COM_ERRORS
 ,COM_DIALOG
 ,COM_READCONF
 ,COM_SENDCONF
 ,COM_DELIVER
 ,COM_PROGRESS
 ,COM_DELETE
 ,COM_EDITSUBJECT
 ,COM_EDITBODY
 ,COM_NOTIFY
 ,COM_WRITE
 ,COM_NOPWDCRYPT
 ,COM_PASSWORD
 ,COM_ATTACH
 ,COM_IMPORT
 ,COM_LANGUAGE
 ,COM_TIMEOUT
 ,COM_LOGLEVEL
};

// Functions to find the command and type

CommandNR FindMailCommand  (XString& p_name,PMCType& p_type);
int       FindDeliverStatus(XString& p_status);
int       FindMailPriority (XString& p_priority);
int       FindMailBoolean  (XString& p_value);   // Returns 1 for true, 0 for false, -1 for not found
int       FindMailLanguage (XString& p_language);

