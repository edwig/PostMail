/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: OutBoxDlg.h
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#pragma once
#include "OutBox.h"

// OutBoxDlg dialog

class OutBoxDlg : public StyleDialog
{
	//DECLARE_DYNAMIC(OutBoxDlg)
public:
	OutBoxDlg(CWnd* p_parent,OutBox& p_outbox);   // standard constructor
	virtual ~OutBoxDlg();
  OutBox&  GetOutbox();

// Dialog Data
	enum { IDD = IDD_OUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;
  virtual void SetupDynamicLayout() override;
  virtual BOOL OnInitDialog() override;
  XString      GetClipboardViewer(const XString& p_filename);
  void         Process(int p_item);

	DECLARE_MESSAGE_MAP()

private:
  void      InitTexts();
  void      InitList();
  void      FillOutbox();
  bool      BeSureToDelete();

  OutBox&       m_outbox;
  XString       m_title;
  XString       m_search;

  StyleEdit     m_editTitle;
  StyleEdit     m_editSearch;
  StyleButton   m_buttonSearch;
  StyleButton   m_buttonRemoveFilter;
  StyleListCtrl m_list;
  StyleCheckbox m_buttonToday;
  StyleCheckbox m_buttonWeek;
  StyleCheckbox m_buttonMonth;
  StyleCheckbox m_button3Months;
  StyleCheckbox m_button6Months;
  StyleCheckbox m_buttonYear;
  StyleCheckbox m_buttonAll;
  StyleButton   m_buttonConfig;
  StyleButton   m_buttonLogfile;
  StyleCheckbox m_buttonActief;
  StyleStatic   m_selection;
  StyleButton   m_buttonDelete;
  StyleButton   m_buttonAgain;
  StyleButton   m_buttonOK;

  StyleStatic   m_staticOutbox;
  StyleStatic   m_staticSearch;

public:
  afx_msg void OnEnKillfocusSearch();
  afx_msg void OnBnClickedSearch();
  afx_msg void OnBnClickedRemoveFilter();
  afx_msg void OnLvnItemDoubleClick(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedToday();
  afx_msg void OnBnClickedWeek();
  afx_msg void OnBnClickedMonth();
  afx_msg void OnBnClicked3months();
  afx_msg void OnBnClicked6months();
  afx_msg void OnBnClickedYear();
  afx_msg void OnBnClickedAll();
  afx_msg void OnBnClickedConfig();
  afx_msg void OnBnClickedLogfile();
  afx_msg void OnBnClickedActief();
  afx_msg void OnBnClickedDelete();
  afx_msg void OnBnClickedAgain();
  afx_msg void OnBnClickedOk();
  afx_msg void OnOK();
};

inline OutBox& 
OutBoxDlg::GetOutbox()
{
  return m_outbox;
}
