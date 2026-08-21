/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Profiles.cpp
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
#include "StdAfx.h"
#include "Profiles.h"
#include "PostMail.h"
#include "Message.h"
#include <Crypto.h>
#include <WinFile.h>
#include <XMLMessage.h>
#include <Shlwapi.h>
#include <direct.h>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 6284)

// Language of the current instance
extern XString GetSender();
extern XString GetOrganization();
extern XString GetLoginName();

Profiles::Profiles()
{
  // Defaults
  m_chosen            = -1;
  m_changed           = false;
  m_writeaccess       = false;
  m_canChangeOutbox   = true;
  m_outboxType        = OBT_SHARED;
  m_outboxSelection   = DS_WEEK;
  m_outboxMAX         = MAX_OUTBOX;
}

Profiles::~Profiles()
{
  WriteProfiles();
}

void
Profiles::SetOutboxMAX(int p_maxfiles)
{
  if(p_maxfiles > MAX_OUTBOX && p_maxfiles < MAX_NTFS_FILES)
  {
    m_outboxMAX = p_maxfiles;
  }
  else
  {
    m_outboxMAX = MAX_OUTBOX;
  }
}

// Commando's 
bool 
Profiles::ReadProfiles()
{
  XString fileName;
  int     number = 0;

  // Read the default profiles from the current program directory
  fileName = GetExePath() + DEFAULT_PROFILES;
  CheckDefaults(fileName);
  number = ReadProfilesFile(fileName, false);

  // Read the 'roaming' profiles of the user
  CreateProfileDirectory();
  fileName = m_profileMap + _T("Profiles.xml");
  number += ReadProfilesFile(fileName,true);

  // Check that we have an outbox directory
  if(!m_sharedOutbox.IsEmpty())
  {
    CreateProfileMap(m_sharedOutbox);
  }

  // No file, empty file or not read/write accessible
  if(!number)
  {
    number = CreateProfilesInMemory();
  }
  return (number > 0);
}

XString
Profiles::ReadSetting(XMLMessage* p_doc,XMLElement* p_node,const XString& p_name,const XString& p_default /*=""*/)
{
  XString setting;
  XMLElement* xnaam = p_doc->FindElement(p_node,p_name,false);
  if(xnaam)
  {
    setting = xnaam->GetValue();
    return setting;
  }
  return p_default;
}

bool 
Profiles::WriteSetting(XMLMessage* p_doc,XMLElement* p_node,const XString& p_name,const XString& p_value,const XString& p_before)
{
  XMLElement* naam = p_doc->FindElement(p_node,p_name);
  if(naam)
  {
    // Remove the previous value
    naam->SetValue(p_value);
  }
  else
  {
    bool front = false;
    XMLElement* before = nullptr;
    if (!p_before.IsEmpty())
    {
      before = p_doc->FindElement(p_node,p_before);
      front = true;
    }
    naam = p_doc->AddElement(p_node,p_name,p_value,XmlDataType::XDT_String,front);
  }
  return true;
}

