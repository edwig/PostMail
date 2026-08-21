/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: AdresDlg.cpp
//
// Mailer to post an e-mail to the DARPA-net
// 
// Written by W.E. Huisman
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
#include "AdresDlg.h"
#include "NewAdresDlg.h"
#include "Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// AdresDlg dialog

IMPLEMENT_DYNAMIC(AdresDlg, StyleDialog)

AdresDlg::AdresDlg(CWnd* pParent,SMTPMessage* p_message)
         :StyleDialog(AdresDlg::IDD, pParent)
         ,m_message(p_message)
         ,m_brush(NULL)
{
}

AdresDlg::~AdresDlg()
{
  if(m_brush)
  {
    delete m_brush;
    m_brush = NULL;
  }
}


void 
AdresDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Control(pDX,IDC_STATIC_TO,m_staticTO);
  DDX_Control(pDX,IDC_STATIC_CC,m_staticCC);
  DDX_Control(pDX,IDC_LIST_CC,  m_listCC);
  DDX_Control(pDX,IDC_LIST_TO,  m_listTO);

  DDX_Control(pDX, IDC_ADD_TO,   m_buttonAddTo);
  DDX_Control(pDX, IDC_DELETE_TO,m_buttonDeleteTo);
  DDX_Control(pDX, IDC_ADD_CC,   m_buttonAddCc);
  DDX_Control(pDX, IDC_DELETE_CC,m_buttonDeleteCc);
  DDX_Control(pDX, IDOK,         m_buttonOK);
  DDX_Control(pDX, IDCANCEL,     m_buttonCancel);
}

BEGIN_MESSAGE_MAP(AdresDlg, StyleDialog)
  ON_WM_CTLCOLOR()
  ON_LBN_DBLCLK(IDC_LIST_TO,   &AdresDlg::OnLbnSelchangeListTo)
  ON_BN_CLICKED(IDC_ADD_TO,    &AdresDlg::OnBnClickedAddTo)
  ON_BN_CLICKED(IDC_DELETE_TO, &AdresDlg::OnBnClickedDeleteTo)
  ON_LBN_DBLCLK(IDC_LIST_CC,   &AdresDlg::OnLbnSelchangeListCc)
  ON_BN_CLICKED(IDC_ADD_CC,    &AdresDlg::OnBnClickedAddCc)
  ON_BN_CLICKED(IDC_DELETE_CC, &AdresDlg::OnBnClickedDeleteCc)
  ON_BN_CLICKED(IDOK,          &AdresDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL,      &AdresDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL
AdresDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  SetWindowText(g_message[MESS_SENDING_TO][g_lang]);

  InitTexts();
  InitTOList();
  InitCCList();

  m_listTO.SetFocus();

  return TRUE;
}

void
AdresDlg::InitTexts()
{
  m_staticTO      .SetWindowText(g_message[MESS_TO    ][g_lang]);
  m_staticCC      .SetWindowText(g_message[MESS_CC    ][g_lang]);
  m_buttonAddTo   .SetWindowText(g_message[MESS_NEW   ][g_lang]);
  m_buttonDeleteTo.SetWindowText(g_message[MESS_DELETE][g_lang]);
  m_buttonAddCc   .SetWindowText(g_message[MESS_NEW   ][g_lang]);
  m_buttonDeleteCc.SetWindowText(g_message[MESS_DELETE][g_lang]);
  m_buttonOK      .SetWindowText(g_message[MESS_OK    ][g_lang]);
  m_buttonCancel  .SetWindowText(g_message[MESS_CANCEL][g_lang]);
}

void 
AdresDlg::InitTOList()
{
  m_listTO.ResetContent();

  for (int i=0;i < m_message->GetNumberOfRecipients(TO); ++i)
  {
    SMTPAddress recipient = m_message->GetRecipient(i,TO);
    m_listTO.AddString(recipient.GetRegularFormat());
  }
  m_listTO.SetCurSel(0);
}

void 
AdresDlg::InitCCList()
{
  m_listCC.ResetContent();

  for (int i=0;i < m_message->GetNumberOfRecipients(CC); ++i)
  {
    SMTPAddress recipient = m_message->GetRecipient(i, CC);
    m_listCC.AddString(recipient.GetRegularFormat());
  }
  for (int i=0;i < m_message->GetNumberOfRecipients(BCC); ++i)
  {
    SMTPAddress recipient = m_message->GetRecipient(i, BCC);
    m_listCC.AddString(XString(_T("(BCC) ")) + recipient.GetRegularFormat());
  }
  m_listCC.SetCurSel(0);
}

