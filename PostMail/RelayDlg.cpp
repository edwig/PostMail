/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: RelayDlg.cpp
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
#include "RelayDlg.h"
#include "PostMail.h"
#include "Message.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 28159)  // GetTickCount

BEGIN_MESSAGE_MAP(RelayDlg,StyleDialog)
  ON_WM_TIMER()
  ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

RelayDlg::RelayDlg(CDialog* p_parent,bool show)
         :StyleDialog(RelayDlg::IDD, p_parent)
         ,m_max(5)
         ,m_current(0)
         ,m_brush(NULL)
{
  m_show = show ? SW_SHOW : SW_HIDE;
}

RelayDlg::~RelayDlg()
{
  Reset();
  if(m_brush)
  {
    delete m_brush;
    m_brush = NULL;
  }
}

void
RelayDlg::DoTheInit()
{
  Create(IDD_RELAY,NULL);
  ShowWindow(m_show);
  SetIcon(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDR_MAINFRAME)),FALSE);
}

BOOL
RelayDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();

  // Set a timer and set the window in the foreground after 0.2 seconds
  SetTimer(1,200,NULL);

  if(theApp.GetODataConfig().GetHasBeenRead())
  {
    // "Create a concept e-mail"
    SetWindowText(g_message[MESS_INITRELAY][g_lang]);
  }
  SetSaveMonitor(false);
  UpdateData(FALSE);
  return TRUE;
}

void 
RelayDlg::Reset()
{
  m_gauge.SetPos(m_max);
  SetText(_T(""));
  UpdateData(FALSE);
  if(m_show)
  {
    Invalidate();
    Repaint();
  }
  // Now hide it explicitly
  ShowWindow(SW_HIDE);
}

void   
RelayDlg::OnTimer(UINT_PTR nIDEvent)
{
  if(nIDEvent == 1)
  {
    KillTimer(1);
    // Set topmost window and force repainting
    SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
    // After 1 second: release the topmost
    SetTimer(2,200,NULL);
  }
  if(nIDEvent == 2)
  {
    KillTimer(2);
    // Release topmost
    SetWindowPos(&CWnd::wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  }
}

void 
RelayDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Control(pDX,IDC_RELAY_TEXT,m_editText,m_text);
  DDX_Control(pDX,IDC_PROGRESS,  m_gauge);
}

void
RelayDlg::SetCurrent(int p_current)
{
  if(p_current > m_current && p_current <= m_max)
  {
    m_gauge.SetPos(m_current = p_current);
  }
  UpdateData(FALSE);
  if(m_show)
  {
    Invalidate();
    Repaint();
  }
}

void
RelayDlg::SetMax(int p_max)
{
  if(p_max > 5 && p_max < 32000)
  {
    m_max = p_max;
  }
  // Set the gouge maximum and reset it
  m_gauge.SetRange(0,(short)m_max);
  m_gauge.SetPos(0);
  UpdateData(FALSE);
  if(m_show)
  {
    Invalidate();
    Repaint();
  }
}

void
RelayDlg::Increment()
{
  if(m_current < m_max)
  {
    m_gauge.SetPos(++m_current);
  }
  UpdateData(FALSE);
  if(m_show)
  {
    Invalidate();
    Repaint();
  }
}

void 
RelayDlg::SetAtEnd()
{
  m_gauge.SetPos(m_max);
  UpdateData(FALSE);
  if(m_show)
  {
    Invalidate();
    Repaint();
  }
}

void
RelayDlg::SetText(XString p_text)
{
  m_text = p_text;
  UpdateData(FALSE);
  CWnd* tt = GetDlgItem(IDC_RELAY_TEXT);
  if(m_show)
  {
    tt->Invalidate();
    Invalidate();
    Repaint();
  }
}

void
RelayDlg::Repaint()
{
  // Without this yield the progress control will NEVER be shown
  Sleep(10);
  Yield();
  MSG  msg;
  UINT ticks = GetTickCount();
  while(GetTickCount() - ticks < 1000 && PeekMessage(&msg,NULL,0,0,PM_REMOVE))
  {
    try
    {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
    catch(...)
    {
      // How now, brown cow?
    }
  }
  Yield();
  Sleep(300);
}

void 
RelayDlg::SetBkColor(COLORREF p_colorref)
{
  if(m_brush)
  {
    delete m_brush;
  }
  m_brush = new CBrush(p_colorref);
  Invalidate();
}

HBRUSH
RelayDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
