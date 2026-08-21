/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ProfileManagamentDlg.h
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

#define TWIPS 20

class ProfileManagamentDlg : public StyleDialog
{
	DECLARE_DYNAMIC(ProfileManagamentDlg)

public:
	ProfileManagamentDlg(CWnd* p_parent,Profiles& p_profiles);   // standard constructor
 ~ProfileManagamentDlg();
  BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_PROFILE };

protected:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  bool SaveChangedProfile(Profile *p_profile);
  void LoadProfile(Profile* p_profile);
  void GoToProfile(int p_profile,bool p_focus = false);
  bool SplitFontAndSize(const XString& p_defaultFont,XString& p_font,LONG& p_size,XString& p_color);
  bool CheckPasswords();
  void SetMutable();
  void SaveNewProfile();
  void InitTexts();
  void FillList();
  void MakeEmpty();
  void LoadServers();
  void LoadPorts();
  void SetBkColor(COLORREF p_colorref);

	DECLARE_MESSAGE_MAP()

  bool            m_editting;
  bool            m_new;
  int             m_number;
  bool            m_personal;
  bool            m_canchange;
  XString         m_profileName;
  XString         m_emailAddress;
  XString         m_bccAddress;
  XString         m_smtpServer;
  int             m_smtpPort;
  bool            m_isStandard;
  bool            m_useLogin;
  XString         m_userLogin;
  XString         m_password1;
  XString         m_password2;
  XString         m_defaultFont;

  StyleEdit       m_editProfileName;
  StyleEdit       m_editEmailAddress;
  StyleEdit       m_editBCCAddress;
  StyleEdit       m_editUserLogin;
  StyleEdit       m_editPassword1;
  StyleEdit       m_editPassword2;
  StyleListCtrl   m_list;
  StyleButton     m_buttonNew;
  StyleButton     m_buttonModify;
  StyleButton     m_buttonDelete;
  StyleButton     m_buttonUp;
  StyleButton     m_buttonDown;
  StyleComboBox   m_comboServer;
  StyleComboBox   m_comboPort;
  StyleCheckbox   m_checkStandard;
  StyleCheckbox   m_checkUseLogin;
  StyleButton     m_buttonOK;
  StyleButton     m_buttonCancel;
  Profiles&       m_profiles;
  CBrush*         m_brush;
  StyleButton     m_buttonFont;

  StyleStatic     m_statProfileName;
  StyleStatic     m_statEmail;
  StyleStatic     m_statBcc;
  StyleStatic     m_statServer;
  StyleStatic     m_statPort;
  StyleStatic     m_statStandard;
  StyleStatic     m_statLogin;
  StyleStatic     m_statUser;
  StyleStatic     m_statWW1;
  StyleStatic     m_statWW2;
  
public:
  afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnLvnItemActivate   (NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedNew();
  afx_msg void OnBnClickedChange();
  afx_msg void OnBnClickedDelete();
  afx_msg void OnBnClickedUp();
  afx_msg void OnBnClickedDown();
  afx_msg void OnEnChangeProfileName();
  afx_msg void OnEnChangeEmail();
  afx_msg void OnEnChangeBCC();
  afx_msg void OnCbnSelchangeServer();
  afx_msg void OnCbnEditchangeServer();
  afx_msg void OnCbnSelchangePort();
  afx_msg void OnCbnEditchangePort();
  afx_msg void OnBnClickedStandard();
  afx_msg void OnBnClickedUselogin();
  afx_msg void OnEnChangeUserLogin();
  afx_msg void OnEnChangePassword1();
  afx_msg void OnEnChangePassword2();
  afx_msg void OnBnClickedFont();
  afx_msg void OnCancel();
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
