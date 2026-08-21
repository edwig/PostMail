/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: PostMailDlg.h
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
#include "SMTPMessage.h"

// CPostMailDlg dialog
class PostMailDlg : public StyleDialog
{
// Construction
public:
	PostMailDlg(SMTPMessage*  p_message
             ,bool          p_viewer
             ,bool          p_editSubject
             ,bool          p_editBody
             ,Profiles&     p_profiles
             ,CWnd*         p_parent = nullptr);
 ~PostMailDlg();
 // Override from CWnd
 virtual BOOL PreTranslateMessage(MSG* pMsg) override;
 virtual void SetupDynamicLayout() override;

  // Dialog Data
	enum { IDD = IDD_POSTMAIL_DIALOG };
 
  XString GetSubject() { return m_sSubject;  }
  XString GetBody()    { return m_sBody;     }

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;
  virtual BOOL OnInitDialog() override;

  // Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
  void    SetRecipients();
  void    SetDeliveryStatus();
  void    SetDisposition();
  void    SetPriority();
  void    SetAttachments();
  void    SetSenderTitle(const XString& p_profile = _T(""));
  void    TryToOpenAttachment(XString p_file);
  void    SetChosenProfile(bool p_force);
  void    InitSystemMenu();
  void    InitTexts();
  void    InitEditOrView();
  void    FillDialog();
  void    FillMessage();
  void    SyncExplorer(BOOL p_force = FALSE);
  XString GetOuterHTML();
  XString GetBodyInnerHTML(const XString& p_html);
  void    ExecHelper(DWORD p_command);
  void    SetEditButtons();
  int     TranslateSizeToHTMLPoints(LONG p_size);
  bool    SplitFontAndSize(const XString& p_defaultFont,XString& p_font,LONG& p_size,XString& p_color);
  XString PlainToBodyText(const XString& p_body);
  XString FilterLocalAnchors(XString& p_html);
  void    PrepareForSend();

  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnPaint();
  afx_msg void ReadIt();
  afx_msg void OnBnClickedAdressen();
  afx_msg void OnBnClickedProfile();
  afx_msg void OnBnClickedConfig();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnSize(UINT nType, int cx, int cy);
	
  DECLARE_MESSAGE_MAP()
  DECLARE_EVENTSINK_MAP()

private:
  bool          m_init   { false   };
  XString       m_sFrom;
  CStringArray* m_sTo    { nullptr };
  CStringArray* m_sCc    { nullptr };
  int           m_num_to { 0 };
  int           m_num_cc { 0 };
  XString       m_sSubject;
  XString       m_sBody;
  bool          m_viewer        { false };
  bool          m_canEditSubject{ false };
  bool          m_canEditBody   { false };
  bool          m_doHTML        { false };
  bool          m_doRTF         { false };
  SMTPMessage*  m_message       { nullptr };
  Profiles&     m_profiles;
  OutBox        m_outbox;

  StyleEdit     m_editNumTo;
  StyleEdit     m_editNumCc;
  StyleEdit     m_editSubject;
  StyleComboBox m_comboTO;
  StyleComboBox m_comboCC;
  StyleComboBox m_comboPrio;
  StyleComboBox m_comboNotify;
  StyleComboBox m_comboDisposition;
  StyleComboBox m_comboTextType;
  StyleListCtrl m_listAttach;
  CBitmap       m_bitmap;
  CImageList    m_images;
  CWnd*         m_wndSubject { nullptr };

  StyleButton   m_buttonAddress;
  StyleButton   m_buttonProfile;
  StyleButton   m_buttonConfig;
  StyleButton   m_buttonSend;
  StyleButton   m_buttonAddAttach;
  StyleButton   m_buttonDelAttach;
  StyleButton   m_buttonShowAttach;
  StyleButton   m_buttonOutBox;

  StyleButton   m_buttonBold;
  StyleButton   m_buttonItalic;
  StyleButton   m_buttonUnder;
  StyleButton   m_buttonStrike;
  StyleButton   m_buttonColor;
  StyleButton   m_buttonFont;

  StyleStatic   m_statTO;
  StyleStatic   m_statCC;
  StyleStatic   m_statSubject;
  StyleStatic   m_statText;
  StyleStatic   m_statAttach;
  StyleStatic   m_statPrior;
  StyleStatic   m_statDeliver;
  StyleStatic   m_statRead;

  // Text and RTF-Text
  StyleRichEdit m_body;
  // WebBrowser for HTML text
  CComQIPtr<IWebBrowser2> m_spBrowser;
  CWnd*         m_explorer { nullptr };

  StyleButton   m_buttonOK;
  StyleButton   m_buttonCancel;

public:
  afx_msg void OnCbnSelchangeTolist();
  afx_msg void OnCbnSelchangePriority();
  afx_msg void OnCbnSelchangeNotify();
  afx_msg void OnCbnSelchangeDisposition();
  afx_msg void OnCbnSelchangeTexttype();
  afx_msg void OnBnClickedAddAttach();
  afx_msg void OnBnClickedDelAttach();
  afx_msg void OnBnClickedShowAttach();
  afx_msg void OnBnClickedSend();
  afx_msg void OnBnClickedCancel();
  afx_msg void OnLvnItemchangedAttach(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnDocumentComplete (LPDISPATCH pDisp, LPVARIANT pURL);
  afx_msg void OnBnClickedBold();
  afx_msg void OnBnClickedItalic();
  afx_msg void OnBnClickedUnder();
  afx_msg void OnBnClickedStrike();
  afx_msg void OnBnClickedColor();
  afx_msg void OnBnClickedFont();
  afx_msg void OnBnClickedOutBox();
  afx_msg void OnEncrypt();
  afx_msg void OnHelpManual();
  afx_msg void OnHelpCommandLine();
  afx_msg void OnHelpAbout();
};
