/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: PostMailDlg.cpp
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
#include "PostMail.h"
#include "PostMailDlg.h"
#include "Message.h"
#include "ChooseProfileDlg.h"
#include "ProfileManagamentDlg.h"
#include "AdresDlg.h"
#include "EncryptDlg.h"
#include "OutboxDlg.h"
#include "Version.h"
#include <FileDialog.h>
#include <mshtml.h>
#include <mshtmhst.h>
#include <mshtmcid.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAboutDlg dialog used for App About

class AboutDlg : public StyleDialog
{
public:
  AboutDlg();

  // Dialog Data
  enum { IDD = IDD_ABOUTBOX };

protected:
  virtual void DoDataExchange(CDataExchange* pDX) override;
  virtual BOOL OnInitDialog() override;

  // Implementation
protected:
  StyleStatic m_staticVersion;
  StyleStatic m_staticText;
  StyleButton m_buttonOK;

  DECLARE_MESSAGE_MAP()
};

AboutDlg::AboutDlg() 
          :StyleDialog(AboutDlg::IDD)
{
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX,IDC_ST_VERSION,m_staticVersion);
  DDX_Control(pDX,IDC_ST_TEXT,   m_staticText);
  DDX_Control(pDX,IDOK,          m_buttonOK);
}

BEGIN_MESSAGE_MAP(AboutDlg,StyleDialog)
END_MESSAGE_MAP()

BOOL
AboutDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();

  CString about = g_message[MESS_ABOUTBOX][g_lang];
  about.TrimRight(_T('.'));
  about.TrimRight(_T(' '));

  SetWindowText(about);
  m_staticVersion.SetWindowText(_T(PRODUCT_NAME " Version: " PRODUCT_VERSION));
  m_staticText   .SetWindowText(_T(COPYRIGHT));

  CenterWindow();
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// PostMailDlg
//
//////////////////////////////////////////////////////////////////////////

// CPostMailDlg dialog
PostMailDlg::PostMailDlg(SMTPMessage * p_message
                        ,bool          p_viewer
                        ,bool          p_editSubject
                        ,bool          p_editBody
                        ,Profiles&     p_profiles
                        ,CWnd*         p_parent /*= nullptr */)
              :StyleDialog(PostMailDlg::IDD, p_parent)
              ,m_profiles(p_profiles)
              ,m_message(p_message)
              ,m_viewer(p_viewer)
              ,m_canEditSubject(p_editSubject)
              ,m_canEditBody(p_editBody)
              ,m_explorer(NULL)
              ,m_init(false)
              ,m_doHTML(false)
              ,m_doRTF(false)
              ,m_num_to(0)
              ,m_num_cc(0)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

  m_sFrom    = p_message->GetSender()->GetRegularFormat(false);
  m_sSubject = p_message->GetSubject();
  m_sBody    = p_message->GetBody();
}

PostMailDlg::~PostMailDlg()
{
}

void PostMailDlg::DoDataExchange(CDataExchange* pDX)
{
  StyleDialog::DoDataExchange(pDX);
  DDX_Control(pDX,IDC_TOLIST,         m_comboTO);
  DDX_Control(pDX,IDC_CCLIST,         m_comboCC);
  DDX_Control(pDX,IDC_NUM_TO,         m_editNumTo,  m_num_to);
  DDX_Control(pDX,IDC_NUM_CC,         m_editNumCc,  m_num_cc);
  DDX_Control(pDX,IDC_SUBJECT,        m_editSubject,m_sSubject);
  DDX_Control(pDX,IDC_MESSAGE,        m_body);
  DDX_Control(pDX,IDC_EDIT_PRIOR,     m_comboPrio);             // m_sPrior;
  DDX_Control(pDX,IDC_EDIT_DELIVER,   m_comboNotify);           // m_sDsn;
  DDX_Control(pDX,IDC_EDIT_READ,      m_comboDisposition);      // m_sMdn;
  DDX_Control(pDX,IDC_ADRES,          m_buttonAddress);
  DDX_Control(pDX,IDC_PROFILE,        m_buttonProfile);
  DDX_Control(pDX,IDC_CONFIG,         m_buttonConfig);
  DDX_Control(pDX,IDC_OUTBOX,         m_buttonOutBox);

  DDX_Control(pDX,IDC_TEXTTYPE,       m_comboTextType);
  DDX_Control(pDX,IDC_BOLD,           m_buttonBold);
  DDX_Control(pDX,IDC_ITALIC,         m_buttonItalic);
  DDX_Control(pDX,IDC_UNDERLINE,      m_buttonUnder);
  DDX_Control(pDX,IDC_STRIKE,         m_buttonStrike);
  DDX_Control(pDX,IDC_FONT,           m_buttonFont);
  DDX_Control(pDX,IDC_COLOR,          m_buttonColor);

  DDX_Control(pDX,IDC_ATTACH,         m_listAttach);
  DDX_Control(pDX,IDC_ADD_ATTACH,     m_buttonAddAttach);
  DDX_Control(pDX,IDC_DEL_ATTACH,     m_buttonDelAttach);
  DDX_Control(pDX,IDC_SHOW_ATTACH,    m_buttonShowAttach);

  DDX_Control(pDX,IDC_STATIC_TO,      m_statTO);
  DDX_Control(pDX,IDC_STATIC_CC,      m_statCC);
  DDX_Control(pDX,IDC_STATIC_SUBJECT, m_statSubject);
  DDX_Control(pDX,IDC_STATIC_TEXT,    m_statText);
  DDX_Control(pDX,IDC_STATIC_ATTACH,  m_statAttach);
  DDX_Control(pDX,IDC_STATIC_PRIOR,   m_statPrior);
  DDX_Control(pDX,IDC_STATIC_DELIVER, m_statDeliver);
  DDX_Control(pDX,IDC_STATIC_LEES,    m_statRead);

  DDX_Control(pDX,IDOK,               m_buttonOK);
  DDX_Control(pDX,IDCANCEL,           m_buttonCancel);
}

void
PostMailDlg::ReadIt()
{
  UpdateData(TRUE);
}