int
Profiles::ReadProfilesFile(const XString& p_fileName,bool p_personal)
{
  int total = GetHighestProfileNumber();
  bool writeaccess = false;

  // Check write access on the config file
  if(_taccess(p_fileName,2) == 0)
  {
    writeaccess = true;
    if(!p_personal)
    {
      // If we have write access to the profiles config file in the program directory
      // we are able to save the outbox and central logfile settings
      m_writeaccess = true;
    }
  }
  // Check read access. If not: do not do anything
  if(_taccess(p_fileName,4) != 0)
  {
    return 0;
  }

  // Load the file as an XML document
  XMLMessage doc;
  doc.LoadFile(p_fileName);
  XMLElement* root = doc.GetRoot();

  XString config = root ? root->GetName() : _T("");
  if(root == nullptr || 
     config.Compare(_T("PostMailProfiles")) ||
     doc.GetInternalError() != XmlError::XE_NoError)
  {
    // Not our config
    return 0;
  }

  // Read the personal profile settings
  if(p_personal)
  {
    m_outboxType      = (BoxType) _ttoi(ReadSetting(&doc,root,_T("OutBoxType"),     _T("1")));
    m_outboxSelection = (DateSel) _ttoi(ReadSetting(&doc,root,_T("OutBoxSelection"),_T("1")));
    m_outboxMAX       =           _ttoi(ReadSetting(&doc,root,_T("OutBoxMaxFiles")));
    m_outboxDirectory =                 ReadSetting(&doc,root,_T("OutBoxDirectory"));
    m_outboxMAX       =           _ttoi(ReadSetting(&doc,root,_T("OutBoxMaxFiles"), _T("2000")));

    for(int ind = 1; ind <= MAX_OUTBOX_HISTORY; ++ind)
    {
      XString hist;
      hist.Format(_T("OutBoxHistory%d"),ind);
      XString directory = ReadSetting(&doc,root,hist);
      if(!directory.IsEmpty())
      {
        AddOutboxHistory(directory);
      }
    }
    SetOutboxMAX(m_outboxMAX);
    if(m_outboxType == OBT_SHARED)
    {
      m_outboxDirectory = m_sharedOutbox;
    }
  }

  XMLElement* node = doc.GetElementFirstChild(root);
  while(node)
  {
    Profile prf;
    XString txt;

    // Default port = 25
    prf.m_smtpPort = DEFAULT_SMTP_PORT;

    // Remember the personal profile
    prf.m_private = p_personal;
    prf.m_mutable = p_personal || writeaccess;

    XString param = node->GetName();
    if(param == _T("Profile"))
    {
      // Keep track of the number of profiles
      prf.m_number = ++total;

      prf.m_profileName  =       ReadSetting(&doc,node,_T("Name"));
      prf.m_smtpServer   =       ReadSetting(&doc,node,_T("Mailhost"));
      prf.m_smtpPort     = _ttoi(ReadSetting(&doc,node,_T("Mailport"),_T("25")));
      prf.m_emailAddress =       ReadSetting(&doc,node,_T("Sender"));
      prf.m_bccAddress   =       ReadSetting(&doc,node,_T("BCC"));
      prf.m_isStandard   =       ReadSetting(&doc,node,_T("IsDefault")).CompareNoCase(_T("true")) == 0;
      prf.m_useLogin     =       ReadSetting(&doc,node,_T("UseLogin")) .CompareNoCase(_T("true")) == 0;
      prf.m_loginUser    =       ReadSetting(&doc,node,_T("LoginUser"));
      prf.m_password     = DecryptPassword(prf.m_loginUser,ReadSetting(&doc,node,_T("Password")));
      prf.m_smtpServer   = StripPortnumber(prf.m_smtpServer);

      // Check personal profile
      CheckDefaults(prf);

      m_profiles.push_back(prf);
    }
    if(param == _T("SharedOutbox"))
    {
      XString outbox = node->GetValue();
      if(!outbox.IsEmpty() && !p_personal)
      {
        // The organization has a shared outbox
        // So it cannot be altered by the current user
        m_canChangeOutbox = false;
        m_sharedOutbox    = outbox;
        m_outboxType      = OBT_SHARED;
      }
    }
    if(param == _T("DefaultFont"))
    {
      m_defaultFont = node->GetValue();
    }
    if(param == _T("CentralLogfile"))
    {
      // Default central logfile 
      XString logfile = node->GetValue();
      if(!logfile.IsEmpty() && !p_personal)
      {
        m_centralLogfile = logfile;
      }
    }
    // Next node
    node = doc.GetElementSibling(node);
  }
  return total;
}