bool
AdresDlg::CheckAddress()
{
  if((m_message->GetNumberOfRecipients(TO) +
      m_message->GetNumberOfRecipients(CC) +
      m_message->GetNumberOfRecipients(BCC)) == 0)
  {
    return false;
  }
  return true;
}

// AdresDlg message handlers

void 
AdresDlg::OnLbnSelchangeListTo()
{
  int item = m_listTO.GetCurSel();
  if(item >= 0)
  {
    XString adres;
    m_listTO.GetText(item,adres);

    SMTPAddress* smtp = m_message->GetRecipient(adres,TO);

    NewAdresDlg dlg(this,smtp,false,false);
    if(dlg.DoModal() == IDOK)
    {
      InitTOList();
    }
  }
}

void AdresDlg::OnBnClickedAddTo()
{
  SMTPAddress adres;
  NewAdresDlg dlg(this,&adres,false,false);
  if(dlg.DoModal() == IDOK)
  {
    m_message->AddRecipient(adres,TO);
    InitTOList();
  }
}

void 
AdresDlg::OnBnClickedDeleteTo()
{
  int item = m_listTO.GetCurSel();
  if(item >= 0)
  {
    CString adres;
    m_listTO.GetText(item,adres);
    CString melding;
    melding.Format(g_message[MESS_ADRES_DELETE][g_lang],adres.GetString());
    if(theApp.WideMessageBox(GetSafeHwnd(),melding,WhoAmI,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
    {
      XString todel(adres);
      m_message->DeleteRecipient(todel,TO);
      InitTOList();
    }
  }
}

void 
AdresDlg::OnLbnSelchangeListCc()
{
  bool useBCC = false;

  int item = m_listCC.GetCurSel();
  if(item >= 0)
  {
    XString adres;
    m_listCC.GetText(item,adres);
    if(adres.Left(5) == _T("(BCC)"))
    {
      useBCC = true; 
      adres  = adres.Mid(6);
    }

    SMTPAddress* smtp = NULL;
    if(useBCC)
    {
      smtp = m_message->GetRecipient(adres,BCC);
    }
    else
    {
      smtp = m_message->GetRecipient(adres,CC);
    }
    NewAdresDlg dlg(this,smtp,true,useBCC);
    if(dlg.DoModal() == IDOK)
    {
      InitCCList();
    }
  }
}

void 
AdresDlg::OnBnClickedAddCc()
{
  SMTPAddress adres;
  NewAdresDlg dlg(this,&adres,true,false);
  if(dlg.DoModal() == IDOK)
  {
    if(dlg.InBCCList())
    {
      m_message->AddRecipient(adres,BCC);
    }
    else
    {
      m_message->AddRecipient(adres,CC);
    }
    InitCCList();
  }
}

void 
AdresDlg::OnBnClickedDeleteCc()
{
  bool useBCC = false;

  int item = m_listCC.GetCurSel();
  if(item >= 0)
  {
    CString adres;
    m_listCC.GetText(item,adres);
    if(adres.Left(5) == _T("(BCC)"))
    {
      useBCC = true; 
      adres  = adres.Mid(6);
    }
    CString melding;
    melding.Format(g_message[MESS_ADRES_DELETE][g_lang],adres.GetString());
    if(theApp.WideMessageBox(GetSafeHwnd(),melding,WhoAmI,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
    {
      XString todel(adres);
      if(useBCC)
      {
        m_message->DeleteRecipient(todel,BCC);
      }
      else
      {
        m_message->DeleteRecipient(todel,CC);
      }
      InitCCList();
    }
  }
}

void 
AdresDlg::OnBnClickedOk()
{
  // Must have at least one address!
  if(CheckAddress() == false)
  {
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_NORCPT][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
    return;
  }
  OnOK();
}

void 
AdresDlg::OnBnClickedCancel()
{
  OnCancel();
}

void 
AdresDlg::SetBkColor(COLORREF p_colorref)
{
  if(m_brush)
  {
    delete m_brush;
  }
  m_brush = new CBrush(p_colorref);
  Invalidate();
}

HBRUSH
AdresDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
