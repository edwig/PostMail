/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: AdresDlg.h
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
#pragma once
#include "SMTPMessage.h"

// AdresDlg dialog

class AdresDlg : public StyleDialog
{
	DECLARE_DYNAMIC(AdresDlg)

public:
	AdresDlg(CWnd* pParent,SMTPMessage* p_message);
 ~AdresDlg();
  BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_USERS };

protected:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  bool CheckAddress();
  void InitTexts();
  void InitTOList();
  void InitCCList();
  void SetBkColor(COLORREF p_colorref);

	DECLARE_MESSAGE_MAP()

  StyleListBox  m_listTO;
  StyleListBox  m_listCC;
  SMTPMessage*  m_message;
  CBrush*       m_brush;
  StyleStatic   m_staticTO;
  StyleStatic   m_staticCC;

  StyleButton   m_buttonAddTo;
  StyleButton   m_buttonDeleteTo;
  StyleButton   m_buttonAddCc;
  StyleButton   m_buttonDeleteCc;
  StyleButton   m_buttonOK;
  StyleButton   m_buttonCancel;

public:
  afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void    OnLbnSelchangeListTo();
  afx_msg void    OnBnClickedAddTo();
  afx_msg void    OnBnClickedDeleteTo();
  afx_msg void    OnLbnSelchangeListCc();
  afx_msg void    OnBnClickedAddCc();
  afx_msg void    OnBnClickedDeleteCc();
  afx_msg void    OnBnClickedOk();
  afx_msg void    OnBnClickedCancel();
};
