////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: OutBox.cpp
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "stdafx.h"
#include "PostMail.h"
#include "OutBox.h"
#include "SMTPAddress.h"
#include "SMTPMessage.h"
#include "Message.h"
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

OutBox::OutBox()
{
  m_maxOutbox = MAX_OUTBOX;
  m_canSetDirectory = true;
  ReadSettings();
}

OutBox::~OutBox()
{
}

void
OutBox::ReadSettings()
{
  Profiles& prof = theApp.GetProfiles();

  m_type      = prof.GetOutboxType();
  m_selection = prof.GetOutboxSelection();
  m_directory = prof.GetOutboxDirectory();

  ComputeDirectory();
}

void
OutBox::SaveSettings()
{
  Profiles& prof = theApp.GetProfiles();

  prof.SetTypeOfOutbox(m_type);
  prof.SetOutboxSelection(m_selection);
  prof.SetOutboxDirectory(m_directory);

  // En direct opslaan
  prof.WriteProfiles();
}

bool
OutBox::IsActivated()
{
  if(m_type > OBT_NO_OUTBOX && !m_directory.IsEmpty())
  {
    return true;
  }
  return false;
}

void
OutBox::ComputeDirectory()
{
  if(m_type == OBT_APPDATA)
  {
    m_directory = theApp.GetProfiles().GetProfileMap() + _T("OutBox");
  }
  else if(m_type == OBT_SHARED)
  {
    extern XString GetLoginName();

    XString shared = theApp.GetProfiles().GetCommonOutbox();
    if(!shared.IsEmpty())
    {
      XString user = GetLoginName();
      user.Replace(_T('\\'),_T('_'));

      shared.TrimRight(_T('\\'));
      m_directory = shared + _T("\\") + user;
      m_canSetDirectory = theApp.GetProfiles().GetCanModifyOutbox();

      theApp.GetProfiles().CreateProfileMap(m_directory);
    }
  }
  else // m_type == OBT_PRIVATE || OBT_NO_OUTBOX
  {
    // m_directory is not changed
  }
}

void
OutBox::SetDirectory(const XString& p_directory) 
{
  if(m_canSetDirectory)
  {
    m_directory = p_directory;
  }
}

bool
OutBox::SaveInOutbox(SMTPMessage& p_message,bool p_sent)
{
  // Fail quickly
  if(!IsActivated())
  {
    return false;
  }
  // Get first recipient of default type "TO"
  SMTPAddress to = p_message.GetRecipient(0);
  if(to.m_emailAddress.IsEmpty())
  {
    return false;
  }
  // Create filename
  XString filename(m_directory);
  if(filename.Right(1) != _T("\""))
  {
    filename += _T("\\");
  }
  filename += CreateNewMessageFilename(to,p_message.GetSubject(),p_sent);

  // Now save under this filename
  return p_message.SaveToFile(filename);
}

// File name consists of <timestamp>_<mailto>
XString   
OutBox::CreateNewMessageFilename(SMTPAddress& p_address,const XString& p_subject,bool p_sent)
{
  __timeb64 now;
  struct tm today;
  XString timestamp;
  XString name;

  _ftime64_s(&now);
  _localtime64_s(&today,&now.time);
  timestamp.Format(_T("%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2d%03d-")
                  ,today.tm_year + 1900
                  ,today.tm_mon  + 1
                  ,today.tm_mday
                  ,today.tm_hour
                  ,today.tm_min
                  ,today.tm_sec
                  ,now.millitm);

  // Getting friendly name or email address
  if(!p_address.m_friendlyName.IsEmpty())
  {
    name = p_address.m_friendlyName;
  }
  else
  {
    name = p_address.m_emailAddress;
  }
  FilterFilename(name);

  // Total PostMailFile (PMF)
  XString filename = timestamp + name + _T("-");
  XString subject  = p_subject.Left(MAX_SUBJECT_FILENAME);
  FilterFilename(subject);
  filename += subject;
  filename += p_sent ? _T(".pmf") : _T(".pme");
  return filename;
}

void
OutBox::FilterFilename(XString& p_name)
{
  // All letters not allowed in a filename
  p_name.Replace(_T('\\'),_T('_'));
  p_name.Replace(_T('/'), _T('_'));
  p_name.Replace(_T(':'), _T('_'));
  p_name.Replace(_T('*'), _T('_'));
  p_name.Replace(_T('?'), _T('_'));
  p_name.Replace(_T('\"'),_T('_'));
  p_name.Replace(_T('<'), _T('_'));
  p_name.Replace(_T('>'), _T('_'));
  p_name.Replace(_T('|'), _T('_'));
  // This one is allowed, but reserved by OutBox
  p_name.Replace(_T('-'), _T('_'));
}

