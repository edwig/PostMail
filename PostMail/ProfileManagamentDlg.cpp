/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ProfileManagamentDlg.cpp
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
#include "ProfileManagamentDlg.h"
#include "Message.h"
#include "Version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ProfileManagamentDlg dialog

IMPLEMENT_DYNAMIC(ProfileManagamentDlg,StyleDialog)

ProfileManagamentDlg::ProfileManagamentDlg(CWnd* p_parent,Profiles& p_profiles)
                     :StyleDialog(ProfileManagamentDlg::IDD, p_parent)
                     ,m_profiles(p_profiles)
                     ,m_brush(NULL)
{
  m_new       = false;
  m_editting  = false;
  m_personal  = false;
  m_canchange = false;
  m_number    = 0;
}

ProfileManagamentDlg::~ProfileManagamentDlg()
{
  if(m_brush)
  {
    delete m_brush;
    m_brush = NULL;
  }
}

void ProfileManagamentDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST,        m_list);
  DDX_Control(pDX, IDC_NIEUW,       m_buttonNew);
  DDX_Control(pDX, IDC_WIJZIGEN,    m_buttonModify);
  DDX_Control(pDX, IDC_VERWIJDER,   m_buttonDelete);
  DDX_Control(pDX, IDC_BOVEN,       m_buttonUp);
  DDX_Control(pDX, IDC_BENEDEN,     m_buttonDown);
  DDX_Control(pDX, IDC_PROFILENAME, m_editProfileName, m_profileName);
  DDX_Control(pDX, IDC_EMAIL,       m_editEmailAddress,m_emailAddress);
  DDX_Control(pDX, IDC_BCC,         m_editBCCAddress,  m_bccAddress);
  DDX_Control(pDX, IDC_SERVER,      m_comboServer); // m_smtpServer
  DDX_Control(pDX, IDC_PORT,        m_comboPort);   // m_smtpPort
  DDX_Control(pDX, IDC_STANDARD,    m_checkStandard);
  DDX_Control(pDX, IDC_USELOGIN,    m_checkUseLogin);
  DDX_Control(pDX, IDC_USER,        m_editUserLogin,m_userLogin);
  DDX_Control(pDX, IDC_WACHTWOORD1, m_editPassword1,m_password1);
  DDX_Control(pDX, IDC_WACHTWOORD2, m_editPassword2,m_password2);
  DDX_Control(pDX, IDC_FONT,        m_buttonFont);
  DDX_Control(pDX, IDOK,            m_buttonOK);
  DDX_Control(pDX, IDCANCEL,        m_buttonCancel);

  DDX_Control(pDX, IDC_STATIC_PROFILENAME, m_statProfileName);
  DDX_Control(pDX, IDC_STATIC_EMAIL,       m_statEmail);
  DDX_Control(pDX, IDC_STATIC_BCC,         m_statBcc);
  DDX_Control(pDX, IDC_STATIC_SERVER,      m_statServer);
  DDX_Control(pDX, IDC_STATIC_PORT,        m_statPort);
  DDX_Control(pDX, IDC_STATIC_STANDARD,    m_statStandard);
  DDX_Control(pDX, IDC_STATIC_LOGIN,       m_statLogin);
  DDX_Control(pDX, IDC_STATIC_USER,        m_statUser);
  DDX_Control(pDX, IDC_STATIC_WW1,         m_statWW1);
  DDX_Control(pDX, IDC_STATIC_WW2,         m_statWW2);

  SetMutable();
}

