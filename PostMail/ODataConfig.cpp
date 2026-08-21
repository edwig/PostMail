/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ODataConfig.cpp
//
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#include "stdafx.h"
#include "OdataConfig.h"
#include "PostMail.h"
#include "Version.h"
#include <JSONMessage.h>
#include <StdException.h>
#include <Crypto.h>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// {
// 	"type"             : "Office365"
// 	"token-server"     : ""
//  "azure-tenant"     : "xxxxxxx",
// 	"app-scope"        : "xxxxxxx",
// 	"app-identity"     : "xxxxxxx",
// 	"app-secret"       : "xxxxxxx",
//  "app-browser"      : "msg" / "browser"
// 	"robot" : {
// 				      "uses-account" : "noreply@organization.com"
// 				      "uses-alias"   : "General No Reply"
// 	          }
// }

bool
ODataConfig::ReadConfig()
{
  XString filename = theApp.GetProfiles().GetExePath() + _T("PostMail.json");

  JSONMessage json;
  json.LoadFile(filename);
  if(json.GetErrorState())
  {
    XString error(_T("The OData configuration file ccontains a JSON error: "));
    error += json.GetLastError();
    theApp.Log(LOGLEVEL_ERROR,error);
    return false;
  }

  try
  {
    JSONvalue& val = json.GetValue();
    if(val.GetDataType() == JsonType::JDT_object)
    {
      for(auto& pair : val.GetObject())
      {
        if(pair.m_value.GetDataType() == JsonType::JDT_string)
        {
          AddParameter(pair.m_name,pair.m_value.GetString());
        }
        else if((pair.m_value.GetDataType() == JsonType::JDT_object) &&
                (pair.m_name.CompareNoCase(_T("Robot")) == 0))
        {
          for(auto& robo : pair.m_value.GetObject())
          {
            AddParameter(robo.m_name,robo.m_value.GetString());
          }
        }
        else
        {
          throw StdException(_T("Incorrect JSON: No valid structure"));
        }
      }
    }
    else
    {
      throw StdException(_T("No root JSON object present."));
    }
    DecryptFields();
  }
  catch(StdException& ex)
  {
    XString foutmelding(_T("ERROR in Odata configuration file: "));
    foutmelding += ex.GetErrorMessage();
    theApp.Log(LOGLEVEL_ERROR,foutmelding);
    return false;
  }
  m_readFromFile = true;
  return true;
}

//////////////////////////////////////////////////////////////////////////
//
// PRIVATE
//
//////////////////////////////////////////////////////////////////////////

void 
ODataConfig::AddParameter(const XString& p_name,const XString& p_value)
{
       if(p_name.CompareNoCase(_T("type"))             == 0) m_type             = p_value;
  else if(p_name.CompareNoCase(_T("token-server"))     == 0) m_tokenServer      = p_value;
  else if(p_name.CompareNoCase(_T("azure-tenant"))     == 0) m_azureTenant      = p_value;
  else if(p_name.CompareNoCase(_T("app-identity"))     == 0) m_appIdentity      = p_value;
  else if(p_name.CompareNoCase(_T("app-secret"))       == 0) m_appSecret        = p_value;
  else if(p_name.CompareNoCase(_T("app-scope"))        == 0) m_appScope         = p_value;
  else if(p_name.CompareNoCase(_T("app-browser"))      == 0) m_appBrowser       = p_value;
  else if(p_name.CompareNoCase(_T("uses-account"))     == 0) m_robotUsesAccount = p_value;
  else if(p_name.CompareNoCase(_T("uses-alias"))       == 0) m_robotUsesAlias   = p_value;
  else
  {
    throw StdException(_T("Unknown parameter: ") + p_name);
  }
}

void
ODataConfig::DecryptFields()
{
  int pos = 0;
  Crypto crypt;
  XString passPhrase(POSTMAIL_PASSWORD);

  XString tenant   = crypt.Decryption(m_azureTenant,passPhrase);
  XString identity = crypt.Decryption(m_appIdentity,passPhrase);
  XString secret   = crypt.Decryption(m_appSecret,  passPhrase);

  pos = tenant.Find(_T(':'));
  if(pos > 0)
  {
    m_azureTenant = tenant.Mid(pos + 1);
  }
  pos = identity.Find(_T(':'));
  if(pos > 0)
  {
    m_appIdentity = identity.Mid(pos + 1);
  }
  pos = secret.Find(_T(':'));
  if(pos > 0)
  {
    m_appSecret = secret.Mid(pos + 1);
  }
}
