/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: OutBoxDlg.cpp
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
#include "Postmail.h"
#include "OutBoxDlg.h"
#include "ConfigOutboxDlg.h"
#include "Message.h"
#include "Version.h"
#include <ExecuteProcess.h>
#include <FileDialog.h>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// OutBoxDlg dialog

OutBoxDlg::OutBoxDlg(CWnd* p_parent,OutBox& p_outbox)
        	:StyleDialog(OutBoxDlg::IDD,p_parent)
          ,m_outbox(p_outbox)
{
}

OutBoxDlg::~OutBoxDlg()
{
}

void OutBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  DDX_Control(pDX,IDC_OUTBOX,   m_editTitle, m_title);
  DDX_Control(pDX,IDC_SEARCH,   m_editSearch,m_search);
  DDX_Control(pDX,IDC_DOSEARCH, m_buttonSearch);
  DDX_Control(pDX,IDC_REMFILT,  m_buttonRemoveFilter);
  DDX_Control(pDX,IDC_LIST,     m_list);
  DDX_Control(pDX,IDC_TODAY,    m_buttonToday);
  DDX_Control(pDX,IDC_WEEK,     m_buttonWeek);
  DDX_Control(pDX,IDC_MONTH,    m_buttonMonth);
  DDX_Control(pDX,IDC_3MONTHS,  m_button3Months);
  DDX_Control(pDX,IDC_6MONTHS,  m_button6Months);
  DDX_Control(pDX,IDC_YEAR,     m_buttonYear);
  DDX_Control(pDX,IDC_ALL,      m_buttonAll);
  DDX_Control(pDX,IDC_CONFIG,   m_buttonConfig);
  DDX_Control(pDX,IDC_LOGFILE,  m_buttonLogfile);
  DDX_Control(pDX,IDC_ACTIEF,   m_buttonActief);
  DDX_Control(pDX,IDC_SELECTION,m_selection);
  DDX_Control(pDX,IDC_DELETE,   m_buttonDelete);
  DDX_Control(pDX,IDC_AGAIN,    m_buttonAgain);
  DDX_Control(pDX,IDC_CLOSE,    m_buttonOK);

  DDX_Control(pDX,IDC_ST_OUTBOX,m_staticOutbox);
  DDX_Control(pDX,IDC_ST_SEARCH,m_staticSearch);

  if(pDX->m_bSaveAndValidate == FALSE)
  {
    m_buttonLogfile.EnableWindow(theApp.GetProfiles().GetWriteAccess());
    m_buttonActief .EnableWindow(theApp.GetProfiles().GetWriteAccess());
  }
}

BEGIN_MESSAGE_MAP(OutBoxDlg, StyleDialog)
  ON_NOTIFY(NM_DBLCLK,IDC_LIST, &OutBoxDlg::OnLvnItemDoubleClick)
  ON_EN_KILLFOCUS(IDC_SEARCH,   &OutBoxDlg::OnEnKillfocusSearch)
  ON_BN_CLICKED(IDC_DOSEARCH,   &OutBoxDlg::OnBnClickedSearch)
  ON_BN_CLICKED(IDC_REMFILT,    &OutBoxDlg::OnBnClickedRemoveFilter)
  ON_BN_CLICKED(IDC_TODAY,      &OutBoxDlg::OnBnClickedToday)
  ON_BN_CLICKED(IDC_WEEK,       &OutBoxDlg::OnBnClickedWeek)
  ON_BN_CLICKED(IDC_MONTH,      &OutBoxDlg::OnBnClickedMonth)
  ON_BN_CLICKED(IDC_3MONTHS,    &OutBoxDlg::OnBnClicked3months)
  ON_BN_CLICKED(IDC_6MONTHS,    &OutBoxDlg::OnBnClicked6months)
  ON_BN_CLICKED(IDC_YEAR,       &OutBoxDlg::OnBnClickedYear)
  ON_BN_CLICKED(IDC_ALL,        &OutBoxDlg::OnBnClickedAll)
  ON_BN_CLICKED(IDC_CONFIG,     &OutBoxDlg::OnBnClickedConfig)
  ON_BN_CLICKED(IDC_LOGFILE,    &OutBoxDlg::OnBnClickedLogfile)
  ON_BN_CLICKED(IDC_ACTIEF,     &OutBoxDlg::OnBnClickedActief)
  ON_BN_CLICKED(IDC_DELETE,     &OutBoxDlg::OnBnClickedDelete)
  ON_BN_CLICKED(IDC_AGAIN,      &OutBoxDlg::OnBnClickedAgain)
  ON_BN_CLICKED(IDC_CLOSE,      &OutBoxDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL
OutBoxDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(g_message[MESS_OUTBOXVIEWER][g_lang]);
  ShowMinMaxButton();

  InitTexts();
  InitList();
  FillOutbox();
  SetCanResize();

  UpdateData(FALSE);
  return TRUE;
}