BEGIN_MESSAGE_MAP(ProfileManagamentDlg, StyleDialog)
  ON_WM_CTLCOLOR()
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST,  &ProfileManagamentDlg::OnLvnItemchangedList)
  ON_NOTIFY(LVN_ITEMACTIVATE,IDC_LIST,  &ProfileManagamentDlg::OnLvnItemActivate)
  ON_BN_CLICKED(IDC_NIEUW,              &ProfileManagamentDlg::OnBnClickedNew)
  ON_BN_CLICKED(IDC_WIJZIGEN,           &ProfileManagamentDlg::OnBnClickedChange)
  ON_BN_CLICKED(IDC_VERWIJDER,          &ProfileManagamentDlg::OnBnClickedDelete)
  ON_BN_CLICKED(IDC_BOVEN,              &ProfileManagamentDlg::OnBnClickedUp)
  ON_BN_CLICKED(IDC_BENEDEN,            &ProfileManagamentDlg::OnBnClickedDown)
  ON_EN_CHANGE (IDC_PROFILENAME,        &ProfileManagamentDlg::OnEnChangeProfileName)
  ON_EN_CHANGE (IDC_EMAIL,              &ProfileManagamentDlg::OnEnChangeEmail)
  ON_EN_CHANGE (IDC_BCC,                &ProfileManagamentDlg::OnEnChangeBCC)
  ON_CBN_KILLFOCUS (IDC_SERVER,         &ProfileManagamentDlg::OnCbnSelchangeServer)
  ON_CBN_EDITCHANGE(IDC_SERVER,         &ProfileManagamentDlg::OnCbnEditchangeServer)
  ON_CBN_KILLFOCUS (IDC_PORT,           &ProfileManagamentDlg::OnCbnSelchangePort)
  ON_CBN_EDITCHANGE(IDC_PORT,           &ProfileManagamentDlg::OnCbnEditchangePort)
  ON_BN_CLICKED(IDC_STANDARD,           &ProfileManagamentDlg::OnBnClickedStandard)
  ON_BN_CLICKED(IDC_USELOGIN,           &ProfileManagamentDlg::OnBnClickedUselogin)
  ON_EN_CHANGE (IDC_USER,               &ProfileManagamentDlg::OnEnChangeUserLogin)
  ON_EN_CHANGE (IDC_WACHTWOORD1,        &ProfileManagamentDlg::OnEnChangePassword1)
  ON_EN_CHANGE (IDC_WACHTWOORD2,        &ProfileManagamentDlg::OnEnChangePassword2)
  ON_BN_CLICKED(IDC_FONT,               &ProfileManagamentDlg::OnBnClickedFont)
  ON_BN_CLICKED(IDCANCEL,               &ProfileManagamentDlg::OnCancel)
END_MESSAGE_MAP()

BOOL
ProfileManagamentDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(g_message[MESS_MANPROFILES][g_lang]);

  m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EDITLABELS);
  m_list.InsertColumn(0,g_message[MESS_PROFILE][g_lang],LVCFMT_LEFT,180);
  m_list.InsertColumn(1,g_message[MESS_SENDER ][g_lang],LVCFMT_LEFT,180);

  InitTexts();
  FillList();
  SetMutable();

  return TRUE;
}

void
ProfileManagamentDlg::InitTexts()
{
  m_buttonNew      .SetWindowText(g_message[MESS_NEW   ][g_lang]);
  m_buttonModify   .SetWindowText(g_message[MESS_MODIFY][g_lang]);
  m_buttonDelete   .SetWindowText(g_message[MESS_DELETE][g_lang]);
  m_buttonUp       .SetWindowText(g_message[MESS_UP    ][g_lang]);
  m_buttonDown     .SetWindowText(g_message[MESS_DOWN  ][g_lang]);

  m_statProfileName.SetWindowText(g_message[MESS_PROFILE_NAME  ][g_lang]);
  m_statEmail      .SetWindowText(g_message[MESS_EMAIL_ADDRESS ][g_lang]);
  m_statBcc        .SetWindowText(g_message[MESS_BCC_ADDRESS   ][g_lang]);
  m_statServer     .SetWindowText(g_message[MESS_SMTP_SERVER   ][g_lang]);
  m_statPort       .SetWindowText(g_message[MESS_SMTP_PORT     ][g_lang]);
  m_statStandard   .SetWindowText(g_message[MESS_SET_AS_DEFAULT][g_lang]);
  m_statLogin      .SetWindowText(g_message[MESS_USE_LOGIN     ][g_lang]);
  m_statUser       .SetWindowText(g_message[MESS_USER_LOGIN    ][g_lang]);
  m_statWW1        .SetWindowText(g_message[MESS_PASSWORD      ][g_lang]);
  m_statWW2        .SetWindowText(g_message[MESS_PASSWORD_AGAIN][g_lang]);
  m_buttonOK       .SetWindowText(g_message[MESS_OK            ][g_lang]);
  m_buttonCancel   .SetWindowText(g_message[MESS_CANCEL        ][g_lang]);
}

