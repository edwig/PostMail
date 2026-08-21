/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ConfigOutboxDlg.cpp
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
#include "Postmail.h"
#include "ConfigOutboxDlg.h"
#include "MapDialog.h" 
#include "OutBoxDlg.h"
#include "Message.h"
#include "Version.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ConfigOutboxDlg dialog

IMPLEMENT_DYNAMIC(ConfigOutboxDlg, StyleDialog)

ConfigOutboxDlg::ConfigOutboxDlg(CWnd* p_parent,bool p_canSetDir,const XString& p_directory)
	              :StyleDialog(ConfigOutboxDlg::IDD, p_parent)
                ,m_canSetDirectory(p_canSetDir)
                ,m_directory(p_directory)
                ,m_type(OBT_SHARED)
                ,m_selection(DS_WEEK)
                ,m_maxFiles(MAX_OUTBOX)
                ,m_init(false)
                ,m_admin(false)
{
}

ConfigOutboxDlg::~ConfigOutboxDlg()
{
}

void 
ConfigOutboxDlg::DoDataExchange(CDataExchange* pDX)
{
	StyleDialog::DoDataExchange(pDX);

  DDX_Control(pDX,IDC_TYPE,         m_comboType);
  DDX_Control(pDX,IDC_DIRECTORY,    m_editDirectory,m_directory);
  DDX_Control(pDX,IDC_CHOOSE,       m_buttonChoose);
  DDX_Control(pDX,IDC_SELECT,       m_comboSelection);
  DDX_Control(pDX,IDC_MAXFILES,     m_editMaxFiles,m_maxFiles);
  DDX_Control(pDX,IDC_LIST,         m_list);

  DDX_Control(pDX,IDC_ST_TYPEBOX,   m_staticTypeOutbox);
  DDX_Control(pDX,IDC_ST_MAPMAIL,   m_staticMapMail);
  DDX_Control(pDX,IDC_ST_SELECTION, m_staticInitSelect);
  DDX_Control(pDX,IDC_ST_WARNAT,    m_staticWarnAt);
  DDX_Control(pDX,IDC_ST_HISTORY,   m_staticHistory);

  DDX_Control(pDX,IDC_DELETE,       m_buttonDelete);
  DDX_Control(pDX,IDOK,             m_buttonOk);
  DDX_Control(pDX,IDCANCEL,         m_buttonCancel);

  if(pDX->m_bSaveAndValidate == FALSE)
  {
    CWnd* w1 = GetDlgItem(IDC_DIRECTORY);
    CWnd* w2 = GetDlgItem(IDC_CHOOSE);
    w1->EnableWindow(((m_type == OBT_PRIVATE) && m_canSetDirectory) || ((m_type == OBT_SHARED) && m_admin));
    w2->EnableWindow(((m_type == OBT_PRIVATE) && m_canSetDirectory) || ((m_type == OBT_SHARED) && m_admin));
  }
}

BEGIN_MESSAGE_MAP(ConfigOutboxDlg, StyleDialog)
  ON_CBN_SELCHANGE(IDC_TYPE,          &ConfigOutboxDlg::OnCbnSelchangeType)
  ON_EN_KILLFOCUS (IDC_DIRECTORY,     &ConfigOutboxDlg::OnEnChangeDirectory)
  ON_BN_CLICKED   (IDC_CHOOSE,        &ConfigOutboxDlg::OnBnClickedChoose)
  ON_CBN_SELCHANGE(IDC_SELECT,        &ConfigOutboxDlg::OnCbnSelchangeSelect)
  ON_EN_CHANGE    (IDC_MAXFILES,      &ConfigOutboxDlg::OnEnChangeMaxFiles)
  ON_NOTIFY       (NM_DBLCLK,IDC_LIST,&ConfigOutboxDlg::OnLvnItemDoubleClick)
  ON_BN_CLICKED   (IDC_DELETE,        &ConfigOutboxDlg::OnBnClickedDelete)
  ON_BN_CLICKED   (IDOK,              &ConfigOutboxDlg::OnBnClickedOk)
  ON_BN_CLICKED   (IDCANCEL,          &ConfigOutboxDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL
ConfigOutboxDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(g_message[MESS_CONFIGUREBOX][g_lang]);

  InitTexts();
  SetDefaults();
  ReadConfig();
  UpdateData(FALSE);

  m_init = true;
  return TRUE;
}

