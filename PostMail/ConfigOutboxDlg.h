/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ConfigOutboxDlg.h
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
#include "OutBox.h"

// ConfigOutboxDlg dialog

class ConfigOutboxDlg : public StyleDialog
{
	DECLARE_DYNAMIC(ConfigOutboxDlg)

public:
	ConfigOutboxDlg(CWnd* p_parent,bool p_canSetDir,const XString& p_directory);
	virtual ~ConfigOutboxDlg();
  BOOL     OnInitDialog();

// Dialog Data
	enum { IDD = IDD_CONFIG_OUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  void         ReadConfig();
  bool         SaveConfig();
  void         SetDefaults();
  bool         CheckDirectory(const XString& p_directory);
  void         AddHistory(const XString& p_directory);
  void         SetType(BoxType p_type);
  void         InitTexts();

	DECLARE_MESSAGE_MAP()
  
  StyleEdit     m_editDirectory;
  StyleEdit     m_editMaxFiles;
  StyleListCtrl m_list;
  StyleComboBox m_comboType;
  StyleComboBox m_comboSelection;
  StyleButton   m_buttonChoose;
  StyleButton   m_buttonDelete;
  StyleButton   m_buttonOk;
  StyleButton   m_buttonCancel;

  StyleStatic   m_staticTypeOutbox;
  StyleStatic   m_staticMapMail;
  StyleStatic   m_staticInitSelect;
  StyleStatic   m_staticWarnAt;
  StyleStatic   m_staticHistory;

public:
  afx_msg void OnCbnSelchangeType();
  afx_msg void OnEnChangeDirectory();
  afx_msg void OnBnClickedChoose();
  afx_msg void OnCbnSelchangeSelect();
  afx_msg void OnEnChangeMaxFiles();
  afx_msg void OnLvnItemDoubleClick(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
  afx_msg void OnBnClickedDelete();

private:
  bool      m_init;
  bool      m_admin;
  bool      m_canSetDirectory;
  int       m_maxFiles;
  BoxType   m_type;
  XString   m_directory;
  DateSel   m_selection;
};