void
ProfileManagamentDlg::FillList()
{
  // Empty the list first
  m_list.DeleteAllItems();

  // Fill
  int aantal = m_profiles.GetNumberOfProfiles();
  for(int ind = 1;ind <= aantal; ++ind)
  {
    Profile* profile = m_profiles.GetProfile(ind);

    m_list.InsertItem(LVIF_TEXT|LVIF_STATE, ind, profile->m_profileName, 0, 0, 0, 0);
    m_list.SetItemText(ind,1,profile->m_emailAddress);
  }
  // Setting the first line
  if(m_list.GetSelectionMark() < 0)
  {
    m_list.SetSelectionMark(0);
    m_list.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetFocus();
  }

  // Getting the default font
  m_defaultFont = m_profiles.GetDefaultFont();
}

void
ProfileManagamentDlg::SetMutable()
{
  CWnd* wnd = nullptr;

  int row    = m_list.GetHotItem();
  int aantal = m_profiles.GetNumberOfProfiles();

  // Fields
  wnd = GetDlgItem(IDC_PROFILENAME);  wnd->EnableWindow(m_editting && m_new);
  wnd = GetDlgItem(IDC_EMAIL);        wnd->EnableWindow(m_editting);
  wnd = GetDlgItem(IDC_BCC);          wnd->EnableWindow(m_editting);
  wnd = GetDlgItem(IDC_SERVER);       wnd->EnableWindow(m_editting);
  wnd = GetDlgItem(IDC_PORT);         wnd->EnableWindow(m_editting);
  wnd = GetDlgItem(IDC_STANDARD);     wnd->EnableWindow(m_editting);
  wnd = GetDlgItem(IDC_USELOGIN);     wnd->EnableWindow(m_editting);
  wnd = GetDlgItem(IDC_USER);         wnd->EnableWindow(m_editting && m_useLogin);
  wnd = GetDlgItem(IDC_WACHTWOORD1);  wnd->EnableWindow(m_editting && m_useLogin);
  wnd = GetDlgItem(IDC_WACHTWOORD2);  wnd->EnableWindow(m_editting && m_useLogin);
  wnd = GetDlgItem(IDC_FONT);         wnd->EnableWindow(m_editting == false);

  // Buttons
  wnd = GetDlgItem(IDC_WIJZIGEN);     wnd->EnableWindow(!m_editting && m_canchange);
  wnd = GetDlgItem(IDC_VERWIJDER);    wnd->EnableWindow(!m_editting && m_canchange);
  wnd = GetDlgItem(IDC_BOVEN);        wnd->EnableWindow(!m_editting && m_canchange && (row > 0));
  wnd = GetDlgItem(IDC_BENEDEN);      wnd->EnableWindow(!m_editting && m_canchange && (row < aantal - 1));
  m_buttonOK.EnableWindow(!m_editting);

  // SMTP e-mail server
  wnd = GetDlgItem(IDC_SERVER);
  XString defProfile = m_profiles.GetDefaultMailServer();
  XString chkProfile = m_smtpServer;
  m_profiles.CheckMailServer(chkProfile);
  if(m_editting && 
          (defProfile.IsEmpty() || 
          (m_smtpServer != chkProfile) || 
           m_profiles.GetMailServers().size() > 1))
  {
    wnd->EnableWindow(TRUE);
  }
  else
  {
    wnd->EnableWindow(FALSE);
  }

  // User field
  if(m_editting && m_useLogin)
  {
    if(m_userLogin.GetAt(0) == _T('<'))
    {
      m_userLogin.Empty();
    }
  }
  else if(m_userLogin.IsEmpty())
  {
    m_userLogin = _T("<Single sign-on>");
  }
  Invalidate(FALSE);
  m_list.Invalidate(FALSE);
}