void
OutBoxDlg::InitTexts()
{
  m_staticOutbox .SetWindowText(g_message[MESS_OUTBOX      ][g_lang]);
  m_staticSearch .SetWindowText(g_message[MESS_SEARCH      ][g_lang]);
  m_buttonSearch .SetWindowText(g_message[MESS_SEARCH      ][g_lang]);
  m_buttonDelete .SetWindowText(g_message[MESS_DELETE      ][g_lang]);
  m_buttonAgain  .SetWindowText(g_message[MESS_AGAINSHOW   ][g_lang]);
  m_selection    .SetWindowText(g_message[MESS_SELECTION   ][g_lang]);
  m_buttonToday  .SetWindowText(g_message[MESS_LASTDAY     ][g_lang]);
  m_buttonWeek   .SetWindowText(g_message[MESS_LASTWEEK    ][g_lang]);
  m_buttonMonth  .SetWindowText(g_message[MESS_LASTMONTH   ][g_lang]);
  m_button3Months.SetWindowText(g_message[MESS_LAST3MONTHS ][g_lang]);
  m_button6Months.SetWindowText(g_message[MESS_LAST6MONTHS ][g_lang]);
  m_buttonYear   .SetWindowText(g_message[MESS_LASTYEAR    ][g_lang]);
  m_buttonAll    .SetWindowText(g_message[MESS_ALLMAIL     ][g_lang]);
  m_buttonConfig .SetWindowText(g_message[MESS_CONFIGOUTBOX][g_lang]);
  m_buttonLogfile.SetWindowText(g_message[MESS_LOGOUTBOX   ][g_lang]);
  m_buttonActief .SetWindowText(g_message[MESS_OUTBOXACTIVE][g_lang]);
  m_buttonOK     .SetWindowText(g_message[MESS_OK          ][g_lang]);
}

void
OutBoxDlg::InitList()
{
  m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EDITLABELS);
  m_list.InsertColumn(0,g_message[MESS_DATETIME][g_lang],LVCFMT_LEFT,120);
  m_list.InsertColumn(1,g_message[MESS_RESULT  ][g_lang],LVCFMT_LEFT,100);
  m_list.InsertColumn(2,g_message[MESS_SUBJECT ][g_lang],LVCFMT_LEFT,300);
  m_list.InsertColumn(3,g_message[MESS_RECEIVER][g_lang],LVCFMT_LEFT,200);
  m_list.InsertColumn(4,g_message[MESS_BESTAND ][g_lang],LVCFMT_LEFT,0);
}

// Triggered by SetCanResize
void
OutBoxDlg::SetupDynamicLayout()
{
  // See to it that we get a manager;
  StyleDialog::SetupDynamicLayout();

  auto manager = GetDynamicLayout();
  if (manager != nullptr)
  {
    manager->AddItem(IDC_OUTBOX,   CMFCDynamicLayout::MoveNone(),          CMFCDynamicLayout::SizeHorizontal(100));
    manager->AddItem(IDC_SEARCH,   CMFCDynamicLayout::MoveNone(),          CMFCDynamicLayout::SizeHorizontal(100));
    manager->AddItem(IDC_DOSEARCH, CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_REMFILT,  CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());

    manager->AddItem(IDC_LIST,     CMFCDynamicLayout::MoveNone(),          CMFCDynamicLayout::SizeHorizontalAndVertical(100,100));

    manager->AddItem(IDC_SELECTION,CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_TODAY,    CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_WEEK,     CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_MONTH,    CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_3MONTHS,  CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_6MONTHS,  CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_YEAR,     CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_ALL,      CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_CONFIG,   CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_LOGFILE,  CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_ACTIEF,   CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());

    manager->AddItem(IDC_DELETE,   CMFCDynamicLayout::MoveVertical(100),   CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_AGAIN,    CMFCDynamicLayout::MoveVertical(100),   CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_CLOSE,    CMFCDynamicLayout::MoveHorizontalAndVertical(100,100),CMFCDynamicLayout::SizeNone());
  }
}