bool 
Profiles::WriteProfiles()
{
  if(m_changed == false)
  {
    return true;
  }
  CreateProfileDirectory();
  XString fileName = m_profileMap + _T("Profiles.xml");


  int count = 0;
  XMLMessage doc;
  XMLElement* root = doc.GetRoot();

  doc.SetRootNodeName(_T("PostMailProfiles"));
  doc.AddElement(root,_T("DefaultFont"),m_defaultFont);
  // Save the outbox first (one time only)
  if(m_outboxType == OBT_SHARED)
  {
    m_outboxDirectory = m_sharedOutbox;
  }

  XString outType;
  XString outSelect;
  XString outMax;
  outType  .Format(_T("%d"),m_outboxType);
  outSelect.Format(_T("%d"),m_outboxSelection);
  outMax.   Format(_T("%d"),m_outboxMAX);

  doc.AddElement(root,_T("OutBoxType"),     outType);
  doc.AddElement(root,_T("OutBoxSelection"),outSelect);
  doc.AddElement(root,_T("OutBoxDirectory"),m_outboxDirectory);
  doc.AddElement(root,_T("OutBoxMaxFiles"), outMax);

  // Append last history
  int index = 1;
  for (auto& directory : m_outboxHistory)
  {
    XString node;
    node.Format(_T("OutBoxHistory%d"),index);
    doc.AddElement(root,node,directory);
    ++index;
  }

  for(unsigned ind = 0;ind < m_profiles.size(); ++ind)
  {
    // Personal profiles are saved in their own directory
    // under %APPDATA%/Roaming/<Company>/Profiles.xml
    if(m_profiles[ind].m_private == false)
    {
      continue;
    }

    // Save
    ++count;

    // Create encrypted password
    XString password = m_profiles[ind].m_password;
    if(!password.IsEmpty())
    {
      password = EncryptPassword(m_profiles[ind].m_loginUser,password);
    }

    XString port;
    port.Format(_T("%d"),m_profiles[ind].m_smtpPort);

    XMLElement* profile = doc.AddElement(root,_T("Profile"));

    doc.AddElement(profile,_T("Name"),     m_profiles[ind].m_profileName);
    doc.AddElement(profile,_T("Mailhost"), m_profiles[ind].m_smtpServer);
    doc.AddElement(profile,_T("Mailport"), port);
    doc.AddElement(profile,_T("Sender"),   m_profiles[ind].m_emailAddress);
    doc.AddElement(profile,_T("BCC"),      m_profiles[ind].m_bccAddress);
    doc.AddElement(profile,_T("IsDefault"),m_profiles[ind].m_isStandard  ? _T("true") : _T("false"));
    doc.AddElement(profile,_T("UseLogin"), m_profiles[ind].m_useLogin ? _T("true") : _T("false"));
    doc.AddElement(profile,_T("LoginUser"),StripSingleSignOn(m_profiles[ind].m_loginUser));
    doc.AddElement(profile,_T("Password"), password);
  }

  // Write file to disk
  if(doc.SaveFile(fileName))
  {
    XString melding;
    melding.Format(g_message[MESS_PROFSAVED][g_lang],fileName.GetString(),count);
    theApp.WideMessageBox(NULL,melding,WhoAmI,MB_OK);

    m_changed = false;
  }
  else
  {
    XString melding;
    melding.Format(g_message[MESS_PROFSAVEFAIL][g_lang],fileName.GetString());
    theApp.WideMessageBox(NULL,melding,WhoAmI,MB_OK | MB_ICONERROR);
    return false;
  }
  // Did it!
  return true;
}

bool
Profiles::WriteCentralSettings(bool p_font /*= false*/)
{
  // No write access, so we do not write central settings
  if(!m_writeaccess)
  {
    return false;
  }

  XString fileNaam = GetExePath() + DEFAULT_PROFILES;
  XMLMessage doc;
  doc.LoadFile(fileNaam);

  XMLElement* root = doc.GetRoot();
  XString config = root ? root->GetName() : _T("");
  if(root == nullptr || config.Compare(_T("PostMailProfiles")))
  {
    // Not our config
    return false;
  }

  WriteSetting(&doc,root,_T("SharedOutbox"),  m_sharedOutbox,  _T("Profile"));
  WriteSetting(&doc,root,_T("CentralLogfile"),m_centralLogfile,_T("Profile"));
  if(p_font)
  {
    WriteSetting(&doc,root,_T("DefaultFont"), m_defaultFont,_T("Profile"));
  }
  // And save again
  return doc.SaveFile(fileNaam);
}

XString
Profiles::StripSingleSignOn(const XString& p_user)
{
  if(p_user.Find(_T("<Single sign-on>")) >= 0)
  {
    return _T("");
  }
  return p_user;
}

void
Profiles::CheckDefaults(Profile& p_profile)
{
  if(p_profile.m_profileName .Compare(_T("@USER")) == 0 ||
     p_profile.m_emailAddress.Compare(_T("@FROM")) == 0 ||
     p_profile.m_bccAddress  .Compare(_T("@FROM")) == 0  )
  {
    XString theSender    = GetSender();
    XString organization = GetOrganization();

    p_profile.m_profileName  = GetLoginName();
    p_profile.m_emailAddress = theSender;
    p_profile.m_bccAddress   = theSender;

    // Profile cannot be changed in the program
    // Otherwise the @USER/@FROM macro's would be lost!
    p_profile.m_mutable = false;
  }

  // Profile cannot be changed
  if(p_profile.m_profileName.Compare(_T("Default")) == 0)
  {
    p_profile.m_mutable = false;
  }
}