bool
ProfileManagamentDlg::CheckPasswords()
{
  if(m_useLogin)
  {
    if(m_password1 != m_password2)
    {
      theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_PASSWORDS][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
      return false;
    }
  }
  return true;
}

void
ProfileManagamentDlg::LoadServers()
{
  m_comboServer.ResetContent();
  MailServers& servers = m_profiles.GetMailServers();
  for(unsigned ind = 0;ind < servers.size(); ++ind)
  {
    m_comboServer.AddString(servers[ind]);
  }
  // If filled, select the first one
  if(!servers.empty())
  {
    m_comboServer.SetCurSel(0);
  }
}

void
ProfileManagamentDlg::LoadPorts()
{
  m_comboPort.ResetContent();
  m_comboPort.AddString(g_message[MESS_SMTP25 ][g_lang]); // "25 Standard insecure SMTP"
  m_comboPort.AddString(g_message[MESS_SMTP587][g_lang]); // "587 Standard secure SMTP"
}

void
ProfileManagamentDlg::MakeEmpty()
{
  m_number = 0;
  m_profileName.Empty();
  m_emailAddress.Empty();
  m_bccAddress.Empty();
  m_smtpServer = m_profiles.GetDefaultMailServer();
  m_smtpPort   = DEFAULT_SMTP_PORT;
  m_userLogin.Empty();
  m_password1.Empty();
  m_password2.Empty();
  m_isStandard  = false;
  m_useLogin    = false;
  m_personal    = false;
  m_canchange   = false;
  m_checkStandard.SetCheck(FALSE);
  m_checkUseLogin.SetCheck(FALSE);
  m_comboServer.ResetContent();
  UpdateData(FALSE);
}

void
ProfileManagamentDlg::LoadProfile(Profile* p_profile)
{
  // Load the first profile
  m_number       = p_profile->m_number;
  m_profileName  = p_profile->m_profileName;
  m_emailAddress = p_profile->m_emailAddress;
  m_bccAddress   = p_profile->m_bccAddress;
  m_smtpServer   = p_profile->m_smtpServer;
  m_smtpPort     = p_profile->m_smtpPort;
  m_isStandard   = p_profile->m_isStandard;
  m_useLogin     = p_profile->m_useLogin;
  m_userLogin    = p_profile->m_loginUser;
  m_password1    = p_profile->m_password;
  m_password2    = p_profile->m_password;
  m_personal     = p_profile->m_private;
  m_canchange    = p_profile->m_mutable;

  m_checkStandard.SetCheck(m_isStandard == true);
  m_checkUseLogin.SetCheck(m_useLogin == true);

  // Fill combo for servers
  LoadServers();
  // Refresh the server display
  int ind = m_comboServer.FindStringExact(-1,m_smtpServer);
  if(ind >= 0)
  {
    m_comboServer.SetCurSel(ind);
  }
  else
  {
    m_comboServer.SetWindowText(m_smtpServer.GetString());
  }

  // Fill combo for ports
  LoadPorts();
  XString port;
  if(m_smtpPort == DEFAULT_SMTP_PORT || m_smtpPort == SECURE_SMTP_PORT)
  {
    int index = (m_smtpPort == DEFAULT_SMTP_PORT) ? 0 : 1;
    m_comboPort.SetCurSel(index);
  }
  else
  {
    port.Format(_T("%d"),m_smtpPort);
    m_comboPort.SetWindowText(port.GetString());
  }

  UpdateData(FALSE);

  // Adjust the buttons
  SetMutable();
}

