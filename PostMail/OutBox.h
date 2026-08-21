////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: OutBox.h
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
#pragma once

typedef enum _outboxType
{
    OBT_NO_OUTBOX = 0   // no outbox configured
   ,OBT_APPDATA         // outbox resides in your roaming profile
   ,OBT_PRIVATE         // outbox is your own private directory
   ,OBT_SHARED          // outbox is a shared directory of your organisation
}
BoxType;

typedef enum _selection
{
  DS_TODAY = 0
 ,DS_WEEK
 ,DS_MONTH
 ,DS_3MONTHS
 ,DS_6MONTHS
 ,DS_YEAR
 ,DS_ALL
}
DateSel;

#define MAX_SUBJECT_FILENAME 100

// Maximum number of emails in a persons outbox.
// In theory it could be as high as 150.000 (the max for NTFS filesystems)
// But a higher setting will cripple the file system
#define MAX_OUTBOX       2000
#define MAX_NTFS_FILES 150000

// Max outboxes to remember to look into
#define MAX_OUTBOX_HISTORY 10

class SMTPMessage;
class SMTPAddress;

class OutBox
{
public:
  OutBox();
 ~OutBox();

  bool      IsActivated();
  void      ReadSettings();
  void      SaveSettings();
  bool      SaveInOutbox(SMTPMessage& p_message,bool p_sent);
  bool      ReadOutBox(CListCtrl& p_list,XString p_search);
  void      ComputeDirectory();
  int       CountEmailFiles();
  void      FullOutboxWarning();

  // GETTERS
  DateSel   GetSelection();
  BoxType   GetType();
  XString   GetDirectory();
  bool      GetCanSetDirectory();
  // SETTERS
  void      SetSelection(DateSel p_selection) { m_selection = p_selection; };
  void      SetType     (BoxType p_type)      { m_type      = p_type;      };
  void      SetDirectory(const XString& p_directory);
private:
  XString   CreateNewMessageFilename(SMTPAddress& p_address,const XString& p_subject,bool p_sent);
  void      FilterFilename(XString& p_name);
  bool      SplitOutboxFilename(const XString& p_file,XString& p_time,XString& p_to,XString& p_subject,XString& p_gelukt);
  XString   GetSelectionString();
  void      Warning(int p_total);

  BoxType   m_type;
  DateSel   m_selection;
  XString   m_directory;
  bool      m_canSetDirectory;
  unsigned  m_maxOutbox;
};

inline DateSel
OutBox::GetSelection()
{
  return m_selection;
}

inline BoxType
OutBox::GetType()
{
  return m_type;
}

inline XString
OutBox::GetDirectory()
{
  return m_directory;
}

inline bool
OutBox::GetCanSetDirectory()
{
  return m_canSetDirectory;
}
