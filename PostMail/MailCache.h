/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: MailCache.h
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
#include "SMTPMessage.h"
#include <map>

// Template for creating and searching email files
#define EMAIL_TEMPLATE _T("email_%u.ceml")
#define EMAIL_SEARCH   _T("email_*.ceml")
// Position of the number in the mail ID
#define EMAIL_NUMPOS   6 

typedef std::map<unsigned,SMTPMessage*> Messages;

class MailCache
{
public:
  MailCache();
 ~MailCache();

  // GENERAL OPERATIONS

  bool    Read(SMTPMessage& p_message,unsigned p_mailID);
  bool    ReadAllMail();
  bool    Flush(bool p_viewer,bool p_graph);
  XString GetFilenameForMailID(unsigned p_mailID);
  void    RemoveEmailFile(unsigned p_mailID);

  // GETTERS AND SETTERS

private:
  Messages  m_cache;
};