void
OutBoxDlg::FillOutbox()
{
  BoxType type = m_outbox.GetType();
  switch(type)
  {
    case OBT_NO_OUTBOX: m_title = g_message[MESS_NOBOXCONFIG][g_lang]; break;
    case OBT_APPDATA:   m_title = g_message[MESS_PERSOUTBOX ][g_lang]; break;
    case OBT_PRIVATE:   m_title = g_message[MESS_PRIVATEBOX ][g_lang]; break;
    case OBT_SHARED:    m_title = g_message[MESS_ORGOUTBOX  ][g_lang]; break;
  }
  CString outbox = m_outbox.GetDirectory();
  if(outbox.IsEmpty() && type == OBT_SHARED)
  {
    m_title = g_message[MESS_NOBOXHELP][g_lang];
  }
  else if(type == OBT_PRIVATE || type == OBT_APPDATA || type == OBT_SHARED)
  {
    m_title += outbox;
  }
  UpdateData(FALSE);

  // Reset radio buttons
  m_buttonToday  .SetCheck(FALSE);
  m_buttonWeek   .SetCheck(FALSE);
  m_buttonMonth  .SetCheck(FALSE);
  m_button3Months.SetCheck(FALSE);
  m_button6Months.SetCheck(FALSE);
  m_buttonYear   .SetCheck(FALSE);
  m_buttonAll    .SetCheck(FALSE);

  switch(m_outbox.GetSelection())
  {
    case DS_TODAY:    m_buttonToday  .SetCheck(TRUE); break;
    case DS_WEEK:     m_buttonWeek   .SetCheck(TRUE); break;
    case DS_MONTH:    m_buttonMonth  .SetCheck(TRUE); break;
    case DS_3MONTHS:  m_button3Months.SetCheck(TRUE); break;
    case DS_6MONTHS:  m_button6Months.SetCheck(TRUE); break;
    case DS_YEAR:     m_buttonYear   .SetCheck(TRUE); break;
    case DS_ALL:      m_buttonAll    .SetCheck(TRUE); break;
  }

  m_buttonActief.SetCheck(theApp.GetProfiles().GetHasCentralLogfile());

  // Now read in a ton of email
  m_outbox.ReadOutBox(m_list,m_search);
}

bool
OutBoxDlg::BeSureToDelete()
{
  CString ask;
  int items = m_list.GetSelectedCount();
  ask.Format(g_message[MESS_DELEMAILS][g_lang],items,items > 1 ? _T("s") : _T(""));

  if(theApp.WideMessageBox(GetSafeHwnd(),ask,WhoAmI,MB_YESNO|MB_DEFBUTTON2|MB_ICONINFORMATION) == IDYES)
  {
    return true;
  }
  return false;
}

// OutBoxDlg message handlers

void
OutBoxDlg::OnEnKillfocusSearch()
{
  CString val(m_search);
  UpdateData();
  if(val.Compare(m_search) && m_search.IsEmpty())
  {
    FillOutbox();
  }
}

void
OutBoxDlg::OnBnClickedSearch()
{
  FillOutbox();
}

void
OutBoxDlg::OnBnClickedRemoveFilter()
{
  m_search.Empty();
  UpdateData(FALSE);
  FillOutbox();
}

XString
OutBoxDlg::GetClipboardViewer(const XString& p_filename)
{
  int pos = p_filename.ReverseFind(_T('.'));
  if(pos > 0)
  {
    CString extensie = p_filename.Mid(pos + 1);
    if(extensie.CompareNoCase(_T("pmf")) == 0)
    {
      return _T("/V ");
    }
  }
  return _T("");
}

void 
OutBoxDlg::OnLvnItemDoubleClick(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  int item = pNMLV->iItem;
  Process(item);
  *pResult = 0;
}

void
OutBoxDlg::OnBnClickedAgain()
{
  POSITION pos = m_list.GetFirstSelectedItemPosition();
  if(pos)
  {
    int item = m_list.GetNextSelectedItem(pos);
    Process(item);
  }
}

void
OutBoxDlg::Process(int p_item)
{
  XString filename = m_list.GetItemText(p_item,4);
  filename = GetClipboardViewer(filename) + _T("\"") + m_outbox.GetDirectory() + _T("\\") + filename + _T("\"");

  XString errors;
  int res = ExecuteProcess(_T("PostMail.exe"),filename,true,errors,SW_SHOW,true);
  if(res && !errors.IsEmpty())
  {
    theApp.WideMessageBox(GetSafeHwnd(),errors,WhoAmI,MB_OK|MB_ICONERROR);
  }
  else if(res == 0)
  {
    // Mail was sent, delete the file
    filename = filename.Trim(_T('\"'));
    if(DeleteFile(filename) == 0)
    {
      // Delete has failed
      int error = GetLastError();
      XString message;
      message.Format(g_message[MESS_FILE_DELETE][g_lang],error,filename.GetString());
      theApp.Log(LOGLEVEL_ERROR,message);
    }
    else
    {
      // Delete the item as well
      m_list.DeleteItem(p_item);
    }
  }
}