void
ProfileManagamentDlg::SaveNewProfile()
{
  if(!CheckPasswords())
  {
    return;
  }
  Profile prf;
  prf.m_number       = m_number;
  prf.m_profileName  = m_profileName;
  prf.m_emailAddress = m_emailAddress;
  prf.m_bccAddress   = m_bccAddress;
  prf.m_smtpServer   = m_smtpServer;
  prf.m_smtpPort     = m_smtpPort;
  prf.m_isStandard   = m_isStandard;
  prf.m_useLogin     = m_useLogin;
  prf.m_loginUser    = m_userLogin;
  prf.m_password     = m_password1;
  prf.m_private      = m_personal;
  prf.m_mutable      = true;

  if(m_profiles.AddProfile(&prf) == false)
  {
    // ProfileName already exists: <profilename>
    XString melding = g_message[MESS_PROFILE_DOUBLE][g_lang] + m_profileName;
    theApp.WideMessageBox(GetSafeHwnd(),melding,WhoAmI,MB_OK|MB_ICONERROR);
  }
  else
  {
    // Save the profiles
    m_profiles.WriteProfiles();
  }
  FillList();
}

bool
ProfileManagamentDlg::SaveChangedProfile(Profile *p_profile)
{
  if(!CheckPasswords())
  {
    return false;
  }
  p_profile->m_emailAddress = m_emailAddress;
  p_profile->m_bccAddress   = m_bccAddress;
  p_profile->m_smtpServer   = m_smtpServer;
  p_profile->m_smtpPort     = m_smtpPort;
  p_profile->m_isStandard   = m_isStandard;
  p_profile->m_useLogin     = m_useLogin;
  p_profile->m_loginUser    = m_userLogin;
  p_profile->m_password     = m_password1;

  return m_profiles.WriteProfiles();
}

// Go to profile x in the list
void
ProfileManagamentDlg::GoToProfile(int p_profile,bool p_focus)
{
  XString naam = m_list.GetItemText(p_profile,0);
  Profile* profiel = m_profiles.GetProfile(naam);
  LoadProfile(profiel);

  SetMutable();

  if(p_focus)
  {
    // Reset focus for all lines
    for(int ind = 0;ind < m_list.GetItemCount(); ++ind)
    {
      m_list.SetItemState(ind,0,LVIS_SELECTED|LVIS_FOCUSED);
    }
    // Focus op current line
    m_list.SetSelectionMark(p_profile);
    m_list.SetItemState(p_profile,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetFocus();
  }
}

// ProfileManagamentDlg message handlers

void 
ProfileManagamentDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  int item = pNMLV->iItem;
  if(item >= 0 && m_editting == false)
  {
    GoToProfile(item);
  }
  *pResult = 0;
}

void 
ProfileManagamentDlg::OnLvnItemActivate(NMHDR* /*pNMHDR*/,LRESULT* /*pResult*/)
{
  SetMutable();
}

void 
ProfileManagamentDlg::OnBnClickedNew()
{
  if(m_editting)
  {
    // We are now a SAVE button
    m_profiles.SetDefaultFont(m_defaultFont);
    if(m_new)
    {
      SaveNewProfile();
    }
    else
    {
      Profile* profile = m_profiles.GetProfile(m_profileName);
      if(profile)
      {
        if(!SaveChangedProfile(profile))
        {
          return;
        }
      }
    }
    MakeEmpty();
    m_new      = false;
    m_editting = false;
    m_buttonNew.SetWindowText(g_message[MESS_NEW][g_lang]);// &New

    int profile = m_list.GetItemCount() - 1;
    GoToProfile(profile,true);
    UpdateData(FALSE);
  }
  else
  {
    // Create new profile
    MakeEmpty();
    LoadServers();
    LoadPorts();
    m_new       = true;
    m_editting  = true;
    m_personal  = true;
    m_canchange = true;
    m_buttonNew.SetWindowText(g_message[MESS_SAVE][g_lang]); // &Save

    CWnd* wnd = GetDlgItem(IDC_PROFILENAME);
    wnd->SetFocus();

    UpdateData(FALSE);
    SetMutable();
  }
}