void
Profiles::CheckDefaults(const XString& p_fileName)
{
  // Check if the file already exists
  if(_taccess(p_fileName,4) == 0)
  {
    return;
  }
  XString mailhost;
  XString afzender    = GetSender();
  XString organisatie = GetOrganization();
  XString noreply;
  noreply.Format(_T("noreply@%s"),organisatie.GetString());
  
  if(!m_servers.empty())
  {
    mailhost = m_servers.front();
  }

  XMLMessage doc;
  doc.SetRootNodeName(_T("PostMailProfiles"));
  XMLElement* root = doc.GetRoot();

  doc.AddElement(root,_T("SharedOutbox"));

  XMLElement* profile = doc.AddElement(root,_T("Profile"));

  // Default profile
  doc.AddElement(profile,_T("Name"),         _T("Default"));
  doc.AddElement(profile,_T("Mailhost"),     mailhost);
  doc.AddElement(profile,_T("Sender"),       noreply);
  doc.AddElement(profile,_T("IsDefault"),    _T("true"));
  doc.AddElement(profile,_T("UseLogin"),     _T("false"));
  doc.AddElement(profile,_T("LoginUser"));
  doc.AddElement(profile,_T("LoginPassword"));

  profile = doc.AddElement(root,_T("Profile"));

  // User profile
  doc.AddElement(profile,_T("Name"),         _T("@USER"));
  doc.AddElement(profile,_T("Mailhost"),     mailhost);
  doc.AddElement(profile,_T("Sender"),       _T("@FROM"));
  doc.AddElement(profile,_T("BCC"),          _T("@FROM"));
  doc.AddElement(profile,_T("IsDefault"),    _T("false"));
  doc.AddElement(profile,_T("UseLogin"),     _T("false"));
  doc.AddElement(profile,_T("LoginUser"));
  doc.AddElement(profile,_T("LoginPassword"));

  if(!doc.SaveFile(p_fileName))
  {
    // Cannot write a standard profiles file in the installation directory of PostMail
    XString mess = g_message[MESS_INST_DEFAULTS][g_lang];
    theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
  }
}

// Files NOT succeeded. Create in memory
int
Profiles::CreateProfilesInMemory()
{
  Profile defProfile;
  Profile usrProfile;
  
  XString mailhost;
  XString theSender    = GetSender();
  XString organization = GetOrganization();
  
  if(!m_servers.empty())
  {
    mailhost = m_servers.front();
  }
  
  // User profile
  usrProfile.m_number       = 1;
  usrProfile.m_private      = false;
  usrProfile.m_profileName  = GetLoginName();
  usrProfile.m_emailAddress = theSender;
  usrProfile.m_bccAddress   = theSender;
  usrProfile.m_smtpServer   = mailhost;
  usrProfile.m_smtpPort     = DEFAULT_SMTP_PORT;
  usrProfile.m_isStandard   = false;
  usrProfile.m_useLogin     = false;
  // Default Profile
  defProfile.m_number       = 2;
  defProfile.m_private      = false;
  defProfile.m_profileName  = _T("Default");
  defProfile.m_emailAddress = _T("noreply@") + organization;
  defProfile.m_smtpServer   = mailhost;
  defProfile.m_smtpPort     = DEFAULT_SMTP_PORT;
  defProfile.m_isStandard   = true;
  defProfile.m_useLogin     = false;
  
  m_profiles.push_back(usrProfile);
  m_profiles.push_back(defProfile);
  
  return (int)m_profiles.size();
}

bool 
Profiles::AddProfile(Profile* p_profile)
{
  for(unsigned int ind = 0;ind < m_profiles.size(); ++ind)
  {
    if(m_profiles[ind].m_profileName.CompareNoCase(p_profile->m_profileName) == 0)
    {
      return false;
    }
  }
  p_profile->m_number = (int)m_profiles.size() + 1;
  m_profiles.push_back(*p_profile);
  m_changed = true;
  return true;
}

