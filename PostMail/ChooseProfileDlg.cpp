/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ChooseProfileDlg.cpp
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
#include "PostMail.h"
#include "ChooseProfileDlg.h"
#include "ProfileManagamentDlg.h"
#include "Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ChooseProfileDlg dialog

IMPLEMENT_DYNAMIC(ChooseProfileDlg, StyleDialog)

ChooseProfileDlg::ChooseProfileDlg(CWnd* pParent,Profiles& p_profiles,bool p_original)
                 :StyleDialog(ChooseProfileDlg::IDD, pParent)
                 ,m_profiles(p_profiles)
                 ,m_original(p_original)
                 ,m_reread(false)
                 ,m_asking(false)
                 ,m_brush(NULL)
{
}

ChooseProfileDlg::~ChooseProfileDlg()
{
  if(m_brush)
  {
    delete m_brush;
    m_brush = NULL;
  }
}

void 
ChooseProfileDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Control(pDX,IDC_LIJST,    m_list);
  DDX_Control(pDX,IDC_PROFILE,  m_editProfile,m_profile);
  DDX_Control(pDX,IDC_SERVER,   m_editServer, m_server);
  DDX_Control(pDX,IDC_USER,     m_editUser,   m_user);
  DDX_Control(pDX,IDC_EMAIL,    m_editEmail,  m_email);
  DDX_Control(pDX,IDC_VRAGEN,   m_buttonSession);  
  DDX_Control(pDX,IDC_ORIGINAL, m_buttonOriginal);

  DDX_Control(pDX,IDC_STATIC_LINE1,   m_statLine1);
  DDX_Control(pDX,IDC_STATIC_LINE3,   m_statLine3);
  DDX_Control(pDX,IDC_ST_LIST,        m_groupList);

  DDX_Control(pDX,IDC_ST_PROFILE,     m_statProfile);
  DDX_Control(pDX,IDC_ST_LOGINNAME,   m_statUser);
  DDX_Control(pDX,IDC_ST_SERVER,      m_statServer);

  DDX_Control(pDX, IDC_CONFIG,        m_buttonConfig);
  DDX_Control(pDX, IDOK,              m_buttonOK);
  DDX_Control(pDX, IDCANCEL,          m_buttonCancel);
}


BEGIN_MESSAGE_MAP(ChooseProfileDlg, StyleDialog)
  ON_WM_CTLCOLOR()
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIJST,  &ChooseProfileDlg::OnLvnItemchangedList)
  ON_NOTIFY(NM_DBLCLK,       IDC_LIJST,  &ChooseProfileDlg::OnLvnDoubleClick)
  ON_BN_CLICKED(IDC_VRAGEN,              &ChooseProfileDlg::OnBnClickedVragen)
  ON_BN_CLICKED(IDC_CONFIG,              &ChooseProfileDlg::OnBnClickedConfig)
  ON_BN_CLICKED(IDC_ORIGINAL,            &ChooseProfileDlg::OnBnClickedOriginal)
END_MESSAGE_MAP()

BOOL
ChooseProfileDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(g_message[MESS_MISSINGPROFILE][g_lang]);
 
  m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EDITLABELS);
  m_list.InsertColumn(0,g_message[MESS_PROFILE][g_lang],LVCFMT_LEFT,180);
  m_list.InsertColumn(1,g_message[MESS_SENDER ][g_lang],LVCFMT_LEFT,180);

  InitTexts();
  FillList();
  UpdateData(FALSE);

  // Show the original button?
  m_buttonOriginal.ShowWindow(m_original ? SW_SHOW : SW_HIDE);

  return TRUE;
}

void
ChooseProfileDlg::InitTexts()
{
  SetDlgItemText(IDC_STATIC_LINE1,  g_message[MESS_CHOOSEPROFILE ][g_lang]);
  SetDlgItemText(IDC_STATIC_LINE3,  g_message[MESS_MANAGEPROFILE ][g_lang]);
  SetDlgItemText(IDC_ST_LIST,       g_message[MESS_PROFILELIST   ][g_lang]);
  SetDlgItemText(IDC_ST_CHOSEN,     g_message[MESS_CHOSENPROFILE ][g_lang]);
  SetDlgItemText(IDC_ST_PROFILE,    g_message[MESS_PROFILE       ][g_lang]);
  SetDlgItemText(IDC_ST_SERVER,     g_message[MESS_MAILSERVER    ][g_lang]);
  SetDlgItemText(IDC_ST_SENDER,     g_message[MESS_SENDER        ][g_lang]);
  SetDlgItemText(IDC_ST_LOGINNAME,  g_message[MESS_LOGINNAME     ][g_lang]);
  SetDlgItemText(IDC_VRAGEN,        g_message[MESS_DONT_ASK_AGAIN][g_lang]);
  SetDlgItemText(IDC_ORIGINAL,      g_message[MESS_ORIGINAL      ][g_lang]);
  SetDlgItemText(IDC_CONFIG,        g_message[MESS_MANAGE        ][g_lang]);
  SetDlgItemText(IDOK,              g_message[MESS_OK            ][g_lang]);
  SetDlgItemText(IDCANCEL,          g_message[MESS_CANCEL        ][g_lang]);
}