BEGIN_MESSAGE_MAP(PostMailDlg, StyleDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_TIMER()
  ON_WM_SIZE()
  ON_WM_QUERYDRAGICON()
  // Dialog interaction
  ON_EN_KILLFOCUS (IDC_SUBJECT,         ReadIt)
  ON_EN_KILLFOCUS (IDC_MESSAGE,         ReadIt)
  ON_CBN_SELCHANGE(IDC_TOLIST,          OnCbnSelchangeTolist)
  ON_BN_CLICKED   (IDC_ADRES,           OnBnClickedAdressen)
  ON_BN_CLICKED   (IDC_PROFILE,         OnBnClickedProfile)
  ON_BN_CLICKED   (IDC_CONFIG,          OnBnClickedConfig)
  ON_CBN_SELCHANGE(IDC_EDIT_PRIOR,      OnCbnSelchangePriority)
  ON_CBN_SELCHANGE(IDC_EDIT_DELIVER,    OnCbnSelchangeNotify)
  ON_CBN_SELCHANGE(IDC_EDIT_READ,       OnCbnSelchangeDisposition)
  ON_CBN_SELCHANGE(IDC_TEXTTYPE,        OnCbnSelchangeTexttype)
  ON_BN_CLICKED   (IDC_BOLD,            OnBnClickedBold)
  ON_BN_CLICKED   (IDC_ITALIC,          OnBnClickedItalic)
  ON_BN_CLICKED   (IDC_UNDERLINE,       OnBnClickedUnder)
  ON_BN_CLICKED   (IDC_STRIKE,          OnBnClickedStrike)
  ON_BN_CLICKED   (IDC_COLOR,           OnBnClickedColor)
  ON_BN_CLICKED   (IDC_FONT,            OnBnClickedFont)
  ON_NOTIFY       (NM_DBLCLK,IDC_ATTACH,OnLvnItemchangedAttach)
  ON_BN_CLICKED   (IDC_ADD_ATTACH,      OnBnClickedAddAttach)
  ON_BN_CLICKED   (IDC_DEL_ATTACH,      OnBnClickedDelAttach)
  ON_BN_CLICKED   (IDC_SHOW_ATTACH,     OnBnClickedShowAttach)
  ON_BN_CLICKED   (IDC_OUTBOX,          OnBnClickedOutBox)
  ON_BN_CLICKED   (IDOK,                OnBnClickedSend)
  ON_BN_CLICKED   (IDCANCEL,            OnBnClickedCancel)
  // Standard StyleDialog methods
  ON_COMMAND      (ID_THEMA_LIME,       OnStyleLime)
  ON_COMMAND      (ID_THEMA_SKYBLUE,    OnStyleSkyblue)
  ON_COMMAND      (ID_THEMA_PURPLE,     OnStylePurple)
  ON_COMMAND      (ID_THEMA_MUSTARD,    OnStyleMustard)
  ON_COMMAND      (ID_THEMA_MODERATE,   OnStyleModerateGray)
  ON_COMMAND      (ID_THEMA_PUREGRAY,   OnStylePureGray)
  ON_COMMAND      (ID_THEMA_BLACKWHITE, OnStyleBlackWhite)
  ON_COMMAND      (ID_THEMA_DARKTHEME,  OnStyleDark)
  // Menu commands
  ON_COMMAND      (ID_ENCRYPT,          OnEncrypt)
  ON_COMMAND      (ID_HELP_MANUAL,      OnHelpManual)
  ON_COMMAND      (ID_HELP_COMMANDLINE, OnHelpCommandLine)
  ON_COMMAND      (ID_HELP_ABOUT,       OnHelpAbout)
  ON_COMMAND      (ID_CLOSE,            OnBnClickedCancel)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(PostMailDlg, StyleDialog)
  ON_EVENT(PostMailDlg, IDC_EXPLORER, 0x00000103, OnDocumentComplete, VTS_DISPATCH VTS_VARIANT)
END_EVENTSINK_MAP()

// CPostMailDlg message handlers

BOOL PostMailDlg::OnInitDialog()
{
  StyleDialog::OnInitDialog();
  ShowMinMaxButton();

  // Append system menu items
  InitSystemMenu();

  // Fill the dialog
  FillDialog();

  // Fill the text message body
  FillMessage();

  // Set statics and buttons in the correct language
  InitTexts();

  // Enable controls or just use as a viewer
  InitEditOrView();

  // Process the chosen profile
  if(m_profiles.GetChosenProfile() > 0)
  {
    SetChosenProfile(false);
  }

  // Set a timer and set the window in the foreground after 0.2 seconds
  SetTimer(1,200,NULL);

  // Initiate the layout in SetupDynamicLayout
  SetCanResize();

  m_init = TRUE;
  UpdateData(FALSE);

  // Resize m_explorer
  SyncExplorer(TRUE);

  // Use the saved window position
  PostInitDialog();

  return TRUE;
}

// Triggered by SetCanResize
void
PostMailDlg::SetupDynamicLayout()
{
  // See to it that we get a manager;
  StyleDialog::SetupDynamicLayout();

  auto manager = GetDynamicLayout();
  if(manager != nullptr)
  {
    manager->AddItem(IDC_STATIC_TO,     CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_TOLIST,        CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeHorizontal(80));
    manager->AddItem(IDC_NUM_TO,        CMFCDynamicLayout::MoveHorizontal(80), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_ADRES,         CMFCDynamicLayout::MoveHorizontal(80), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_STATIC_PRIOR,  CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_EDIT_PRIOR,    CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));

    manager->AddItem(IDC_STATIC_CC,     CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_CCLIST,        CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeHorizontal(80));
    manager->AddItem(IDC_NUM_CC,        CMFCDynamicLayout::MoveHorizontal(80), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_PROFILE,       CMFCDynamicLayout::MoveHorizontal(80), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_STATIC_DELIVER,CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_EDIT_DELIVER,  CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));

    manager->AddItem(IDC_STATIC_SUBJECT,CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_SUBJECT,       CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeHorizontal(80));
    manager->AddItem(IDC_CONFIG,        CMFCDynamicLayout::MoveHorizontal(80), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_STATIC_LEES,   CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_EDIT_READ,     CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));

    manager->AddItem(IDC_STATIC_TEXT,   CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_MESSAGE,       CMFCDynamicLayout::MoveNone(),         CMFCDynamicLayout::SizeHorizontalAndVertical(90,80));
    manager->AddItem(IDC_TEXTTYPE,      CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_BOLD,          CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(5));
    manager->AddItem(IDC_ITALIC,        CMFCDynamicLayout::MoveHorizontal(95), CMFCDynamicLayout::SizeHorizontal(5));
    manager->AddItem(IDC_UNDERLINE,     CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_STRIKE,        CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_FONT,          CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_COLOR,         CMFCDynamicLayout::MoveHorizontal(90), CMFCDynamicLayout::SizeHorizontal(10));

    manager->AddItem(IDC_STATIC_ATTACH, CMFCDynamicLayout::MoveVertical(80),   CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDC_ATTACH,        CMFCDynamicLayout::MoveVertical(80),   CMFCDynamicLayout::SizeHorizontalAndVertical(90,20));

    manager->AddItem(IDC_ADD_ATTACH,    CMFCDynamicLayout::MoveHorizontalAndVertical(90,80), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_DEL_ATTACH,    CMFCDynamicLayout::MoveHorizontalAndVertical(90,80), CMFCDynamicLayout::SizeHorizontal(10));
    manager->AddItem(IDC_SHOW_ATTACH,   CMFCDynamicLayout::MoveHorizontalAndVertical(90,80), CMFCDynamicLayout::SizeHorizontal(10));

    manager->AddItem(IDC_OUTBOX,        CMFCDynamicLayout::MoveVertical(100),                 CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDOK,              CMFCDynamicLayout::MoveHorizontalAndVertical(100,100),CMFCDynamicLayout::SizeNone());
    manager->AddItem(IDCANCEL,          CMFCDynamicLayout::MoveHorizontalAndVertical(100,100),CMFCDynamicLayout::SizeNone());
  }
}