void      
Profiles::RemoveProfile(const XString& p_name)
{
  ProfileMap::iterator it = m_profiles.begin();
  while(it != m_profiles.end())
  {
    if(it->m_profileName.CompareNoCase(p_name) == 0)
    {
      m_changed = true;
      m_profiles.erase(it);
      break;
    }
    // Next profile
    ++it;
  }
  // Renumber the profiles
  for(unsigned int ind = 0;ind < m_profiles.size(); ++ind)
  {
    m_profiles[ind].m_number = ind + 1;
  }
}

// Getters
Profile* 
Profiles::GetStandardProfile()
{
  for(unsigned int ind = 0;ind < m_profiles.size(); ++ind)
  {
    if(m_profiles[ind].m_isStandard)
    {
      return &(m_profiles[ind]);
    }
  }
  return NULL;
}

bool      
Profiles::SetChosenProfile(int p_index)
{
  if(p_index > 0 && p_index <= (int)m_profiles.size())
  {
    m_chosen = p_index;
    AfxGetApp()->WriteProfileInt(_T(""),_T("LastProfile"),m_chosen);
    return true;
  }
  return false;
}

bool
Profiles::SetChosenProfile(const XString& p_name)
{
  for(unsigned int ind = 0;ind < m_profiles.size(); ++ind)
  {
    if(m_profiles[ind].m_profileName.CompareNoCase(p_name) == 0)
    {
      m_chosen = m_profiles[ind].m_number;
      AfxGetApp()->WriteProfileInt(_T(""),_T("LastProfile"),m_chosen);
      return true;
    }
  }
  return false;
}

Profile* 
Profiles::GetProfile(int p_index)
{
  if(p_index > 0 && p_index <= (int)m_profiles.size())
  {
    return &(m_profiles[p_index - 1]);
  }
  return NULL;
}

Profile* 
Profiles::GetProfile(const XString& p_name)
{
  for(unsigned int ind = 0;ind < m_profiles.size(); ++ind)
  {
    if(m_profiles[ind].m_profileName.CompareNoCase(p_name) == 0)
    {
      return &(m_profiles[ind]);
    }
  }
  return NULL;
}

// TRUE if there is exactly one (1) email server and email address
bool
Profiles::HasACompleteProfile()
{
  if(m_profiles.size() == 1)
  {
    Profile& profile = m_profiles[0];
    if(!profile.m_smtpServer.IsEmpty() &&
       !profile.m_emailAddress.IsEmpty() )
    {
      return true;
    }
  }
  return false;
}

void
Profiles::SetCentralLogfile(const XString& p_logfile)
{
  if(m_centralLogfile.CompareNoCase(p_logfile))
  {
    m_centralLogfile = p_logfile;
  }
}

// Privates

void 
Profiles::FindAppData(int p_directory)
{
  IMalloc*      pShellMalloc = NULL;    // A pointer to the shell's IMalloc interface
  IShellFolder* psfParent;              // A pointer to the parent folder object's IShellFolder interface.
  LPITEMIDLIST  pidlItem     = NULL;    // The item's PIDL.
  LPITEMIDLIST  pidlRelative = NULL;    // The item's PIDL relative to the parent folder.
  TCHAR         szPath[MAX_PATH];       // The path for Favorites.
  STRRET        str;                    // The structure for strings returned from IShellFolder.

  szPath[0] = 0;
  HRESULT hres = SHGetMalloc(&pShellMalloc);
  if (FAILED(hres))
  {
    return;
  }
  hres = SHGetSpecialFolderLocation(NULL,p_directory,&pidlItem);
  if (SUCCEEDED(hres))
  {
    hres = SHBindToParent(pidlItem, IID_IShellFolder, (void**)&psfParent, (LPCITEMIDLIST*)&pidlRelative);
    if (SUCCEEDED(hres))
    {
      // Retrieve the path
      memset(&str, 0, sizeof(str));
      hres = psfParent->GetDisplayNameOf(pidlRelative, SHGDN_NORMAL | SHGDN_FORPARSING, &str);
      if (SUCCEEDED(hres))
      {
        if (StrRetToBuf(&str, pidlItem, szPath, ARRAYSIZE(szPath)) != S_OK)
        {
          szPath[0] = 0;
        }
      }
      psfParent->Release();
    }
    // Clean up allocated memory
    if (pidlItem)
    {
      pShellMalloc->Free(pidlItem);
    }
  }
  pShellMalloc->Release();

  m_appData = szPath;
}