void 
OutBoxDlg::OnBnClickedToday()
{
  m_outbox.SetSelection(DS_TODAY);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClickedWeek()
{
  m_outbox.SetSelection(DS_WEEK);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClickedMonth()
{
  m_outbox.SetSelection(DS_MONTH);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClicked3months()
{
  m_outbox.SetSelection(DS_3MONTHS);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClicked6months()
{
  m_outbox.SetSelection(DS_6MONTHS);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClickedYear()
{
  m_outbox.SetSelection(DS_YEAR);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClickedAll()
{
  m_outbox.SetSelection(DS_ALL);
  FillOutbox();
}

void 
OutBoxDlg::OnBnClickedConfig()
{
  ConfigOutboxDlg config(this
                        ,m_outbox.GetCanSetDirectory()
                        ,m_outbox.GetDirectory());
  if(config.DoModal() == IDOK)
  {
    m_outbox.ReadSettings();
  }
  FillOutbox();
}

void
OutBoxDlg::OnBnClickedLogfile()
{
  Profiles& profiles = theApp.GetProfiles();
  XString filename = profiles.GetCentralLogfile();

  DocFileDialog dlg(GetSafeHwnd()
                   ,false
                   ,g_message[MESS_PROMPTLOG][g_lang]
                   ,_T("txt")
                   ,filename
                   ,0
                   ,g_message[MESS_LOGEXTENT][g_lang]);
  if(dlg.DoModal() == IDOK)
  {
    filename = dlg.GetChosenFile();

    // Check if we can open the file for appending
    WinFile file(filename);
    if(file.Open(winfile_append,FAttributes::attrib_none,Encoding::UTF8))
    {
      file.Write(g_message[MESS_LOGHEADER][g_lang]);
      file.Close();
    }
    else 
    {
      theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_NOACCESS][g_lang] + filename,WhoAmI,MB_OK|MB_ICONERROR);
      return;
    }
    profiles.SetCentralLogfile(filename);
    profiles.WriteCentralSettings();
    m_buttonActief.SetCheck(TRUE);
  }
}

void
OutBoxDlg::OnBnClickedActief()
{
  Profiles& profiles = theApp.GetProfiles();
  int actief = m_buttonActief.GetCheck();
  if(actief)
  {
    // Activating? Choose your logfile
    if (!profiles.GetHasCentralLogfile())
    {
      OnBnClickedLogfile();
    }
  }
  else
  {
    // Trying to deactivate
    if(profiles.GetHasCentralLogfile())
    {
      CString filename = profiles.GetCentralLogfile();
      CString vraag;
      vraag.Format(g_message[MESS_CLOSELOG][g_lang],filename.GetString());
      if(theApp.WideMessageBox(GetSafeHwnd(),vraag,_T(PRODUCT_NAME),MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
      {
        profiles.SetCentralLogfile(_T(""));
        profiles.WriteCentralSettings();
      }
    }
  }
  // Show current actual state of the central logfile
  m_buttonActief.SetCheck(profiles.GetHasCentralLogfile());
}

void
OutBoxDlg::OnBnClickedDelete()
{
  int deleted = 0;
  int failed  = 0;
  int oneitem = 0;

  if(BeSureToDelete() == false)
  {
    return;
  }

  // Remove all selected items from the list;
  POSITION pos = m_list.GetFirstSelectedItemPosition();
  int ind = m_list.GetNextSelectedItem(pos);
  while(ind >= 0)
  {
    XString filename = m_list.GetItemText(ind,4);
    filename = m_outbox.GetDirectory() + _T("\\") + filename;
    if(DeleteFile(filename))
    {
      oneitem = ind;
      ++deleted;
    }
    else
    {
      ++failed;
    }
    ind = m_list.GetNextSelectedItem(pos);
  }

  // Optimize for the deletion of one record
  if(deleted == 1 && failed == 0)
  {
    m_list.DeleteItem(oneitem);
  }
  else
  {
    FillOutbox();
  }
}

void 
OutBoxDlg::OnBnClickedOk()
{
  StyleDialog::OnOK();
}

void
OutBoxDlg::OnOK()
{
  CWnd* focus = GetFocus();
  if(focus->GetDlgCtrlID() == IDC_SEARCH)
  {
    UpdateData();
    FillOutbox();
  }
  else
  {
    StyleDialog::OnOK();
  }
}
