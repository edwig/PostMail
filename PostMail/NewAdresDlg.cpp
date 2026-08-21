/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: NewAdresDlg.cpp
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
#include "NewAdresDlg.h"
#include "Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NewAdresDlg dialog

IMPLEMENT_DYNAMIC(NewAdresDlg, StyleDialog)

NewAdresDlg::NewAdresDlg(CWnd* pParent,SMTPAddress* p_address
                        ,bool p_ccList,bool p_inBCC)
            :StyleDialog(NewAdresDlg::IDD, pParent)
            ,m_address(p_address)
            ,m_ccList(p_ccList)
            ,m_inBCC(p_inBCC)
            ,m_brush(NULL)
{
}

NewAdresDlg::~NewAdresDlg()
{
  if(m_brush)
  {
    delete m_brush;
    m_brush = nullptr;
  }
}

void 
NewAdresDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Control(pDX,IDC_EMAILADRES,     m_editEmail,   m_email);
  DDX_Control(pDX,IDC_FRIENDLY,       m_editFriendly,m_friendly);
  DDX_Control(pDX,IDC_PUTBCC,         m_buttonBCC);
  DDX_Control(pDX,IDC_STATIC_EMAIL,   m_staticEmail);
  DDX_Control(pDX,IDC_STATIC_FRIENDLY,m_staticFriendly);
  DDX_Control(pDX,IDOK,               m_buttonOK);
  DDX_Control(pDX,IDCANCEL,           m_buttonCancel);
}

BEGIN_MESSAGE_MAP(NewAdresDlg, StyleDialog)
  ON_WM_CTLCOLOR()
  ON_EN_CHANGE (IDC_EMAILADRES, &NewAdresDlg::OnEnChangeEmailadres)
  ON_EN_CHANGE (IDC_FRIENDLY,   &NewAdresDlg::OnEnChangeFriendly)
  ON_BN_CLICKED(IDC_PUTBCC,     &NewAdresDlg::OnBnClickedPutbcc)
  ON_BN_CLICKED(IDOK,           &NewAdresDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL
NewAdresDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(g_message[MESS_ADDADDRESS][g_lang]);

  InitTexts();
  m_email    = m_address->m_emailAddress;
  m_friendly = m_address->m_friendlyName;
  UpdateData(FALSE);

  if(m_ccList == false)
  {
    m_buttonBCC.EnableWindow(FALSE);
  }

  return TRUE;
}

void
NewAdresDlg::InitTexts()
{
  m_staticEmail   .SetWindowText(g_message[MESS_EMAILADDRESS][g_lang]);
  m_staticFriendly.SetWindowText(g_message[MESS_FRIENDLYNAME][g_lang]);
  m_buttonBCC     .SetWindowText(g_message[MESS_PUTINBCC    ][g_lang]);
  m_buttonCancel  .SetWindowText(g_message[MESS_CANCEL      ][g_lang]);
}

bool
NewAdresDlg::CheckSpecials()
{
  if(m_friendly.FindOneOf(_T("(),:;<>@[\\]")) >= 0)
  {
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_ADRES_SPECIALS][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    return true;
  }
  return false;
}

// NewAdresDlg message handlers

void 
NewAdresDlg::OnEnChangeEmailadres()
{
  UpdateData();
}

void 
NewAdresDlg::OnEnChangeFriendly()
{
  UpdateData();
  CheckSpecials();
}

void 
NewAdresDlg::OnBnClickedPutbcc()
{
  m_inBCC = m_buttonBCC.GetCheck() > 0; 
}

void 
NewAdresDlg::OnBnClickedOk()
{
  if(m_email.IsEmpty())
  {
    // No address given
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_NORCPT][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    return;
  }
  if(CheckSpecials())
  {
    return;
  }
  // Put back in the address
  m_address->m_emailAddress = m_email;
  m_address->m_friendlyName = m_friendly;

  // End of dialog
  OnOK();
}

void 
NewAdresDlg::SetBkColor(COLORREF p_colorref)
{
  if(m_brush)
  {
    delete m_brush;
  }
  m_brush = new CBrush(p_colorref);
  Invalidate();
}

HBRUSH
NewAdresDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