void
ChooseProfileDlg::FillList()
{
  // Begin by emptying the list
  m_list.DeleteAllItems();
  int total = m_profiles.GetNumberOfProfiles();

  // checking
  if(total == 0 && m_profiles.GetDefaultMailServer().IsEmpty())
  {
    XString melding = g_message[MESS_NO_PROFILES][g_lang];
    theApp.WideMessageBox(GetSafeHwnd(),melding,WhoAmI,MB_OK|MB_ICONEXCLAMATION);

    ProfileManagamentDlg dlg(this,m_profiles);
    dlg.DoModal();

    // Recalculate the number
    total = m_profiles.GetNumberOfProfiles();
  }

  // If we have a default mailserver and no profiles,
  // we will show no dialog at all
  if(total == 0 && !m_profiles.GetDefaultMailServer().IsEmpty())
  {
    OnCancel();
  }

  // Filling
  for(int ind = 0;ind < total; ++ind)
  {
    Profile* profile = m_profiles.GetProfile(ind + 1);

    m_list.InsertItem(LVIF_TEXT|LVIF_STATE, ind, profile->m_profileName, 0, 0, 0, 0);
    m_list.SetItemText(ind,1,profile->m_emailAddress);
  }

  // Selecting the first line
  if(m_list.GetSelectionMark() < 0)
  {
    m_list.SetSelectionMark(0);
    m_list.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetFocus();
  }
}

void
ChooseProfileDlg::LoadProfile(Profile* p_profile)
{
  m_profile = p_profile->m_profileName;
  m_email   = p_profile->m_emailAddress;
  m_user    = p_profile->m_loginUser;

  m_server.Format(_T("%s:%d")
                 ,p_profile->m_smtpServer.GetString()
                 ,p_profile->m_smtpPort);

  if(m_user.IsEmpty())
  {
    m_user = _T("<Single sign-on>");
  }
  UpdateData(FALSE);
}

// ChooseProfileDlg message handlers

void 
ChooseProfileDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  int item = pNMLV->iItem;
  if(item >= 0 )
  {
    XString naam = m_list.GetItemText(item,0);
    Profile* profile = m_profiles.GetProfile(naam);
    LoadProfile(profile);
  }
  *pResult = 0;
}

void
ChooseProfileDlg::OnLvnDoubleClick(NMHDR *pNMHDR, LRESULT* /*pResult*/)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  int item = pNMLV->iItem;
  if(item >= 0)
  {
    XString naam = m_list.GetItemText(item,0);
    Profile* profile = m_profiles.GetProfile(naam);
    LoadProfile(profile);
    m_profiles.SetChosenProfile(profile->m_number);
    OnOK();
  }
}

void
ChooseProfileDlg::OnBnClickedVragen()
{
  m_asking = m_buttonSession.GetCheck() > 0;
  m_profiles.SetDoNotAskAgain(m_asking);
}

void    
ChooseProfileDlg::OnBnClickedConfig()
{
  ProfileManagamentDlg dlg(this,m_profiles);
  dlg.DoModal();

  FillList();
  UpdateData(FALSE);
}

void
ChooseProfileDlg::OnBnClickedOriginal()
{
  // ASKS: The chosen profile will be removed and all original data as given by the starting program will be re-read. Do you want to continue?
  if(theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_ASKDELPROFILE][g_lang]
                          ,WhoAmI
                          ,MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES)
  {
    m_reread = true;
    StyleDialog::OnOK();
  }
}

void
ChooseProfileDlg::OnOK()
{
  int item = m_list.GetSelectionMark();
  if(item >= 0)
  {
    XString naam = m_list.GetItemText(item,0);
    Profile* profile = m_profiles.GetProfile(naam);
    LoadProfile(profile);
    m_profiles.SetChosenProfile(profile->m_number);
    StyleDialog::OnOK();
  }
}

void 
ChooseProfileDlg::SetBkColor(COLORREF p_colorref)
{
  if(m_brush)
  {
    delete m_brush;
  }
  m_brush = new CBrush(p_colorref);
  Invalidate();
}

HBRUSH
ChooseProfileDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  if(nCtlColor == CTLCOLOR_DLG)
  {
    if(m_brush)
    {
      return (HBRUSH)*m_brush;
    }
  }
  return StyleDialog::OnCtlColor(pDC,pWnd,nCtlColor);
}