void 
ProfileManagamentDlg::OnBnClickedChange()
{
  int ind = m_list.GetSelectionMark();
  if(ind >= 0)
  {
    XString melding;
    XString naam = m_list.GetItemText(ind,0);
//  melding.Format(message[MESS_PROFILE_EDIT][lang],naam);
//  if(theApp.WideMessageBox(melding,WhoAmI,MB_YESNO|MB_DEFBUTTON1|MB_ICONQUESTION) == IDYES)
    Profile* profile = m_profiles.GetProfile(naam);
    if(profile)
    {
      m_editting = true;
      LoadServers();
      LoadPorts();
      LoadProfile(profile);
      m_buttonNew.SetWindowText(g_message[MESS_SAVE][g_lang]); // &Save

      CWnd* wnd = GetDlgItem(IDC_EMAIL);
      wnd->SetFocus();
    }
  }
  SetMutable();
}

void 
ProfileManagamentDlg::OnBnClickedDelete()
{
  int ind = m_list.GetSelectionMark();
  if(ind >= 0)
  {
    XString melding;
    XString naam = m_list.GetItemText(ind,0);
    melding.Format(g_message[MESS_PROFILE_REMOVE][g_lang],naam.GetString());
    if(theApp.WideMessageBox(GetSafeHwnd(),melding,WhoAmI,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
    {
      m_profiles.RemoveProfile(naam);
      FillList();
    }
  }
}

void
ProfileManagamentDlg::OnBnClickedUp()
{
  int row = m_list.GetSelectionMark();
  if(row > 0)
  {
    m_profiles.ProfileUpgrade(row);
    FillList();
    --row;
    m_list.SetSelectionMark(row);
    m_list.SetItemState(0,0,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetItemState(row,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetFocus();
    m_list.SetHotItem(row);
    SetMutable();
  }
}

void
ProfileManagamentDlg::OnBnClickedDown()
{
  int aantal = m_profiles.GetNumberOfProfiles();
  int row    = m_list.GetSelectionMark();
  
  if(row < aantal)
  {
    m_profiles.ProfileDowngrade(row);
    FillList();
    ++row;
    m_list.SetSelectionMark(row);
    m_list.SetItemState(0,0,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetItemState(row,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    m_list.SetFocus();
    m_list.SetHotItem(row);
    SetMutable();
  }
}

void 
ProfileManagamentDlg::OnEnChangeProfileName()
{
  UpdateData();
}

void 
ProfileManagamentDlg::OnEnChangeEmail()
{
  UpdateData();
}

void
ProfileManagamentDlg::OnEnChangeBCC()
{
  UpdateData();
}

void
ProfileManagamentDlg::OnCbnSelchangeServer()
{
  int ind = m_comboServer.GetCurSel();
  if(ind >= 0)
  {
    m_comboServer.GetLBText(ind,m_smtpServer);
  }
  else
  {
    m_profiles.CheckMailServer(m_smtpServer);
    m_comboServer.SetWindowText(m_smtpServer.GetString());
  }
}

void
ProfileManagamentDlg::OnCbnEditchangeServer()
{
  CString server;
  m_comboServer.GetWindowText(server);
  m_smtpServer = server;
}

void
ProfileManagamentDlg::OnCbnSelchangePort()
{
  XString port;
  int ind = m_comboPort.GetCurSel();
  if(ind >= 0)
  {
    m_smtpPort = ind == 0 ? DEFAULT_SMTP_PORT : SECURE_SMTP_PORT;
  }
  else
  {
    port.Format(_T("%d"),m_smtpPort);
    m_comboPort.SetWindowText(port.GetString());
  }
}

void
ProfileManagamentDlg::OnCbnEditchangePort()
{
  CString port;
  m_comboPort.GetWindowText(port);
  m_smtpPort = _ttoi(port);
}

void 
ProfileManagamentDlg::OnBnClickedStandard()
{
  m_isStandard = m_checkStandard.GetCheck() > 0;
}

void 
ProfileManagamentDlg::OnBnClickedUselogin()
{
  m_useLogin = m_checkUseLogin.GetCheck() > 0;
  if(m_editting)
  {
    if(!m_useLogin)
    {
      m_password1.Empty();
      m_password2.Empty();
      m_userLogin.Empty();
    }
    UpdateData(FALSE);
    SetMutable();
  }
}

void
ProfileManagamentDlg::OnEnChangeUserLogin()
{
  UpdateData();
}

void 
ProfileManagamentDlg::OnEnChangePassword1()
{
  UpdateData();
}

void 
ProfileManagamentDlg::OnEnChangePassword2()
{
  UpdateData();
}

void
ProfileManagamentDlg::OnCancel()
{
  if(m_editting)
  {
    MakeEmpty();
    m_new    = false;
    m_editting = false;
    m_buttonNew.SetWindowText(g_message[MESS_NEW][g_lang]); // &New
    GoToProfile(0,true);
    return;
  }
  StyleDialog::OnCancel();
}

void 
ProfileManagamentDlg::SetBkColor(COLORREF p_colorref)
{
  if(m_brush)
  {
    delete m_brush;
  }
  m_brush = new CBrush(p_colorref);
  Invalidate();
}

HBRUSH
ProfileManagamentDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

// Splitting default font string "Verdana;20" in 
// a string "Verdana"
// a long (20)
bool
ProfileManagamentDlg::SplitFontAndSize(const XString& p_defaultFont,XString& p_font,LONG& p_size,XString& p_color)
{
  int pos = p_defaultFont.Find(_T(';'));
  if(pos < 0)
  {
    return false;
  }
  p_font = p_defaultFont.Left(pos);
  p_size = _ttoi(p_defaultFont.Mid(pos + 1));

  pos = p_defaultFont.Find(_T(';'),pos + 1);
  if(pos > 0)
  {
    p_color = p_defaultFont.Mid(pos + 1);
  }
  return true;
}

void 
ProfileManagamentDlg::OnBnClickedFont()
{
  m_defaultFont = m_profiles.GetDefaultFont();

  // Getting the font information
  CHARFORMAT2 form;
  memset(&form,0,sizeof(CHARFORMAT2));
  form.dwMask = CFM_ALL2; 
  form.cbSize = sizeof(CHARFORMAT2);

  XString font;
  LONG    size;
  XString color;
  DWORD   colornum = 0;
  if(SplitFontAndSize(m_defaultFont,font,size,color))
  {
    _tcsncpy_s(form.szFaceName,font,LF_FACESIZE);
    form.yHeight = size * TWIPS;
    _stscanf_s(color,_T("#%X"),&colornum);
    form.crTextColor = colornum;
  }

  // Choose a font
  DWORD flags = CF_SCREENFONTS | CF_PRINTERFONTS | CF_NOVERTFONTS | CF_NOSTYLESEL; // CF_USESTYLE;
  CFontDialog dlg(form,flags,NULL,this);
  if(dlg.DoModal() == IDOK)
  {
    // Change the font
    CHARFORMAT2 format;
    memset(&format,0,sizeof(CHARFORMAT2));
    format.cbSize = sizeof(CHARFORMAT2);
    dlg.GetCharFormat(format);

    font     = format.szFaceName;
    size     = format.yHeight / TWIPS;
    colornum = format.crTextColor;

    m_defaultFont.Format(_T("%s;%d;#%06X"),font.GetString(),size,colornum);
    m_profiles.SetDefaultFont(m_defaultFont);
    m_profiles.WriteProfiles();

    if(m_profiles.GetWriteAccess())
    {
      // Write font data for all users as the 'default font'?
      if(theApp.WideMessageBox(GetSafeHwnd()
                              ,g_message[MESS_WRITE_DEFAULT_FONT][g_lang]
                              ,_T(PRODUCT_NAME)
                              ,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
      {
        m_profiles.WriteCentralSettings(true);
      }
    }
  }
}