bool
Profiles::CreateProfileMap(const XString& p_name)
{
  if(CreateDirectory(p_name,NULL) == FALSE)
  {
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
      return true;
    }
    WinFile ensure(p_name + _T("\\name.txt"));
    if(ensure.CreateDirectory())
    {
      XString melding;
      melding.Format(g_message[MESS_WINDIRFAIL][g_lang],p_name.GetString(),SystemError().GetString());
      theApp.WideMessageBox(NULL,melding,WhoAmI,MB_OK|MB_ICONERROR);
      return false;
    }
  }
  return true;
}

bool
Profiles::CreateProfileDirectory()
{
  XString pad;

  // CSIDL_APPDATA + \\<COMPANY>
  FindAppData(CSIDL_APPDATA);
  pad = m_appData;

  // See to it we have a target
  if(CreateProfileMap(pad) == false)
  {
    return false;
  }
  // Company directory creation
  pad += _T("\\") COMPANY;
  if(CreateProfileMap(pad) == false)
  {
    return false;
  }
  // CSIDL_APPDATA + \\<COMPANY>\\ + m_applicName
  pad += _T("\\") APPLICATION;
  if(CreateProfileMap(pad) == false)
  {
    return false;
  }
  m_profileMap = pad;
  m_profileMap += _T("\\");
  return true;
}

XString   
Profiles::EncryptPassword(const XString& p_key,const XString& p_string)
{
  // Create a password key of at least 8 chars in size
  XString key = p_key + EXPAND_KEY;
  // Create string to embed in
  XString string = key + _T("19591015:Postmail:") + p_string + _T(":XXXXXXX");

  // Encrypt
  Crypto crypt;
  return crypt.Encryption(string,key);
}

