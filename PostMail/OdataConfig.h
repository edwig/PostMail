/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ODataConfig.h
//
// Written by W.E. Huisman (2006-2026)
// MIT License
//
#pragma once

class ODataConfig
{
public:
  ODataConfig() = default;

  // Read PostMail.json
  bool    ReadConfig();

  bool    GetHasBeenRead()       { return m_readFromFile;        }
  XString GetType()              { return m_type;                }
  XString GetTokenServer()       { return m_tokenServer;         }
  XString GetAzureTenant()       { return m_azureTenant;         }
  XString GetAppIdentity()       { return m_appIdentity;         }
  XString GetAppSecret()         { return m_appSecret;           }
  XString GetAppScope()          { return m_appScope;            }
  XString GetAppBrowser()        { return m_appBrowser;          }
  XString GetRobotUsesAccount()  { return m_robotUsesAccount;    }
  XString GetRobotUsesAlias()    { return m_robotUsesAlias;      }

private:
  void AddParameter(const XString& p_name,const XString& p_value);
  void DecryptFields();

  bool    m_readFromFile { false };
  // All parameters
  XString m_type;
  XString m_tokenServer;
  XString m_azureTenant;
  XString m_appIdentity;
  XString m_appSecret;
  XString m_appScope;
  XString m_appBrowser;
  XString m_robotUsesAccount;
  XString m_robotUsesAlias;
};
