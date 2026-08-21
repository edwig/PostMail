////////////////////////////////////////////////////////////////////////
//
// PostMailEncrypt\EncryptDlg.h
// 
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#pragma once
#include "resource.h"
#include <StyleFrameWork.h>

class EncryptDlg : public StyleDialog
{
	DECLARE_DYNAMIC(EncryptDlg)

public:
	EncryptDlg(CWnd* pParent = nullptr);
	virtual ~EncryptDlg()      override;
  virtual BOOL OnInitDialog() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POSTMAILENCRYPT };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX) override;
  virtual void SetupDynamicLayout() override;

  bool CheckAllFieldsOK();
  void EncryptPassword();

	DECLARE_MESSAGE_MAP()

  CString         m_password1;
  CString         m_password2;
  CString         m_gecodeerd;
  StyleEdit       m_edit1;
  StyleEdit       m_edit2;
  StyleEdit       m_edit3;
  StyleCheckbox   m_checkToon;
  StyleButton     m_buttonCopy;
  StyleButton     m_buttonOK;

public:
  afx_msg void OnAbout();
  afx_msg void OnEnChangeEdit1();
  afx_msg void OnEnChangeEdit2();
  afx_msg void OnEnChangeEdit3();
  afx_msg void OnBnClickedToon();
  afx_msg void OnBnClickedCopy();
  afx_msg void OnBnClickedOk();
};
