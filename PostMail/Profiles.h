/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Profiles.h
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
#include <vector>

#define DEFAULT_PROFILES    _T("PostMail.DefaultProfiles.xml")

#define DEFAULT_SMTP_PORT 25
#define SECURE_SMTP_PORT  587

#define EXPAND_KEY   _T(":!@#$%:")

class XMLMessage;
class XMLElement;

typedef struct _profile
{
  int       m_number;				    // Ordinal number of the profiles.xml file
  bool      m_private;		      // Private (mutable) profile op APPDATA (true) of general profile (false)
  bool      m_mutable;          // Personal profile, or access rights on the PostMail directory 
  XString   m_profileName;			// Display name of the profile
  XString   m_emailAddress;     // Mail address as sender of the mail
  XString   m_bccAddress;       // Blind carbon copy at sending
  XString   m_smtpServer;       // Default mail server
  int       m_smtpPort;         // Port number of the mailserver
  bool      m_isStandard;       // It is a standard profile
  bool      m_useLogin;         // Should we login on the server
  XString   m_loginUser;        // User for the login on the mailserver
  XString   m_password;         // Password for the login on the mailserver
}
Profile;

typedef std::vector<Profile> ProfileMap;
typedef std::vector<XString> MailServers;
typedef std::vector<XString> BoxHistory;

class Profiles
{
public:
  Profiles();
 ~Profiles();

  // Commando's 
  bool          ReadProfiles();
  bool          WriteProfiles();
  void          ReadMailServers();
  bool          AddProfile(Profile* p_profile);
  void          RemoveProfile(const XString& p_name);
  bool          HasACompleteProfile();
  void          CheckMailServer(XString& p_server);
  void          ProfileUpgrade(int p_profile);
  void          ProfileDowngrade(int p_profile);
  void          ResetOutboxHistory();
  void          AddOutboxHistory(const XString& p_directory);
  bool          WriteCentralSettings(bool p_font = false);
  bool          CreateProfileMap(const XString& p_name);

  // Setters
  void          SetChanged();
  bool          SetChosenProfile(int p_index);
  bool          SetChosenProfile(const XString& p_name);
  void          SetDoNotAskAgain(bool p_ask);
  void          SetDefaultFont(const XString& p_defaultFont);
  void          SetCentralLogfile(const XString& p_logfile);
  void          SetTypeOfOutbox(BoxType p_type);
  void          SetOutboxSelection(DateSel p_selection);
  void          SetOutboxDirectory(const XString& p_directory);
  void          SetOutboxMAX(int p_maxfiles);
  void          SetCommonOutbox(const XString& p_directory);

  // Getters
  int           GetNumberOfProfiles();
  int           GetChosenProfile();
  Profile*      GetStandardProfile();
  Profile*      GetProfile(const XString& p_name);
  Profile*      GetProfile(int p_index);
  bool          GetDoNotAskAgain();
  XString       GetDefaultMailServer();
  MailServers&  GetMailServers();
  XString       GetProfileMap();
  XString       GetExePath();
  XString       GetCommonOutbox();
  XString       GetDefaultFont();
  XString       GetCentralLogfile();
  bool          GetHasCentralLogfile();
  bool          GetCanModifyOutbox();
  BoxType       GetOutboxType();
  DateSel       GetOutboxSelection();
  XString       GetOutboxDirectory();
  int           GetOutboxMAX();
  BoxHistory&   GetOutboxHistory();
  bool          GetWriteAccess();
  
private:       
  void          FindAppData(int p_directory);
  bool          CreateProfileDirectory();
  XString       EncryptPassword(const XString& p_key,const XString& p_string);
  XString       DecryptPassword(const XString& p_key,const XString& p_string);
  int           ReadProfilesFile(const XString& p_fileName,bool p_personal);
  void          CheckDefaults(const XString& p_fileName);
  void          CheckDefaults(Profile& p_profile);
  int           CreateProfilesInMemory();
  int           GetHighestProfileNumber();
  void          SwapProfiles(int p_one,int p_two);
  XString       StripSingleSignOn(const XString& p_user);
  XString       ReadSetting (XMLMessage* p_doc,XMLElement* p_node,const XString& p_name,const XString& p_default = _T(""));
  bool          WriteSetting(XMLMessage* p_doc,XMLElement* p_node,const XString& p_name,const XString& p_value,const XString& p_before = _T(""));
  XString       StripPortnumber(const XString& p_server);

  bool          m_changed;
  XString       m_appData;
  XString       m_profileMap;
  XString       m_sharedOutbox;
  XString       m_centralLogfile;
  XString       m_defaultFont;
  ProfileMap    m_profiles;
  MailServers   m_servers;
  int           m_chosen;
  bool          m_canChangeOutbox;
  bool          m_doNotAskAgain;
  bool          m_writeaccess;
  // Outbox
  BoxType       m_outboxType;
  DateSel       m_outboxSelection;
  XString       m_outboxDirectory;
  int           m_outboxMAX;
  BoxHistory    m_outboxHistory;
};

inline int
Profiles::GetNumberOfProfiles()
{
  return (int)m_profiles.size();
}

inline void
Profiles::SetChanged()
{
  m_changed = true;
}

inline int
Profiles::GetChosenProfile()
{
  return m_chosen;
}

inline void
Profiles::SetDoNotAskAgain(bool p_vragen)
{
  m_doNotAskAgain = p_vragen;
  SetChanged();
}

inline bool
Profiles::GetDoNotAskAgain()
{
  return m_doNotAskAgain;
}

inline MailServers& 
Profiles::GetMailServers()
{
  return m_servers;
}

inline XString
Profiles::GetProfileMap()
{
  return m_profileMap;
}

inline int
Profiles::GetHighestProfileNumber()
{
  return (int) m_profiles.size();
}

inline XString
Profiles::GetCommonOutbox()
{
  return m_sharedOutbox;
}

inline XString
Profiles::GetDefaultFont()
{
  return m_defaultFont;
}

inline void
Profiles::SetDefaultFont(const XString& p_defaultFont)
{
  m_defaultFont = p_defaultFont;
  SetChanged();
}

inline XString
Profiles::GetCentralLogfile()
{
  return m_centralLogfile;
}

inline bool
Profiles::GetHasCentralLogfile()
{
  return !m_centralLogfile.IsEmpty();
}

inline bool
Profiles::GetCanModifyOutbox()
{
  return m_canChangeOutbox;
}


inline void
Profiles::SetTypeOfOutbox(BoxType p_type)
{
  m_outboxType = p_type;
  SetChanged();
}

inline void
Profiles::SetOutboxSelection(DateSel p_selection)
{
  m_outboxSelection = p_selection;
  SetChanged();
}

inline void
Profiles::SetOutboxDirectory(const XString& p_directory)
{
  m_outboxDirectory = p_directory;
  SetChanged();
}

inline BoxType
Profiles::GetOutboxType()
{
  return m_outboxType;
}

inline DateSel
Profiles::GetOutboxSelection()
{
  return m_outboxSelection;
}

inline XString
Profiles::GetOutboxDirectory()
{
  return m_outboxDirectory;
}

inline int
Profiles::GetOutboxMAX()
{
  return m_outboxMAX;
}

inline BoxHistory& 
Profiles::GetOutboxHistory()
{
  return m_outboxHistory;
}

inline bool
Profiles::GetWriteAccess()
{
  return m_writeaccess;
}

inline void 
Profiles::SetCommonOutbox(const XString& p_directory)
{
  m_sharedOutbox = p_directory;
}
