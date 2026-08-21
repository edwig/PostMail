/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ChooseProfileDlg.h
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
#include "Profiles.h"

// ChooseProfileDlg dialog

class ChooseProfileDlg : public StyleDialog
{
	DECLARE_DYNAMIC(ChooseProfileDlg)

public:
	ChooseProfileDlg(CWnd* pParent,Profiles& p_profiles,bool p_original = false);
 ~ChooseProfileDlg();
  BOOL OnInitDialog();
  bool GetReread();

// Dialog Data
	enum { IDD = IDD_USEPROFILE };

protected:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  void FillList();
  void InitTexts();
  void LoadProfile(Profile* p_profile);
  void SetBkColor(COLORREF p_colorref);

	DECLARE_MESSAGE_MAP()

  XString m_profile;
  XString m_server;
  XString m_user;
  XString m_email;
  bool    m_asking;
  bool    m_original;
  bool    m_reread;

  StyleEdit     m_editProfile;
  StyleEdit     m_editServer;
  StyleEdit     m_editUser;
  StyleEdit     m_editEmail;
  StyleListCtrl m_list;
  CBrush*       m_brush;
  StyleCheckbox m_buttonSession;
  StyleButton   m_buttonOriginal;
  StyleButton   m_buttonConfig;
  StyleButton   m_buttonOK;
  StyleButton   m_buttonCancel;

  Profiles&     m_profiles;

  StyleStatic   m_statLine1;
  StyleStatic   m_statLine3;
  StyleGroupBox m_groupList;
  StyleStatic   m_statProfile;
  StyleStatic   m_statServer;
  StyleStatic   m_statUser;
  StyleStatic   m_statEmail;

public:
  afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void    OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void    OnLvnDoubleClick    (NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void    OnBnClickedVragen();
  afx_msg void    OnBnClickedConfig();
  afx_msg void    OnBnClickedOriginal();
  afx_msg void    OnOK();
};

inline bool
ChooseProfileDlg::GetReread()
{
  return m_reread;
}