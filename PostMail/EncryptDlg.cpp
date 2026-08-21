////////////////////////////////////////////////////////////////////////
//
// PostMailEncrypt\EncryptDlg.cpp
// 
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#include "stdafx.h"
#include "EncryptDlg.h"
#include "PostMail.h"
#include <Crypto.h>
#include <afxdialogex.h>
#include "Version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// AboutDlg dialog used for App About

class EncAboutDlg : public StyleDialog
{
public:
  EncAboutDlg();

  // Dialog Data
  enum { IDD = IDD_ABOUTBOX };

protected:
  virtual void DoDataExchange(CDataExchange* pDX) override;
  virtual BOOL OnInitDialog() override;

// Implementation
protected:
  DECLARE_MESSAGE_MAP()

  CString m_versie;
  CString m_copyright;
};

EncAboutDlg::EncAboutDlg() : StyleDialog(EncAboutDlg::IDD)
{
  m_versie    = APPLICATION _T(" ") PRODUCT_VERSION;
  m_copyright = COPYRIGHT;
}

void EncAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Text(pDX,IDC_ST_VERSION,m_versie);
  DDX_Text(pDX,IDC_ST_TEXT,   m_copyright);
}

BEGIN_MESSAGE_MAP(EncAboutDlg, StyleDialog)
END_MESSAGE_MAP()

BOOL EncAboutDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(_T("About..."));
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// PASSWORD DIALOG
//
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(EncryptDlg, StyleDialog)

EncryptDlg::EncryptDlg(CWnd* pParent /*=nullptr*/)
           :StyleDialog(IDD_POSTMAILENCRYPT, pParent,true,true)
{
  m_saveMonitor = false;
}

EncryptDlg::~EncryptDlg()
{
}

void EncryptDlg::DoDataExchange(CDataExchange* pDX)
{
	StyleDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_EDIT1, m_edit1, m_password1);
  DDX_Control(pDX, IDC_EDIT2, m_edit2, m_password2);
  DDX_Control(pDX, IDC_EDIT3, m_edit3, m_gecodeerd);
  DDX_Control(pDX, IDC_TOON,  m_checkToon);
  DDX_Control(pDX, IDC_COPY,  m_buttonCopy);
  DDX_Control(pDX, IDOK,      m_buttonOK);
}

BEGIN_MESSAGE_MAP(EncryptDlg, StyleDialog)
  ON_EN_KILLFOCUS(IDC_EDIT1,          OnEnChangeEdit1)
  ON_EN_KILLFOCUS(IDC_EDIT2,          OnEnChangeEdit2)
  ON_EN_KILLFOCUS(IDC_EDIT3,          OnEnChangeEdit3)
  ON_BN_CLICKED  (IDC_TOON,           OnBnClickedToon)
  ON_BN_CLICKED  (IDC_COPY,           OnBnClickedCopy)
  ON_BN_CLICKED  (IDOK,               OnBnClickedOk)
  ON_COMMAND     (ID_HELP_ABOUT,      OnAbout)
  ON_COMMAND     (ID_THEMA_LIME,      OnStyleLime)
  ON_COMMAND     (ID_THEMA_SKYBLUE,   OnStyleSkyblue)
  ON_COMMAND     (ID_THEMA_PURPLE,    OnStylePurple)
  ON_COMMAND     (ID_THEMA_MUSTARD,   OnStyleMustard)
  ON_COMMAND     (ID_THEMA_MODERATE,  OnStyleModerateGray)
  ON_COMMAND     (ID_THEMA_PUREGRAY,  OnStylePureGray)
  ON_COMMAND     (ID_THEMA_BLACKWHITE,OnStyleBlackWhite)
  ON_COMMAND     (ID_THEMA_DARKTHEME, OnStyleDark)
END_MESSAGE_MAP()

