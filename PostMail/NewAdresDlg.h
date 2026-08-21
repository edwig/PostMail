/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: NewAdresDlg.h
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
#include "SMTPAddress.h"

// NewAdresDlg dialog

class NewAdresDlg : public StyleDialog
{
	DECLARE_DYNAMIC(NewAdresDlg)

public:
	NewAdresDlg(CWnd* pParent,SMTPAddress* p_address
             ,bool p_ccList,bool p_inBCC);
 ~NewAdresDlg();
  BOOL OnInitDialog();
  bool InBCCList();

// Dialog Data
	enum { IDD = IDD_ADDUSER };

protected:
	void  DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  bool  CheckSpecials();
  void  SetBkColor(COLORREF p_colorref);
  void  InitTexts();

	DECLARE_MESSAGE_MAP()

  XString       m_email;
  XString       m_friendly;
  bool          m_ccList;
  bool          m_inBCC;
  SMTPAddress*  m_address;
  CBrush*       m_brush;
  StyleCheckbox m_buttonBCC;
  StyleStatic   m_staticEmail;
  StyleStatic   m_staticFriendly;
  StyleEdit     m_editEmail;  
  StyleEdit     m_editFriendly;
  StyleButton   m_buttonOK;
  StyleButton   m_buttonCancel;

public:
  afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void    OnEnChangeEmailadres();
  afx_msg void    OnEnChangeFriendly();
  afx_msg void    OnBnClickedPutbcc();
  afx_msg void    OnBnClickedOk();
};

inline bool 
NewAdresDlg::InBCCList()
{
  return m_inBCC;
}
