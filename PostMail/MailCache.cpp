/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: MailCache.cpp
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
#include "StdAfx.h"
#include "PostMail.h"
#include "MailCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MailCache::MailCache()
{
}

MailCache::~MailCache()
{
  Messages::iterator it;
  for(it = m_cache.begin();it != m_cache.end();++it)
  {
    delete it->second;
  }
}

//////////////////////////////////////////////////////////////////////////
//
// GENERAL (PUBLIC) OPERATIONS
//
//////////////////////////////////////////////////////////////////////////

bool    
MailCache::Read(SMTPMessage& p_message,unsigned p_mailID)
{
  XString fileName = GetFilenameForMailID(p_mailID);
  return p_message.ReadFromFile(fileName);
}

bool    
MailCache::ReadAllMail()
{
  Profiles& profiles = theApp.GetProfiles();
  XString map = profiles.GetProfileMap();
  map.AppendFormat(EMAIL_SEARCH);

  CFileFind finder;
  if(finder.FindFile(map))
  {
    finder.FindNextFile();
    do
    {
      XString padNaam  = finder.GetFilePath();
      XString fileNaam = finder.GetFileName();
      int     mailId   = _ttoi(fileNaam.Mid(EMAIL_NUMPOS));

      SMTPMessage* message = new SMTPMessage();
      message->ReadFromFile(padNaam);

      m_cache[mailId] = message;
    }
    while(finder.FindNextFile());
  }
  return true;
}

bool
MailCache::Flush(bool p_viewer,bool p_graph)
{
  bool result = true;

  Messages::iterator it;
  for(it = m_cache.begin();it != m_cache.end();++it)
  {
    // Last check against profiles and SEND IT!
    it->second->CheckAfterRead(p_viewer,p_graph);
    if(it->second->DisplayMessage())
    {
      it->second->CheckHost();
      if(it->second->SendMessage())
      {
        RemoveEmailFile(it->first);
      }
      else
      {
        // Tried to send, but received an error
        // Connect error, transmit error etc.
        result = false;
      }
    }
  }
  return result;
}

XString 
MailCache::GetFilenameForMailID(unsigned p_mailID)
{
  Profiles& profiles = theApp.GetProfiles();
  XString map = profiles.GetProfileMap();
  
  // Add filename and profile map
  map.AppendFormat(EMAIL_TEMPLATE,p_mailID);
  return map;
}

void
MailCache::RemoveEmailFile(unsigned p_mailID)
{
  XString fileName = GetFilenameForMailID(p_mailID);
  DeleteFile(fileName);
}

//////////////////////////////////////////////////////////////////////////
//
// PRIVATE OPERATIONS
//
//////////////////////////////////////////////////////////////////////////