void
ConfigOutboxDlg::InitTexts()
{
  m_staticTypeOutbox.SetWindowText(g_message[MESS_TYPEBOX   ][g_lang]);
  m_staticMapMail   .SetWindowText(g_message[MESS_MAPMAIL   ][g_lang]);
  m_staticInitSelect.SetWindowText(g_message[MESS_INITSELECT][g_lang]);
  m_staticWarnAt    .SetWindowText(g_message[MESS_WARNINGAT ][g_lang]);
  m_staticHistory   .SetWindowText(g_message[MESS_HISTORY   ][g_lang]);

  m_buttonDelete    .SetWindowText(g_message[MESS_DELETE    ][g_lang]);
  m_buttonOk        .SetWindowText(g_message[MESS_OK        ][g_lang]);
  m_buttonCancel    .SetWindowText(g_message[MESS_CANCEL    ][g_lang]);
}

void
ConfigOutboxDlg::SetDefaults()
{
  if(m_canSetDirectory)
  {
    m_comboType.AddString(g_message[SEL_NOOUTBOX  ][g_lang]); // Do not use an OUTBOX
    m_comboType.AddString(g_message[SEL_USEAPPDATA][g_lang]); // Use APPDATA\PostMail\Outbox
    m_comboType.AddString(g_message[SEL_USEPRIVATE][g_lang]); // Use your private outbox
  }
  else
  {
    m_buttonChoose.EnableWindow(FALSE);
    CWnd* w = GetDlgItem(IDC_DIRECTORY);
    w->EnableWindow(FALSE);
  }
  // Use the outbox of the organization
  m_comboType.AddString(g_message[SEL_USEORG][g_lang]);
  m_comboType.SetCurSel(0);

  m_comboSelection.AddString(g_message[SEL_EMAILTODAY  ][g_lang]); // Email of today
  m_comboSelection.AddString(g_message[SEL_EMAILWEEK   ][g_lang]); // Email of the last 7 days
  m_comboSelection.AddString(g_message[SEL_EMAILMONTH  ][g_lang]); // Email of the last 31 days
  m_comboSelection.AddString(g_message[SEL_EMAIL3MONTHS][g_lang]); // Email of the last 3 months
  m_comboSelection.AddString(g_message[SEL_EMAIL6MONTHS][g_lang]); // Email of the last 6 months
  m_comboSelection.AddString(g_message[SEL_EMAILYEAR   ][g_lang]); // Email of the last year
  m_comboSelection.AddString(g_message[SEL_EMAILALL    ][g_lang]); // All emails
  m_comboSelection.SetCurSel(1);

  m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EDITLABELS|LVS_EX_SINGLEROW);
  m_list.InsertColumn(0,g_message[MESS_OUTBOXDIRS][g_lang],LVCFMT_LEFT,450);
}

void
ConfigOutboxDlg::ReadConfig()
{
  m_admin = theApp.GetProfiles().GetWriteAccess();
  Profiles& prof = theApp.GetProfiles();

  m_type      = prof.GetOutboxType();
  m_selection = prof.GetOutboxSelection();
  m_maxFiles  = prof.GetOutboxMAX();
  if(m_canSetDirectory)
  {
    m_directory = prof.GetOutboxDirectory();
  }
  if(m_admin && m_type == OBT_SHARED)
  {
    m_directory = prof.GetCommonOutbox();
  }

  int num = 0;
  BoxHistory& history = prof.GetOutboxHistory();
  for(auto& directory : history)
  {
    m_list.InsertItem(LVIF_TEXT|LVIF_STATE,num++,directory,0,0,0,0);
  }
  if(num > 1)
  {
    m_list.SetSelectionMark(0);
    m_list.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
  }
  m_comboSelection.SetCurSel(m_selection);
  m_comboType.SetCurSel(m_canSetDirectory ? m_type : 0);
  SetType(m_type);
}

bool
ConfigOutboxDlg::SaveConfig()
{
  if(CheckDirectory(m_directory))
  {
    Profiles& prof = theApp.GetProfiles();

    prof.SetTypeOfOutbox(m_type);
    prof.SetOutboxSelection(m_selection);
    prof.SetOutboxMAX(m_maxFiles);
    if(m_canSetDirectory)
    {
      prof.SetOutboxDirectory(m_directory);
    }
    
    // Write history
    prof.ResetOutboxHistory();
    int ind = 0;
    for(ind = 0;ind < m_list.GetItemCount() && ind < MAX_OUTBOX_HISTORY - 1;++ind)
    {
      XString directory = m_list.GetItemText(ind,0);
      prof.AddOutboxHistory(directory.GetString());
    }
    prof.WriteProfiles();
  }
  else
  {
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_NODIRECTORY][g_lang] + m_directory,WhoAmI,MB_OK|MB_ICONERROR);
    return false;
  }
  return true;
}