XString
OutBox::GetSelectionString()
{
  __timeb64 now;
  _ftime64_s(&now);
  __int64 diff = 0;

  // Calculate time backwards
  switch(m_selection)
  {
    case DS_TODAY:    diff = 24 * 60 * 60;       break;
    case DS_WEEK:     diff = 24 * 60 * 60 * 7;   break;
    case DS_MONTH:    diff = 24 * 60 * 60 * 31;  break;
    case DS_3MONTHS:  diff = 24 * 60 * 60 * 92;  break;
    case DS_6MONTHS:  diff = 24 * 60 * 60 * 183; break;
    case DS_YEAR:     diff = 24 * 60 * 60 * 365; break;
    case DS_ALL:      diff = 24 * 60 * 60 * 365;
                      diff *= 20;
                      break;
  }
  now.time -= diff;

  // Print in string
  XString timestamp;
  struct tm today;
  _localtime64_s(&today,&now.time);
  timestamp.Format(_T("%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2d%03d-")
                  ,today.tm_year + 1900
                  ,today.tm_mon  + 1
                  ,today.tm_mday
                  ,today.tm_hour
                  ,today.tm_min
                  ,today.tm_sec
                  ,now.millitm);
  return timestamp;
}

// Read in the OutBox directory
// With a m_selection limitation in time
bool
OutBox::ReadOutBox(CListCtrl& p_list,XString p_search)
{
  // Reset the list
  p_list.DeleteAllItems();

  // Fail quickly
  if(!IsActivated())
  {
    return false;
  }
  int ind   = 0;
  int total = 0;
  XString selection = GetSelectionString();
  XString pattern(_T("\\????????T?????????*.pm?"));
  TCHAR buffer[MAX_PATH+1];

  // Search on word
  if(!p_search.IsEmpty())
  {
    p_search.Trim(_T("*"));
    p_search.Trim(_T("%"));
    FilterFilename(p_search);
    pattern = _T("\\*") + p_search + _T("*.pm?");
  }

  _tcscpy_s(buffer,MAX_PATH,m_directory);
  _tcscat_s(buffer,MAX_PATH,pattern);

  // Cycle through the files in this directory
  intptr_t nHandle = 0;
  struct _tfinddata_t fileInfo;
  nHandle = _tfindfirst((LPCTSTR)buffer,&fileInfo);
  if(nHandle != -1)
  {
    do 
    {
      XString time,to,subject,gelukt;
      XString file(fileInfo.name);
      if(file.Compare(selection) > 0)
      {
        // Split filename in parts
        SplitOutboxFilename(file,time,to,subject,gelukt);
        // Add to list control
        p_list.InsertItem(LVIF_TEXT|LVIF_STATE,ind,time,0,0,0,0);
        p_list.SetItemText(ind,1,gelukt);
        p_list.SetItemText(ind,2,subject);
        p_list.SetItemText(ind,3,to);
        p_list.SetItemText(ind,4,file);
      }
      ++total;
    } 
    while(_tfindnext(nHandle,&fileInfo) != -1);
  }
  _findclose(nHandle);

  Warning(total);

  return true;
}

bool
OutBox::SplitOutboxFilename(const XString& p_file,XString& p_time,XString& p_to,XString& p_subject,XString& p_gelukt)
{
  // Reset everything
  p_to.Empty();
  p_time.Empty();
  p_subject.Empty();

  // Time found by pattern
  p_time = p_file.Mid(6,2) + _T("-") + p_file.Mid(4,2)  + _T("-") + p_file.Mid(0,4)  + _T(" ") +
           p_file.Mid(9,2) + _T(":") + p_file.Mid(11,2) + _T(":") + p_file.Mid(13,2);
  // Find separators
  int pos1 = p_file.Find(_T('-'));
  int pos2 = p_file.Find(_T('-'),pos1 + 1);
  // Find addressee
  if(pos1 > 0 && pos2 > 0)
  {
    p_to = p_file.Mid(pos1+1,pos2 - pos1 - 1);
  }
  // Find part of the subject
  if(pos2 > 0)
  {
    p_subject = p_file.Mid(pos2 + 1);
    p_subject = p_subject.Left(p_subject.GetLength() - 4);
  }
  // Find extensie
  pos1 = p_file.ReverseFind(_T('.'));
  if(pos1 > 0)
  {
    XString extensie = p_file.Mid(pos1 + 1);
    p_gelukt = extensie.CompareNoCase(_T("pmf")) == 0 ? _T("OK") : _T("FOUT");
  }
  // AOK
  return !p_to.IsEmpty() && !p_subject.IsEmpty();
}

int
OutBox::CountEmailFiles()
{
  // See if we sould measure something
  if(m_directory.IsEmpty() || m_type == OBT_NO_OUTBOX)
  {
    return 0;
  }

  // Cycle through the files in this directory
  int total = 0;
  XString directory(m_directory);
  directory += _T("\\*.pm?");
  intptr_t nHandle = 0;
  struct _tfinddata_t fileInfo;
  nHandle = _tfindfirst(directory.GetString(),&fileInfo);
  if(nHandle != -1)
  {
    do
    {
      ++total;
    }
    while(_tfindnext(nHandle,&fileInfo) != -1);
  }
  _findclose(nHandle);

  return total;
}

void
OutBox::FullOutboxWarning()
{
  Warning(CountEmailFiles());
}

void
OutBox::Warning(int p_total)
{
  if(p_total > (int)m_maxOutbox)
  {
    XString message;
    message.Format(g_message[MESS_MAX_OUTBOX][g_lang],p_total,MAX_OUTBOX);
    theApp.WideMessageBox(NULL,message,WhoAmI,MB_OK|MB_ICONWARNING);
  }
}