BOOL
EncryptDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(_T("Encrypt for PostMail"));
  ShowMinMaxButton(true,false);
  SetSysMenu(IDR_MENU2);

  m_edit1.SetPassword();
  m_edit1.SetEmpty(true,_T("Configuration text"));

  m_edit2.SetPassword();
  m_edit2.SetEmpty(true,_T("Repeat text"));

  m_buttonOK.SetStyle(_T("ok"));

  SetCanResize();

  UpdateData(FALSE);

  return TRUE;
}

// Triggered by SetCanResize
void
EncryptDlg::SetupDynamicLayout()
{
  // See to it that we get a manager;
  StyleDialog::SetupDynamicLayout();

  auto manager = GetDynamicLayout();
  if(manager != nullptr)
  {
    manager->AddItem(IDC_EDIT1, CMFCDynamicLayout::MoveNone(), CMFCDynamicLayout::SizeHorizontal(100));
    manager->AddItem(IDC_EDIT2, CMFCDynamicLayout::MoveNone(), CMFCDynamicLayout::SizeHorizontal(100));
    manager->AddItem(IDC_EDIT3, CMFCDynamicLayout::MoveNone(), CMFCDynamicLayout::SizeHorizontal(100));
    manager->AddItem(IDC_COPY,  CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDOK,      CMFCDynamicLayout::MoveHorizontal(100), CMFCDynamicLayout::SizeNone());
  }
  CSize size(520, 210);
  manager->SetMinSize(size);
}

// Checks if all fields are OK
// If one of them is, not 'correct' will be zero
bool
EncryptDlg::CheckAllFieldsOK()
{
  int correct = 1;

  if(m_password1.GetLength())
  {
    m_edit1.SetErrorState(false);
  }
  correct *= m_edit1.CheckEditOK();
  correct *= m_edit3.CheckEditOK();

  if(m_password1.Compare(m_password2))
  {
    correct = 0;
  }
  correct *= m_password1.GetLength();

  return correct;
}

void EncryptDlg::EncryptPassword()
{
  if(CheckAllFieldsOK() && (m_password1.Compare(m_password2) == 0 && m_password1.GetLength()))
  {
    if(m_password1.Find(':') >= 0)
    {
      StyleMessageBox(this,_T("Passwords may not contain the ':' character"),_T("ERROR"),MB_OK|MB_ICONERROR);
    }
    else
    {
      Crypto crypt;
      CString encrypted;
      CString reverse(m_password1);
      reverse.MakeReverse();
      XString value = reverse + _T(":") + m_password1;
      m_gecodeerd = crypt.Encryption(value,POSTMAIL_PASSWORD);
    }
  }
  else
  {
    m_gecodeerd.Empty();
  }
}

// message handlers

void EncryptDlg::OnAbout()
{
  EncAboutDlg dlg;
  dlg.DoModal();
}

void EncryptDlg::OnEnChangeEdit1()
{
  UpdateData();
  EncryptPassword();
  UpdateData(FALSE);
}

void EncryptDlg::OnEnChangeEdit2()
{
  UpdateData();
  EncryptPassword();
  UpdateData(FALSE);
}

void EncryptDlg::OnBnClickedToon()
{
  bool hide = m_checkToon.GetCheck() == 0;
  m_edit1.SetPassword(hide);
  m_edit2.SetPassword(hide);
  UpdateData(FALSE);
}

void EncryptDlg::OnEnChangeEdit3()
{
}

void EncryptDlg::OnBnClickedCopy()
{
  if(CheckAllFieldsOK())
  {
    m_edit3.SetSel(0,-1);
    m_edit3.Copy();
    StyleMessageBox(this,_T("Encrypted password has been copied to the clipboard"),_T("Password"),MB_OK|MB_ICONINFORMATION);
  }
}

void EncryptDlg::OnBnClickedOk()
{
  if(CheckAllFieldsOK() == false)
  {
    UpdateData(FALSE);
    return;
  }
  StyleDialog::OnOK();
}