bool
ConfigOutboxDlg::CheckDirectory(const XString& p_directory)
{
  if(_taccess(p_directory.GetString(),0) == 0)
  {
    return true;
  }
  return theApp.GetProfiles().CreateProfileMap(m_directory);
}

void 
ConfigOutboxDlg::AddHistory(const XString& p_directory)
{
  XString top = m_list.GetItemText(0,0);
  if(top.CompareNoCase(p_directory))
  {
    m_list.InsertItem(LVIF_TEXT|LVIF_STATE,0,m_directory.GetString(),0,0,0,0);
    m_list.SetSelectionMark(0);
    m_list.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
  }
}

// ConfigOutboxDlg message handlers

void 
ConfigOutboxDlg::OnCbnSelchangeType()
{
  int ind = m_comboType.GetCurSel();
  if(ind >= 0)
  {
    switch(ind)
    {
      case 0: m_type = OBT_NO_OUTBOX;   break;
      case 1: m_type = OBT_APPDATA;     break;
      case 2: m_type = OBT_PRIVATE;     break;
      case 3: m_type = OBT_SHARED;      break;
    }
    SetType(m_type);
    UpdateData(FALSE);
  }
}

void
ConfigOutboxDlg::SetType(BoxType p_type)
{
  switch(p_type)
  {
    case OBT_NO_OUTBOX:  m_directory.Empty();
                         break;
    case OBT_APPDATA:    // APPDATA subdirectory is hardcoded "OutBox"
                         m_directory = theApp.GetProfiles().GetProfileMap() + _T("OutBox");
                         AddHistory(m_directory);
                         break;
    case OBT_PRIVATE:    break;
    case OBT_SHARED:     if (m_admin)
                         {
                           m_directory = theApp.GetProfiles().GetCommonOutbox();
                         }
                         else
                         {
                           ((OutBoxDlg*)GetParent())->GetOutbox().ComputeDirectory();
                           m_directory = ((OutBoxDlg*)GetParent())->GetOutbox().GetDirectory();
                         }
                         break;
  }
}

void 
ConfigOutboxDlg::OnEnChangeDirectory()
{
  Profiles& prof = theApp.GetProfiles();

  UpdateData();
  if(m_admin)
  {
    prof.SetCommonOutbox(m_directory);
    prof.WriteCentralSettings();
  }
  prof.SetOutboxDirectory(m_directory);
  AddHistory(m_directory);
}

void 
ConfigOutboxDlg::OnBnClickedChoose()
{
  MapDialog dlg;
  if(dlg.Browse(GetSafeHwnd(),g_message[MESS_CHOOSEOUTBOXDIR][g_lang],m_directory.GetString()))
  {
    XString path = dlg.GetPath();
  
    if(CheckDirectory(path))
    {
      CWnd* dir = GetDlgItem(IDC_DIRECTORY);
      dir->SetWindowText(path);
      OnEnChangeDirectory();
    }
  }
}

void 
ConfigOutboxDlg::OnCbnSelchangeSelect()
{
  int ind = m_comboSelection.GetCurSel();
  if(ind >= 0)
  {
    m_selection = (DateSel) ind;
  }
}

void
ConfigOutboxDlg::OnEnChangeMaxFiles()
{
  UpdateData();
  if(m_maxFiles < MAX_OUTBOX)
  {
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_GT2000][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    m_maxFiles = MAX_OUTBOX;
  }
  if(m_maxFiles > MAX_NTFS_FILES)
  {
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_ST150000][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    m_maxFiles = MAX_NTFS_FILES;
  }
  UpdateData(FALSE);
}

void 
ConfigOutboxDlg::OnLvnItemDoubleClick(NMHDR *pNMHDR, LRESULT *pResult)
{
  if(m_init)
  {
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    XString path = m_list.GetItemText(pNMLV->iItem,0);

    m_init = false;
    m_list.DeleteItem(pNMLV->iItem);
    m_init = true;

    m_directory = path;
    UpdateData(FALSE);
    OnEnChangeDirectory();
  }
  *pResult = 0;
}

void
ConfigOutboxDlg::OnBnClickedDelete()
{
  int item = m_list.GetSelectionMark();
  if(item >= 0)
  {
    XString text = m_list.GetItemText(item,0);
    XString vraag;
    vraag.Format(g_message[MESS_DELMAILBOX][g_lang],text.GetString());
    if(theApp.WideMessageBox(GetSafeHwnd(),vraag,_T(PRODUCT_NAME),MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
    {
      m_list.DeleteItem(item);
    }
  }
}

void 
ConfigOutboxDlg::OnBnClickedOk()
{
  if(SaveConfig())
  {
    OnOK();
  }
}

void 
ConfigOutboxDlg::OnBnClickedCancel()
{
  OnCancel();
}