XString   
Profiles::DecryptPassword(const XString& p_key,const XString& p_string)
{
  if(p_string.IsEmpty())
  {
    return p_string;
  }

  // Create a password key at least 8 chars in size
  XString key = p_key + EXPAND_KEY;

  // Decrypt
  Crypto crypto;
  XString result = crypto.Decryption(p_string,key);

  // Retrieve password from the result string
  int pos = result.Find(_T(":Postmail:"));
  if(pos > 0)
  {
    pos += 10;
    result = result.Mid(pos);
    pos = result.Find(_T(":XX"));
    if(pos > 0)
    {
      result = result.Left(pos);
    }
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
//
// MAIL SERVERS
//
//

XString
Profiles::GetExePath()
{
  TCHAR buffer[_MAX_PATH + 1];
  XString applicationAndPath;

  if(GetModuleFileName(GetModuleHandle(NULL), buffer, _MAX_PATH) > 0)
  {
    applicationAndPath = buffer;
  }
  int slashPositie = applicationAndPath.ReverseFind(_T('\\'));
  if (slashPositie == 0)
  {
    return _T("");
  }
  return applicationAndPath.Left(slashPositie + 1);
}

void
Profiles::ReadMailServers()
{
  XString filename = GetExePath() + _T("PostMail.Mailservers.ini");
  WinFile servers;

  // Try to open file
  servers.SetFilename(filename);
  servers.Open(winfile_read,FAttributes::attrib_none,Encoding::UTF8);

  // If not opened
  if(servers.GetIsOpen() == false)
  {
    // Try to write as default
    servers.Open(winfile_write,FAttributes::attrib_none,Encoding::UTF8);
    if(servers.GetIsOpen())
    {
      servers.Write(_T("# Place email servers in Postmail.Mailservers.ini\n"));
      servers.Write(_T("# One per line\n"));
      servers.Close();
    }
    // Opened now?
    servers.Open(winfile_read,FAttributes::attrib_none,Encoding::UTF8);
  }

  // If found, read it
  if(servers.GetIsOpen())
  {
    XString buffer;
    while(servers.Read(buffer))
    {
      XString server = buffer;
      server.Replace(_T("\n"),_T(""));
      server.Replace(_T("\r"),_T(""));
      server = server.Trim();
      // Remove comment and retain servers
      if(server.GetAt(0) != _T(';') && 
         server.GetAt(0) != _T('#') &&
         server.GetLength() > 1)
      {
        // Remove old port settings
        server = StripPortnumber(server);
        m_servers.push_back(server);
      }
    }
    servers.Close();
  }

  // Check that we have at least 1 server
  if(m_servers.empty())
  {
    // No standard email servers found in 'Postmail.Mailservers.ini' in the installation directory of PostMail
    XString mess = g_message[MESS_INST_SERVERS][g_lang];
    theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
  }
}

void
Profiles::CheckMailServer(XString& p_server)
{
  if(m_servers.size() == 0)
  {
    // No check to perform
    return;
  }
  XString logline;

  // If no server found yet
  if(p_server.IsEmpty())
  {
    if(!m_servers.empty())
    {
      // No server set, use first of the servers list
      p_server = m_servers.front();
      // "No server set yet. Using the first SMTP server: %s"
      logline.Format(g_message[MESS_USE1SERVER][g_lang],p_server.GetString());
      theApp.Log(1,logline);
    }
    return;
  }
  // Check mail server against all known mailservers
  MailServers::iterator it;
  for(it = m_servers.begin();it != m_servers.end(); ++it)
  {
    XString server = *it;
    if(p_server.CompareNoCase(server) == 0)
    {
      // Server is in the allowed list
      return;
    }
  }
  // "SMTP Server [%s] is ***NOT*** in the list of configured servers"
  logline.Format(g_message[MESS_SERVERNOCONF][g_lang],p_server.GetString());
  theApp.Log(1,logline);
}

// Return default server (if any)
XString
Profiles::GetDefaultMailServer()
{
  XString defServer;

  if(m_servers.size())
  {
    defServer = m_servers.front();
  }
  return defServer;
}

// Upgrade profiles, if possible
void
Profiles::ProfileUpgrade(int p_profile)
{
  // Already the highest profile
  if(p_profile == 0)
  {
    return;
  }
  SwapProfiles(p_profile-1,p_profile);
}

void
Profiles::ProfileDowngrade(int p_profile)
{
  // Already the lowest profile?
  if(p_profile == m_profiles.size() -1)
  {
    return;
  }
  SwapProfiles(p_profile,p_profile+1);
}

void
Profiles::ResetOutboxHistory()
{
  m_outboxHistory.clear();
}

void
Profiles::AddOutboxHistory(const XString& p_directory)
{
  if(m_outboxHistory.size() < MAX_OUTBOX_HISTORY)
  {
    m_outboxHistory.push_back(p_directory);
  }
}

// Swap two profiles
// ONLY TO BE CALLED AFTER CHECKING ALL PARAMETERS!!
void
Profiles::SwapProfiles(int p_one,int p_two)
{
  std::swap(m_profiles[p_one].m_number,      m_profiles[p_two].m_number);
  std::swap(m_profiles[p_one].m_private,     m_profiles[p_two].m_private);
  std::swap(m_profiles[p_one].m_mutable,     m_profiles[p_two].m_mutable);
  std::swap(m_profiles[p_one].m_profileName, m_profiles[p_two].m_profileName);
  std::swap(m_profiles[p_one].m_emailAddress,m_profiles[p_two].m_emailAddress);
  std::swap(m_profiles[p_one].m_bccAddress,  m_profiles[p_two].m_bccAddress);
  std::swap(m_profiles[p_one].m_smtpServer,  m_profiles[p_two].m_smtpServer);
  std::swap(m_profiles[p_one].m_smtpPort,    m_profiles[p_two].m_smtpPort);
  std::swap(m_profiles[p_one].m_isStandard,  m_profiles[p_two].m_isStandard);
  std::swap(m_profiles[p_one].m_useLogin,    m_profiles[p_two].m_useLogin);
  std::swap(m_profiles[p_one].m_loginUser,   m_profiles[p_two].m_loginUser);
  std::swap(m_profiles[p_one].m_password,    m_profiles[p_two].m_password);
}

XString
Profiles::StripPortnumber(const XString& p_server)
{
  int pos = p_server.Find(_T(':'));
  if (pos > 0)
  {
    return p_server.Left(pos);
  }
  return p_server;
}