void
PostMailDlg::InitTexts()
{
  // Set static text on the dialog
  CWnd* pnt = GetDlgItem(IDOK);
  pnt->SetWindowText(g_message[MESS_SEND][g_lang]);
  if(theApp.GetViewer())
  {
    // Do not show the SEND button in a viewer
    pnt->ShowWindow(SW_HIDE);
  }
  pnt = GetDlgItem(IDCANCEL);           pnt->SetWindowText(g_message[MESS_CANCEL]           [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_TO);      pnt->SetWindowText(g_message[MESS_TO]               [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_CC);      pnt->SetWindowText(g_message[MESS_CC]               [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_ATTACH);  pnt->SetWindowText(g_message[MESS_ATTACH]           [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_PRIOR);   pnt->SetWindowText(g_message[MESS_IMPORTANCE]       [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_DELIVER); pnt->SetWindowText(g_message[MESS_DELIVERY]         [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_LEES);    pnt->SetWindowText(g_message[MESS_DISPOSITION]      [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_SUBJECT); pnt->SetWindowText(g_message[MESS_SUBJECT]          [g_lang]);
  pnt = GetDlgItem(IDC_STATIC_TEXT);    pnt->SetWindowText(g_message[MESS_TEXT]             [g_lang]);
  pnt = GetDlgItem(IDC_ADRES);          pnt->SetWindowText(g_message[MESS_BUTTON_ADD]       [g_lang]);
  pnt = GetDlgItem(IDC_PROFILE);        pnt->SetWindowText(g_message[MESS_BUTTON_PROFILE]   [g_lang]);
  pnt = GetDlgItem(IDC_CONFIG);         pnt->SetWindowText(g_message[MESS_BUTTON_CONFIG]    [g_lang]);
  pnt = GetDlgItem(IDC_ADD_ATTACH);     pnt->SetWindowText(g_message[MESS_BUTTON_ADDATTACH] [g_lang]);
  pnt = GetDlgItem(IDC_DEL_ATTACH);     pnt->SetWindowText(g_message[MESS_BUTTON_DELATTACH] [g_lang]);
  pnt = GetDlgItem(IDC_SHOW_ATTACH);    pnt->SetWindowText(g_message[MESS_BUTTON_SHOWATTACH][g_lang]);
  pnt = GetDlgItem(IDC_BOLD);           pnt->SetWindowText(g_message[MESS_BUTTON_BOLD]      [g_lang]);
  pnt = GetDlgItem(IDC_ITALIC);         pnt->SetWindowText(g_message[MESS_BUTTON_ITALIC]    [g_lang]);
  pnt = GetDlgItem(IDC_UNDERLINE);      pnt->SetWindowText(g_message[MESS_BUTTON_UNDERLINE] [g_lang]);
  pnt = GetDlgItem(IDC_STRIKE);         pnt->SetWindowText(g_message[MESS_BUTTON_STRIKE]    [g_lang]);
  pnt = GetDlgItem(IDC_FONT);           pnt->SetWindowText(g_message[MESS_BUTTON_FONT]      [g_lang]);
  pnt = GetDlgItem(IDC_COLOR);          pnt->SetWindowText(g_message[MESS_BUTTON_COLOR]     [g_lang]);
  pnt = GetDlgItem(IDC_OUTBOX);         pnt->SetWindowText(g_message[MESS_OUTBOX]           [g_lang]);
}

void
PostMailDlg::InitEditOrView()
{
  CWnd* pnt = nullptr;

  if(m_viewer)
  {
    m_buttonAddress   .EnableWindow(FALSE);
    m_buttonProfile   .EnableWindow(FALSE);
    m_comboPrio       .EnableWindow(FALSE);
    m_comboNotify     .EnableWindow(FALSE);
    m_comboDisposition.EnableWindow(FALSE);
    m_buttonAddAttach .EnableWindow(FALSE);
    m_buttonDelAttach .EnableWindow(FALSE);
    m_buttonOutBox    .EnableWindow(FALSE);

    m_buttonBold      .EnableWindow(FALSE);
    m_buttonItalic    .EnableWindow(FALSE);
    m_buttonUnder     .EnableWindow(FALSE);
    m_buttonStrike    .EnableWindow(FALSE);
    m_buttonFont      .EnableWindow(FALSE);
    m_buttonColor     .EnableWindow(FALSE);
  }
  else
  {
    // Only enable for RTF/HTML edit
    SetEditButtons();
  }
  if(m_canEditSubject)
  {
    pnt = GetDlgItem(IDC_SUBJECT);
    pnt->SendMessage(EM_SETREADONLY,FALSE,0);
  }
  if(m_canEditBody)
  {
    pnt = GetDlgItem(IDC_MESSAGE);
    m_body.SetReadOnly(FALSE);
  }
}

void
PostMailDlg::InitSystemMenu()
{
  SetSysMenu(IDR_MENU1);
  ShowSysMenu();

  // Add "About..." menu item to system menu.
  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if(pSysMenu != NULL)
  {
    pSysMenu->AppendMenu(MF_SEPARATOR);
    pSysMenu->AppendMenu(MF_STRING, IDM_HELPFILE, g_message[MESS_MANUAL     ][g_lang]);
    pSysMenu->AppendMenu(MF_STRING, IDM_CMDLINE,  g_message[MESS_COMMANDLINE][g_lang]);
    pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, g_message[MESS_ABOUTBOX   ][g_lang]);
  }
  // Set the icon for this dialog.  The framework does this automatically
  // when the application's main window is not a dialog
  SetIcon(m_hIcon,TRUE);    // Set big icon
  SetIcon(m_hIcon,FALSE);   // Set small icon

}

void
PostMailDlg::FillDialog()
{
  SetDeliveryStatus();
  SetRecipients();
  SetDisposition();
  SetPriority();
  SetAttachments();
  SetSenderTitle();
}

void
PostMailDlg::FillMessage()
{
// #ifdef _DEBUG
//   m_sBody = "TESTING";
// #endif

  // Remember where our subject window is
  m_wndSubject = GetDlgItem(IDC_SUBJECT);

  m_comboTextType.ResetContent();

  // Test body text
  m_sBody.TrimLeft();
  if((m_sBody.Left(6).CompareNoCase(_T("<html>")) == 0) && m_message->GetBodyHTML().IsEmpty())
  {
    m_message->AddHtmlBody(m_sBody);
  }

  // Filling of the text
  m_explorer = GetDlgItem(IDC_EXPLORER);
  if(theApp.GetTextMode() == false && (!m_message->GetBodyHTML().IsEmpty() || m_message->GetIsRTF() == false))
  {
    m_doHTML = true;
    // Check if both are empty, and if yes: set appropriate font
    if(m_message->GetBodyHTML().IsEmpty())
    {
      XString font = _T("Arial");
      LONG    size = 10;
      XString color;
      DWORD   colornum = 0;

      XString defaultFont = m_profiles.GetDefaultFont();
      if(!defaultFont.IsEmpty())
      {
        SplitFontAndSize(defaultFont,font,size,color);
        _stscanf_s(color,_T("#%X"),&colornum);
      }
      int points = TranslateSizeToHTMLPoints(size);

      XString document;
      document.Format(_T("<html><body>\n")
                      _T("<font size=\"%d\" face=\"%s\" color=\"#%06X\">\n")
                      _T("%s\n")
                      _T("</font>\n")
                      _T("</body></html>\n")
                     ,points
                     ,font.GetString()
                     ,colornum
                     ,PlainToBodyText(m_sBody).GetString());

      m_message->AddHtmlBody(document);
    }
    if(m_explorer)
    {
      m_spBrowser = m_explorer->GetControlUnknown();
    }
    m_explorer->ShowWindow(SW_SHOW);
    // m_explorer->EnableWindow(m_editBody);
  	m_body.ShowWindow(SW_HIDE);
    m_body.EnableWindow(FALSE);

    m_spBrowser->Navigate(L"res://postmail.exe/message.html",NULL,NULL,NULL,NULL);
    m_spBrowser->put_Resizable(VARIANT_TRUE);

    m_comboTextType.AddString(_T("HTML Message"));
    m_comboTextType.SetCurSel(0);
    m_comboTextType.EnableWindow(FALSE);

  	m_explorer->Invalidate(TRUE);
  }
  else
  {
    // Plain text or RTF text
    m_comboTextType.EnableWindow(m_canEditBody);
    m_comboTextType.AddString(_T("Plain text"));
    m_comboTextType.AddString(_T("RTF Message"));
    m_comboTextType.SetCurSel(0);

    if(m_explorer)
    {
      // m_explorer->EnableWindow(FALSE);
      m_explorer->ShowWindow(SW_HIDE);
    }
    m_body.ShowWindow(SW_SHOW);

    if(m_message->GetIsRTF())
    {
      m_doRTF = true;
      m_body.SetTextMode(TM_RICHTEXT|TM_MULTICODEPAGE|TM_MULTILEVELUNDO);
      m_comboTextType.SetCurSel(1);
      // Place RTF text in the control
      m_body.SetRTFText(m_sBody.c_str());
    }
    else
    {
      // Plain text: replace the selection
      m_body.SetSel(0,-1);
      m_body.ReplaceSel(m_sBody);
    }
  }
}

// Font punt size to HTML Size
int
PostMailDlg::TranslateSizeToHTMLPoints(LONG p_size)
{
  if(p_size <=  8) return 1;
  if(p_size <= 10) return 2;
  if(p_size <= 12) return 3;
  if(p_size <= 14) return 4;
  if(p_size <= 18) return 5;
  if(p_size <= 24) return 6;
  if(p_size <= 36) return 7;

  return 8;
}

// Splitting default font string "Verdana;20;#4577FF" in 
// a string "Verdana"
// a long (20)
// a color string
bool
PostMailDlg::SplitFontAndSize(const XString& p_defaultFont,XString& p_font,LONG& p_size,XString& p_color)
{
  int pos = p_defaultFont.Find(_T(';'));
  if(pos < 0)
  {
    return false;
  }
  p_font = p_defaultFont.Left(pos);
  p_size = _ttoi(p_defaultFont.Mid(pos + 1));

  pos = p_defaultFont.Find(_T(';'),pos + 1);
  if(pos > 0)
  {
    p_color = p_defaultFont.Mid(pos + 1);
    if(p_color.Left(1) == _T("#") && p_color.Mid(1).GetLength() == 6)
    {
      // Reverse the colors
      XString blauw = p_color.Mid(1,2);
      XString groen = p_color.Mid(3,2);
      XString rood  = p_color.Mid(5,2);
      // HTML colors are the inverse of the desktop colors!!
      p_color = _T("#") + rood + groen + blauw;
    }
  }
  return true;
}

// Prepare for HTML
XString 
PostMailDlg::PlainToBodyText(const XString& p_body)
{
  XString body(p_body);
  body.Replace(_T("\r"),_T(""));
  body.Replace(_T("\n"),_T("<br>"));

  return body;
}

void
PostMailDlg::SetEditButtons()
{
  m_buttonBold  .EnableWindow(m_doRTF || m_doHTML);
  m_buttonItalic.EnableWindow(m_doRTF || m_doHTML);
  m_buttonUnder .EnableWindow(m_doRTF || m_doHTML);
  m_buttonStrike.EnableWindow(m_doRTF || m_doHTML);
  m_buttonFont  .EnableWindow(m_doRTF || m_doHTML);
  m_buttonColor .EnableWindow(m_doRTF || m_doHTML);
}

void
PostMailDlg::SetRecipients()
{
  // Clear settings
  m_comboTO.ResetContent();
  m_comboCC.ResetContent();

  // Add the addresses to the combo box
  // Make the list of recipients (TO)
  for (int i=0;i < m_message->GetNumberOfRecipients(TO); ++i)
  {
    SMTPAddress recipient = m_message->GetRecipient(i, TO);
    m_comboTO.AddString(recipient.GetRegularFormat());
  }

  for (int i=0;i < m_message->GetNumberOfRecipients(CC); ++i)
  {
    SMTPAddress recipient = m_message->GetRecipient(i, CC);
    m_comboCC.AddString(recipient.GetRegularFormat());
  }

  for (int i=0;i < m_message->GetNumberOfRecipients(BCC); ++i)
  {
    SMTPAddress recipient = m_message->GetRecipient(i, BCC);
    m_comboCC.AddString(XString(_T("(BCC) ")) + recipient.GetRegularFormat());
  }

  m_comboTO.SetCurSel(0);
  m_comboCC.SetCurSel(0);
  m_num_to = m_message->GetNumberOfRecipients(TO);
  m_num_cc = m_message->GetNumberOfRecipients(CC) + 
             m_message->GetNumberOfRecipients(BCC);
  UpdateData(FALSE);
}

void
PostMailDlg::SetDeliveryStatus()
{
  m_comboNotify.ResetContent();

  m_comboNotify.AddString(g_message[MESS_DELIVERY_NEVER  ][g_lang]);
  m_comboNotify.AddString(g_message[MESS_DELIVERY_FULL   ][g_lang]);
  m_comboNotify.AddString(g_message[MESS_DELIVERY_FAILED ][g_lang]);
  m_comboNotify.AddString(g_message[MESS_DELIVERY_NOTIFY ][g_lang]);
  m_comboNotify.AddString(g_message[MESS_DELIVERY_DELAY  ][g_lang]);
  m_comboNotify.AddString(g_message[MESS_DELIVERY_HEADERS][g_lang]);

  int delivery = 0;
  NOTIFY_TYPE dsn = m_message->GetNotify();
  if(dsn == NOTIFY_NEVER)
  {
    delivery = 0;
  }
  else
  {
    if(dsn & NOTIFY_FAILURE)
    {
      delivery = 2;
    }
    if(dsn & NOTIFY_FULL) 
    {
      delivery = 1;
    }
    if(dsn & NOTIFY_SUCCESS)
    {
      delivery = 3;
    }
    if(dsn & NOTIFY_DELAY)
    {
      delivery = 4;
    }
    if(dsn & NOTIFY_HEADER)
    {
      delivery = 5;
    }
  }
  m_comboNotify.SetCurSel(delivery);
}


BOOL 
PostMailDlg::PreTranslateMessage(MSG* pMsg)
{
  if((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
  {
    CWnd* wnd = GetFocus();
    if(wnd)
    {
      if(wnd->GetSafeHwnd() == m_body.GetSafeHwnd())
      {
        // Enter in RichEditControl
        wnd->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
      }
      else if(wnd->GetSafeHwnd() == m_wndSubject->GetSafeHwnd())
      {
        // Enter in subject -> Go to message
        if(m_doHTML)
        {
          m_explorer->SetFocus();
        }
        else
        {
          m_body.SetFocus();
        }
      }
      else if(m_doHTML)
      {
        // Enter in HTML control
        ExecHelper(IDM_LINEBREAKNORMAL);
      }
    }
    // Do not process further in PostMailDlg
    return TRUE;                
  }

  if((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_DELETE))
  {
    CWnd* wnd = GetFocus();
    if(wnd && 
       wnd->GetSafeHwnd() != m_wndSubject->GetSafeHwnd() && 
       wnd->GetSafeHwnd() != m_body.GetSafeHwnd() && 
       m_doHTML)
    {
      ExecHelper(IDM_DELETE);
      return TRUE;
    }
  }

  return CWnd::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////

void
PostMailDlg::OnCbnSelchangeNotify()
{
  int ind = m_comboNotify.GetCurSel();
  if(ind >= 0)
  {
    NOTIFY_TYPE notify = NOTIFY_NEVER;
    switch(ind)
    {
      case 0: notify = NOTIFY_NEVER;   break;
      case 1: notify = NOTIFY_FULL;    break;
      case 2: notify = NOTIFY_FAILURE; break;
      case 3: notify = NOTIFY_SUCCESS; break;
      case 4: notify = NOTIFY_DELAY;   break;
      case 5: notify = NOTIFY_HEADER;  break;
    }
    m_message->SetNotify(notify);
  }
}

void
PostMailDlg::SetDisposition()
{
  m_comboDisposition.ResetContent();

  // Mail disposition notification
  m_comboDisposition.AddString(g_message[MESS_DISP_NO ][g_lang]);
  m_comboDisposition.AddString(g_message[MESS_DISP_YES][g_lang]);

  m_comboDisposition.SetCurSel(m_message->GetDisposition());
}

void
PostMailDlg::OnCbnSelchangeDisposition()
{
  int ind = m_comboDisposition.GetCurSel();
  if(ind >= 0)
  {
    m_message->SetDisposition(ind > 0);
  }
}

void
PostMailDlg::SetPriority()
{
  m_comboPrio.ResetContent();

  m_comboPrio.AddString(g_message[MESS_LOW   ][g_lang]);
  m_comboPrio.AddString(g_message[MESS_NORMAL][g_lang]);
  m_comboPrio.AddString(g_message[MESS_HIGH  ][g_lang]);

  m_comboPrio.SetCurSel(m_message->GetPriorityCode());
}

void
PostMailDlg::OnCbnSelchangePriority()
{
  int ind = m_comboPrio.GetCurSel();
  if(ind >= 0)
  {
    m_message->SetPriority(ind);
  }
}

void
PostMailDlg::SetAttachments()
{
  m_listAttach.DeleteAllItems();

  XString attachment = g_message[MESS_ATTACHMENT][g_lang];
  XString sizeText   = g_message[MESS_SIZE_TEXT] [g_lang];

  m_listAttach.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EDITLABELS);
  m_listAttach.InsertColumn(0,attachment,LVCFMT_LEFT,700);
  m_listAttach.InsertColumn(1,sizeText,  LVCFMT_LEFT,150);

  // Make the list of attachments
  for (int ind = 0;ind < m_message->GetNumberOfAttachments(); ++ind)
  {
    SMTPAttachment* attach = m_message->GetAttachment(ind);
    XString file = attach->GetFilename();
    XString size = attach->PrintableSize();
    
    m_listAttach.InsertItem(LVIF_TEXT|LVIF_STATE,ind,file,0,0,0,0);
    m_listAttach.SetItemText(ind,1,size);
  }
  if(m_message->GetNumberOfAttachments() > 0)
  {
    m_listAttach.SetSelectionMark(0);
    m_listAttach.SetItemState(0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
  }  
}

void
PostMailDlg::SetSenderTitle(const XString& p_profile /*=""*/)
{
  XString title(WhoAmI);
  if(m_viewer)
  {
    title += g_message[MESS_VIEWER][g_lang]; // " VIEWER "
    title += m_message->GetFilename();
  }
  else
  {
    title += g_message[MESS_MAILFROM][g_lang]; // " - Mailing from: "

    if(!p_profile.IsEmpty())
    {
      title.AppendFormat(_T(" [%s]"),p_profile.GetString());
    }
  }
  SetWindowText(title);
}

void
PostMailDlg::SetChosenProfile(bool p_forceer)
{
  int gekozen = m_profiles.GetChosenProfile();
  Profile* profile = m_profiles.GetProfile(gekozen);
  if(profile)
  {
    // Try to copy as much as possible from the profile
    // But override with the data from the current mailfile
    m_message->SetSender ((m_sFrom.IsEmpty() || p_forceer) ? profile->m_emailAddress : m_sFrom);
    m_message->SetHost   (profile->m_smtpServer);
    m_message->SetPort   (profile->m_smtpPort);
    m_message->SetDoLogin(profile->m_useLogin);

    // Set window title
    // But take in account the "FROM" from the current mailfile
    if (m_sFrom.IsEmpty() || p_forceer)
    {
      m_sFrom = profile->m_emailAddress;
      SetSenderTitle(profile->m_profileName);
    }

    // Possibly set a login
    if(profile->m_useLogin)
    {
      m_message->SetMailID  (profile->m_loginUser);
      m_message->SetPassword(profile->m_password);
    }
    else
    {
      m_message->SetMailID(_T(""));
      m_message->SetPassword(_T(""));
    }
  }
}

void 
PostMailDlg::OnDocumentComplete(LPDISPATCH /*pDisp*/, LPVARIANT /*pURL*/)
{
  CComPtr<IDispatch> disp;
  HRESULT hr = m_spBrowser->get_Document(&disp);
  if(SUCCEEDED(hr) && disp)
  {
    CComQIPtr<IHTMLDocument2,&IID_IHTMLDocument2> doc = disp;

    // Set body
    IHTMLElement* body = NULL;
    hr = doc->get_body(&body);
    if(SUCCEEDED(hr) && body)
    { 
      XString html = GetBodyInnerHTML(m_message->GetBodyHTML());
      CComBSTR htmlBody(html);
      hr = body->put_innerHTML(htmlBody);

      if(m_canEditBody)
      {
        // BODY Element set to editable
        CComQIPtr<IHTMLElement3,&IID_IHTMLElement3> elem = body;
        if(elem)
        {
          elem->put_contentEditable(L"true");
        }
      }
    }
  }
}

XString 
PostMailDlg::GetBodyInnerHTML(const XString& p_html)
{
  XString html(p_html);
  XString altHtml(p_html);
  altHtml.MakeLower();

  // Find body
  int pos1 = altHtml.Find(_T("<body>"));
  int pos2 = altHtml.Find(_T("</body>"));
  if(pos1 < 0 || pos2 < 0)
  {
    return p_html;
  }
  html = html.Left(pos2);
  html = html.Mid(pos1 + 6);

  return html;
}

// Getting the total message HTML
XString
PostMailDlg::GetOuterHTML()
{
  CComPtr<IDispatch> disp;
  HRESULT hr = m_spBrowser->get_Document(&disp);
  if(SUCCEEDED(hr) && disp)
  {
    CComQIPtr<IHTMLDocument2,&IID_IHTMLDocument2> doc = disp;

    // Set body
    IHTMLElement* body = NULL;
    hr = doc->get_body(&body);
    if(SUCCEEDED(hr) && body)
    {
      CComBSTR bhtml;
      hr = body->get_innerHTML(&bhtml);
      if(SUCCEEDED(hr))
      {
        // Make outer HTML for the total message
        XString html(bhtml);
        html = FilterLocalAnchors(html);
        html = _T("<html><body>\n") + html + _T("\n</body></html>\n");
        return html;
      }
    }
  }
  return _T("");
}

// Local anchors in the HTML must be replaced by a local resource
// By prefixing our own local HTML position by "res://postmail.exe/message.html#position"
// This cannot be send through the email, as it does not exist at the other end
// so we strip it again before sending
XString
PostMailDlg::FilterLocalAnchors(XString& p_html)
{
  XString html(p_html);
  html.MakeLower();
  int pos = html.Find(_T("res://postmail.exe/message.html#"));

  while(pos >= 0)
  {
    p_html = p_html.Left(pos) + p_html.Mid(pos + 32);
    html   = p_html;
    html.MakeLower();
    pos    = html.Find(_T("res://postmail.exe/message.html#"),pos + 1);
  }
  return p_html;
}

void    
PostMailDlg::ExecHelper(DWORD p_command)
{
  CComPtr<IDispatch> disp;
  HRESULT hr = m_spBrowser->get_Document(&disp);
  if(SUCCEEDED(hr) && disp)
  {
    CComQIPtr<IHTMLDocument2,&IID_IHTMLDocument2> doc = disp;
    if(doc)
    {
      CComQIPtr<IOleCommandTarget> cmdTarg = doc;
      if(cmdTarg)
      {
        OLECMD ocmd = { p_command, 0};
        if (S_OK == cmdTarg->QueryStatus(&CGID_MSHTML,1,&ocmd,NULL))
        {
          long status = ocmd.cmdf;
          long minSupportLevel = OLECMDF_SUPPORTED|OLECMDF_ENABLED;
          long execOpt         = OLECMDEXECOPT_DODEFAULT;

          if((status & minSupportLevel) == minSupportLevel)
          {
            cmdTarg->Exec(&CGID_MSHTML,p_command,execOpt,NULL,NULL);
          }
        }
      }
    }
  }
}

// Last step before sending
// Getting the correct body (TEXT, RTF, HTML)
void
PostMailDlg::PrepareForSend()
{
  if(m_doHTML)
  {
    m_sBody = GetOuterHTML();
    m_message->ResetHTMLBodyAfterEdit();
    m_message->SetEncoding(Encoding::UTF8);
  }
  else if(m_doRTF)
  {
    m_sBody = m_body.GetRTFText();
  }
  else
  {
    m_body.SetSel(0,-1);
    m_sBody = m_body.GetSelText();
    m_sBody.Replace(_T("\r"),_T("\r\n"));
  }
}

//////////////////////////////////////////////////////////////////////////
//
// Message handlers
//
//////////////////////////////////////////////////////////////////////////

void
PostMailDlg::OnTimer(UINT_PTR nIDEvent)
{
  if(nIDEvent == 1)
  {
    KillTimer(1);
    // Place top window and force the repainting
    SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  	// Resize m_explorer
	  SyncExplorer(TRUE);
    // Release after 1 second
    SetTimer(2,1000,NULL);
  }
  if(nIDEvent == 2)
  {
    KillTimer(2);
    // Release top window
    SetWindowPos(&CWnd::wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	  // Resize m_explorer
	  SyncExplorer(TRUE);
  }
}

void PostMailDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if((nID & 0xFFF0) == IDM_ABOUTBOX)
  {
    OnHelpAbout();
  }
  else if((nID & 0xFFF0) == IDM_CMDLINE)
  {
    OnHelpCommandLine();
  }
  else if((nID & 0xFFF0) == IDM_HELPFILE)
  {
    OnHelpManual();
  }
  else
  {
    StyleDialog::OnSysCommand(nID,lParam);
  }
}

void
PostMailDlg::OnEncrypt()
{
  EncryptDlg dlg(this);
  dlg.DoModal();
}

void
PostMailDlg::OnHelpManual()
{
  theApp.DoHelpFile();
}

void
PostMailDlg::OnHelpCommandLine()
{
  theApp.DoHelpPage();
}

void
PostMailDlg::OnHelpAbout()
{
  AboutDlg dlgAbout;
  dlgAbout.DoModal();
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void PostMailDlg::OnPaint() 
{
  if (IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    StyleDialog::OnPaint();
  }
}

void 
PostMailDlg::OnSize(UINT nType, int cx, int cy)
{
  StyleDialog::OnSize(nType,cx,cy);

  // Keep DocEditor and HTMLEditor in sync on the screen
  SyncExplorer();
}

void
PostMailDlg::SyncExplorer(BOOL p_force)
{
  if(m_body && m_explorer && m_explorer->m_hWnd)
  {
    CRect rect;
    m_body.GetWindowRect(&rect);
    ScreenToClient(&rect);
    m_explorer->MoveWindow(rect);
    if(p_force)
    {
      // VERY VERY DIRTY HACK!!!
      // The explorer windows sometimes (20% of the time) optimizes itself out
      // By resizing the tiniest of possibilities (1 pixel) you force the explorer
      // plugin to show itself!!!
      rect.InflateRect(1,1);
      m_explorer->MoveWindow(rect);
      rect.InflateRect(-1,-1);
      m_explorer->MoveWindow(rect);
    }
  }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR PostMailDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void 
PostMailDlg::OnCbnSelchangeTolist()
{
  CString text;
  m_comboTO.GetWindowText(text);
  int ind = m_comboTO.FindString(-1,text);
  if(ind < 0 && !text.IsEmpty())
  {
    m_comboTO.AddString(text);
  }
}

void
PostMailDlg::OnBnClickedAdressen()
{
  AdresDlg dlg(this,m_message);
  dlg.DoModal();
  SetRecipients();

  m_message->LogRecipients();
}

void
PostMailDlg::OnBnClickedProfile()
{
  // Check that we have profiles to choose from
  if(m_profiles.GetNumberOfProfiles() == 0)
  {
    // MESS_NOPROFILE
    // It is not possible to choose a profile, because there are none yet
    // Create a profile first with the 'Configure' button
    theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_NOPROFILE][g_lang],WhoAmI,MB_OK|MB_ICONINFORMATION);
  }

  // Show profiles dialog
  ChooseProfileDlg dlg(this,m_profiles,!m_message->GetFilename().IsEmpty());
  if(dlg.DoModal() == IDOK)
  {
    if(dlg.GetReread())
    {
      m_message->ReReadFile();
      FillDialog();
      FillMessage();
    }
    else
    {
      SetChosenProfile(true);
    }
    UpdateData(FALSE);
  }
}

void 
PostMailDlg::OnBnClickedConfig()
{
  ProfileManagamentDlg dlg(this,m_profiles);
  dlg.DoModal();
}

void
PostMailDlg::OnCbnSelchangeTexttype()
{
  int ind = m_comboTextType.GetCurSel();
  if(ind >= 0)
  {
    XString text;
    m_comboTextType.GetLBText(ind,text);

    if(text.CompareNoCase(_T("RTF Message")) == 0)
    {
      if(m_doRTF == false)
      {
        // Allow RTF mode
        m_doRTF = true;
        m_body.SetTextMode(TM_RICHTEXT);
      }
    }
    if(text.CompareNoCase(_T("Plain text")) == 0)
    {
      if(m_doRTF == true)
      {
        // Reset all effects (apart from fonts)
        CHARFORMAT format;
        memset(&format,0,sizeof(CHARFORMAT));
        format.cbSize      = sizeof(CHARFORMAT);
        format.dwMask      = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_COLOR | CFM_FACE | CFM_SIZE;
        format.dwEffects   = 0;           // All effects to OFF
        format.crTextColor = RGB(0,0,0);  // Text should be black
        format.yHeight     = 200;         // Font size = TWIPS * 10 point
        _tcsncpy_s(format.szFaceName,LF_FACESIZE,_T("Verdana"),LF_FACESIZE);
        // Select everything and perform formatting upon it
        m_body.SetSel(0,-1);
        m_body.SetSelectionCharFormat(format);

        // Now reset the RTF Mode
        m_doRTF = false;
        m_body.SetTextMode(TM_PLAINTEXT);
      }
    }
  }
  SetEditButtons();
}

void 
PostMailDlg::OnBnClickedBold()
{
  if(m_doRTF)
  {
    // Getting if it's bold or not
    CHARFORMAT form;
    memset(&form,0,sizeof(CHARFORMAT));
    form.dwMask = CFM_BOLD;
    form.cbSize = sizeof(CHARFORMAT);
    m_body.GetSelectionCharFormat(form);

    // Flip the bold setting
    CHARFORMAT format;
    memset(&format,0,sizeof(CHARFORMAT));
    format.cbSize = sizeof(CHARFORMAT);
    format.dwMask = CFM_BOLD;
    format.dwEffects = form.dwEffects & CFE_BOLD ? 0 : CFE_BOLD;

    m_body.SetSelectionCharFormat(format);
  }
  else if(m_doHTML)
  {
    ExecHelper(IDM_BOLD);
  }
}
void 
PostMailDlg::OnBnClickedItalic()
{
  if(m_doRTF)
  {
    // Getting if it's italic or not
    CHARFORMAT form;
    memset(&form,0,sizeof(CHARFORMAT));
    form.dwMask = CFM_ITALIC;
    form.cbSize = sizeof(CHARFORMAT);
    m_body.GetSelectionCharFormat(form);

    // Flip the italic setting
    CHARFORMAT format;
    memset(&format,0,sizeof(CHARFORMAT));
    format.cbSize = sizeof(CHARFORMAT);
    format.dwMask = CFM_ITALIC;
    format.dwEffects = form.dwEffects & CFE_ITALIC ? 0 : CFE_ITALIC;

    m_body.SetSelectionCharFormat(format);
  }
  else if(m_doHTML)
  {
    ExecHelper(IDM_ITALIC);
  }
}

void 
PostMailDlg::OnBnClickedUnder()
{
  if(m_doRTF)
  {
    // Getting if it's underline or not
    CHARFORMAT form;
    memset(&form,0,sizeof(CHARFORMAT));
    form.dwMask = CFM_UNDERLINE;
    form.cbSize = sizeof(CHARFORMAT);
    m_body.GetSelectionCharFormat(form);

    // Flip the underline setting
    CHARFORMAT format;
    memset(&format,0,sizeof(CHARFORMAT));
    format.cbSize = sizeof(CHARFORMAT);
    format.dwMask = CFM_UNDERLINE;
    format.dwEffects = form.dwEffects & CFE_UNDERLINE ? 0 : CFE_UNDERLINE;

    m_body.SetSelectionCharFormat(format);
  }
  else if(m_doHTML)
  {
    ExecHelper(IDM_UNDERLINE);
  }
}

void 
PostMailDlg::OnBnClickedStrike()
{
  if(m_doRTF)
  {
    // Getting if it's strike-through or not
    CHARFORMAT form;
    memset(&form,0,sizeof(CHARFORMAT));
    form.dwMask = CFM_STRIKEOUT;
    form.cbSize = sizeof(CHARFORMAT);
    m_body.GetSelectionCharFormat(form);

    // Flip the strike setting
    CHARFORMAT format;
    memset(&format,0,sizeof(CHARFORMAT));
    format.cbSize = sizeof(CHARFORMAT);
    format.dwMask = CFM_STRIKEOUT;
    format.dwEffects = form.dwEffects & CFE_STRIKEOUT ? 0 : CFE_STRIKEOUT;

    m_body.SetSelectionCharFormat(format);
  }
  else if(m_doHTML)
  {
    ExecHelper(IDM_STRIKETHROUGH);
  }
}


UINT_PTR CALLBACK 
CdlgHook(HWND hdlg,UINT uiMsg,WPARAM /*wParam*/, LPARAM lParam)
{
  if(uiMsg == WM_INITDIALOG)
  {
    CHOOSECOLOR *pcc = (CHOOSECOLOR*)lParam;
    CRect *rc = (CRect*)pcc->lCustData;
    if(rc)
    {
      SetWindowPos(hdlg,HWND_TOP,rc->right,rc->bottom,0,0,SWP_NOZORDER|SWP_NOSIZE);
    }
    // Choose a text color
    SetWindowText(hdlg,g_message[MESS_CHOOSECOLOR][g_lang]);
  }
  return 0;
}

void
PostMailDlg::OnBnClickedColor()
{
  if(m_doRTF)
  {
    // Getting the font color
    CHARFORMAT form;
    memset(&form,0,sizeof(CHARFORMAT));
    form.dwMask = CFM_COLOR;
    form.cbSize = sizeof(CHARFORMAT);
    m_body.GetSelectionCharFormat(form);
    int color = form.crTextColor;

    // Get new color from dialog
    CColorDialog dlg(color, CC_FULLOPEN | CC_RGBINIT,this);
    dlg.m_cc.Flags |= CC_ENABLEHOOK;
    dlg.m_cc.lpfnHook  = CdlgHook;

    if(dlg.DoModal()==IDOK)
    {
      // Set new color on the text
      COLORREF cr = dlg.GetColor();

      CHARFORMAT format;
      memset(&format,0,sizeof(CHARFORMAT));
      format.cbSize = sizeof(CHARFORMAT);
      format.dwMask = CFM_COLOR;
      format.crTextColor = cr;
      m_body.SetSelectionCharFormat(format);
    }
  }
  else if(m_doHTML)
  {
    // Change the color of the font
    ExecHelper(IDM_FONT);
  }
}

void
PostMailDlg::OnBnClickedFont()
{
  if(m_doRTF)
  {
    // Getting the font information
    CHARFORMAT form;
    memset(&form,0,sizeof(CHARFORMAT));
    form.dwMask = CFM_CHARSET | CFM_SIZE | CFM_OFFSET | CFM_FACE;
    form.cbSize = sizeof(CHARFORMAT);
    m_body.GetSelectionCharFormat(form);

    // Choose a font
    DWORD flags = CF_SCREENFONTS | CF_NOSCRIPTSEL | CF_NOVERTFONTS | CF_FORCEFONTEXIST;
    CFontDialog dlg(form,flags,NULL,this);
    if(dlg.DoModal() == IDOK)
    {
      // Change the font
      CHARFORMAT format;
      memset(&format,0,sizeof(CHARFORMAT));
      format.cbSize = sizeof(CHARFORMAT);
      dlg.GetCharFormat(format);
      m_body.SetSelectionCharFormat(format);
    }
  }
  else if(m_doHTML)
  {
    ExecHelper(IDM_FONT);
  }
}

void 
PostMailDlg::TryToOpenAttachment(XString p_file)
{
  int pos = p_file.ReverseFind(_T('.'));
  if(pos >= 0)
  {
    XString extension = p_file.Mid(pos + 1);
    extension.MakeLower();  
    if(extension == _T("com") || extension == _T("exe") ||
       extension == _T("bat") || extension == _T("cmd"))
    {
      // For security reasons, this attachment has been blocked for opening
      theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_BLOCKED_ATTACH][g_lang],WhoAmI,MB_OK|MB_ICONERROR);
      return;
    }
  }
  theApp.WideMessageBox(GetSafeHwnd(),g_message[MESS_DO_NOT_EDIT][g_lang],WhoAmI,MB_OK|MB_ICONWARNING);
  // Show the file
  ShellExecute(GetSafeHwnd(),_T("open"),p_file,NULL,NULL,SW_SHOW);
}

void 
PostMailDlg::OnLvnItemchangedAttach(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  XString file = m_listAttach.GetItemText(pNMLV->iItem,0);
  
  TryToOpenAttachment(file);
  
  // Ready with handler
  *pResult = 0;
}

void 
PostMailDlg::OnBnClickedAddAttach()
{
  // Select attachment for the email
  XString title = g_message[MESS_ADDATTACHMENT][g_lang];
  DocFileDialog dialog(GetSafeHwnd(),true,title); 
  if(dialog.DoModal() == IDOK)
  {
    // Create attachment
    XString file = dialog.GetChosenFile();
    SMTPAttachment* attach = new SMTPAttachment();
    attach->Attachment(file);
    m_message->AddAttachment(attach);

    // Put attachment in the list  
    int ind = m_listAttach.GetItemCount();
    m_listAttach.InsertItem(LVIF_TEXT|LVIF_STATE,ind,file,0,0,0,0);
    m_listAttach.SetItemText(ind,1,attach->PrintableSize());
    m_listAttach.SetItemState(ind,LVIS_SELECTED,LVIS_SELECTED);
	m_listAttach.SetFocus();
  }
}

void 
PostMailDlg::OnBnClickedDelAttach()
{
  int ind = m_listAttach.GetSelectionMark();
  if(ind >= 0)
  {
    XString file = m_listAttach.GetItemText(ind,0);
    XString message;
    // Do you want to remove attachment [%s] ?
    message.Format(g_message[MESS_REMOVEATTACHMENT][g_lang],file.GetString());
    if(theApp.WideMessageBox(GetSafeHwnd(),message,WhoAmI,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION) == IDYES)
    {
      if(m_listAttach.DeleteItem(ind))
      {
        m_message->RemoveAttachment(ind);
      }
    }
  }
}

void 
PostMailDlg::OnBnClickedShowAttach()
{
  int ind = m_listAttach.GetSelectionMark();
  // Optionally select first item
  if(ind < 0 && m_listAttach.GetItemCount() > 0)
  {
	  ind = 0;
  }
  if(ind >= 0)
  {
    XString file = m_listAttach.GetItemText(ind,0);
    TryToOpenAttachment(file);
  }
}

void
PostMailDlg::OnBnClickedOutBox()
{
  OutBoxDlg outbox(this,m_outbox);
  outbox.DoModal();
}

void
PostMailDlg::OnBnClickedSend()
{
  // Make sure we can read all attachments (and are not opened in a viewer)
  if(m_message->CheckAttachments() == false)
  {
    return;
  }
  // Getting the correct body (TXT, RTF, HTML)
  PrepareForSend();

  StyleDialog::OnOK();
}

void
PostMailDlg::OnBnClickedCancel()
{
  theApp.ResetErrorState();
  OnCancel();
}
