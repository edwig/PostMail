////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Commands.cpp
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
#include "Commands.h"
#include "Message.h"

static Command commands[] =
{
  // ENGLISH                NEDERLANDS                  FRANCAIS                  DEUTSCH                       DATATYPE
  // ---------------------- --------------------------- ------------------------- ----------------------------- -----------------------
  { _T("host"),             _T("mailserver"),           _T("hote"),               _T("server"),                 PMCType::PMC_SString  }
 ,{ _T("login"),            _T("login"),                _T("connecter"),          _T("anmelden"),               PMCType::PMC_Boolean  }
 ,{ _T("mailid"),           _T("gebruiker"),            _T("utilisateur"),        _T("benutzer"),               PMCType::PMC_SString  }
 ,{ _T("from"),             _T("van"),                  _T("de"),                 _T("von"),                    PMCType::PMC_SString  }
 ,{ _T("to"),               _T("aan"),                  _T("a"),                  _T("zu"),                     PMCType::PMC_MString  }
 ,{ _T("cc"),               _T("cc"),                   _T("cc"),                 _T("cc"),                     PMCType::PMC_MString  }
 ,{ _T("bcc"),              _T("bcc"),                  _T("cci"),                _T("bcc"),                    PMCType::PMC_MString  }
 ,{ _T("subject"),          _T("onderwerp"),            _T("sujet"),              _T("thema"),                  PMCType::PMC_SString  }
 ,{ _T("profile"),          _T("profiel"),              _T("profil"),             _T("profil"),                 PMCType::PMC_SString  }
 ,{ _T("errors"),           _T("fouten"),               _T("erreurs"),            _T("fehler"),                 PMCType::PMC_Boolean  }
 ,{ _T("dialog"),           _T("dialoog"),              _T("dialogue"),           _T("dialog"),                 PMCType::PMC_Boolean  }
 ,{ _T("readconfirmation"), _T("leesbevestiging"),      _T("recudelecture"),      _T("lesebestatigung"),        PMCType::PMC_Boolean  }
 ,{ _T("sentconfirmation"), _T("verstuurdbevestiging"), _T("conformationenvoyee"),_T("gesendetbestatigung"),    PMCType::PMC_Boolean  }
 ,{ _T("deliveredstatus"),  _T("bezorgdstatus"),        _T("statutdelivraison"),  _T("zugestellt"),             PMCType::PMC_Notify   }
 ,{ _T("progress"),         _T("voortgang"),            _T("progres"),            _T("fortschritt"),            PMCType::PMC_Boolean  }
 ,{ _T("delete"),           _T("verwijderen"),          _T("suprimer"),           _T("loeschen"),               PMCType::PMC_Boolean  }
 ,{ _T("editsubject"),      _T("wijzigonderwerp"),      _T("modifiersujet"),      _T("themabearbeiten"),        PMCType::PMC_Boolean  }
 ,{ _T("editbody"),         _T("wijzigbericht"),        _T("modifiermessage"),    _T("nachrichtbearbeiten"),    PMCType::PMC_Boolean  }
 ,{ _T("notify"),           _T("notificeer"),           _T("avis"),               _T("hinweiser"),              PMCType::PMC_Boolean  }
 ,{ _T("writechanges"),     _T("bewaarwijzigingen"),    _T("enregistrer"),        _T("speichern"),              PMCType::PMC_Boolean  }
 ,{ _T("nopwdcrypt"),       _T("geenpwdcrypt"),         _T("pasdepmcrypt"),       _T("keinpwcrypt"),            PMCType::PMC_Present  }
 ,{ _T("password"),         _T("wachtwoord"),           _T("motdepasse"),         _T("passwort"),               PMCType::PMC_SHA256   }
 ,{ _T("attach"),           _T("bijlage"),              _T("appendice"),          _T("anhang"),                 PMCType::PMC_MString  }
 ,{ _T("importance"),       _T("prioriteit"),           _T("priorite"),           _T("prioritat"),              PMCType::PMC_Priority }
 ,{ _T("language"),         _T("taal"),                 _T("langue"),             _T("sprache"),                PMCType::PMC_Language }
 ,{ _T("timeout"),          _T("timeout"),              _T("tempsmort"),          _T("time-out"),               PMCType::PMC_Integer  }
 ,{ _T("loglevel"),         _T("logniveau"),            _T("niveaudejournal"),    _T("loglevel"),               PMCType::PMC_Integer  }
};

static Command deliverstatuses[] =
{
  // ENGLISH        NEDERLANDS       FRANCAIS         DEUTSCH             DATATYPE
  // -------------- ---------------- ---------------- ------------------- --------------------------
  { _T("failure"),  _T("mislukt"),   _T("faux"),      _T("fehler"),       (PMCType) NOTIFY_FAILURE }
 ,{ _T("success"),  _T("bezorgd"),   _T("distribue"), _T("zugestellt"),   (PMCType) NOTIFY_SUCCESS }
 ,{ _T("delayed"),  _T("vertraagd"), _T("retarde"),   _T("verzoegert"),   (PMCType) NOTIFY_DELAY   }
 ,{ _T("never"),    _T("nooit"),     _T("jamais"),    _T("niemals"),      (PMCType) NOTIFY_NEVER   }
 ,{ _T("header"),   _T("koptekst"),  _T("en-tete"),   _T("ueberschrift"), (PMCType) NOTIFY_HEADER  }
 ,{ _T("full"),     _T("geheel"),    _T("entier"),    _T("ganz"),         (PMCType) NOTIFY_FULL    }
};

static Command priorities[] =
{
  // ENGLISH        NEDERLANDS     FRANCAIS       DEUTSCH        DATATYPE
  // -------------- -------------- -------------- -------------- -------------------------------
  { _T("high"),    _T("hoog"),     _T("haut"),    _T("hoch"),    (PMCType) MAILPRIORITY_HIGH   }
 ,{ _T("normal"),  _T("normaal"),  _T("normal"),  _T("normal"),  (PMCType) MAILPRIORITY_NORMAL }
 ,{ _T("low"),     _T("laag"),     _T("bas"),     _T("niedrig"), (PMCType) MAILPRIORITY_LOW    }
};

CommandNR FindMailCommand(XString& p_name,PMCType& p_type)
{
  for(int index = 0;index < (sizeof(commands)/sizeof(Command));++index)
  {
    if((p_name.CompareNoCase(commands[index].m_english) == 0) ||
       (p_name.CompareNoCase(commands[index].m_dutch  ) == 0) ||
       (p_name.CompareNoCase(commands[index].m_french ) == 0) ||
       (p_name.CompareNoCase(commands[index].m_german ) == 0))
    {
      p_type = commands[index].m_type;
      return (CommandNR) index;
    }
  }
  return (CommandNR)-1;
}

int FindDeliverStatus(XString& p_status)
{
  for(int index = 0;index < (sizeof(deliverstatuses)/sizeof(Command));++index)
  {
    if((p_status.CompareNoCase(deliverstatuses[index].m_english) == 0) ||
       (p_status.CompareNoCase(deliverstatuses[index].m_dutch)   == 0) ||
       (p_status.CompareNoCase(deliverstatuses[index].m_french)  == 0) ||
       (p_status.CompareNoCase(deliverstatuses[index].m_german)  == 0))
    {
      return (int)deliverstatuses[index].m_type;
    }
  }
  return -1;
}

int FindMailPriority(XString& p_priority)
{
  for(int index = 0;index < (sizeof(priorities)/sizeof(Command));++index)
  {
    if((p_priority.CompareNoCase(priorities[index].m_english) == 0) ||
       (p_priority.CompareNoCase(priorities[index].m_dutch  ) == 0) ||
       (p_priority.CompareNoCase(priorities[index].m_french ) == 0) ||
       (p_priority.CompareNoCase(priorities[index].m_german ) == 0))
    {
      return (int)priorities[index].m_type;
    }
  }
  return -1;
}

// Returns 1 for true, 0 for false, -1 for not found
int FindMailBoolean(XString& p_value)
{
  if(p_value.CompareNoCase(_T("yes")) == 0 ||
     p_value.CompareNoCase(_T("ja" )) == 0 ||
     p_value.CompareNoCase(_T("oui")) == 0 )
  {
    return 1;
  }
  else if(p_value.CompareNoCase(_T("no"))  == 0 ||
          p_value.CompareNoCase(_T("nee")) == 0 ||
          p_value.CompareNoCase(_T("non")) == 0 ||
          p_value.CompareNoCase(_T("nein"))== 0)
  {
    return 0;
  }
  return -1;
}

int FindMailLanguage(XString& p_language)
{
  if(p_language.CompareNoCase(_T("dutch"))      == 0 ||
     p_language.CompareNoCase(_T("nederlands")) == 0 ||
     p_language.CompareNoCase(_T("nl"))         == 0)
  {
    return LANGUAGE_NEDERLANDS;
  }
  else if(p_language.CompareNoCase(_T("english")) == 0 ||
          p_language.CompareNoCase(_T("engels"))  == 0 ||
          p_language.CompareNoCase(_T("en"))      == 0)
  {
    return LANGUAGE_ENGLISH;
  }
  else if(p_language.CompareNoCase(_T("french"))  == 0 ||
          p_language.CompareNoCase(_T("francais"))== 0 ||
          p_language.CompareNoCase(_T("fr"))      == 0)
  {
    return LANGUAGE_FRANCAIS;
  }
  else if(p_language.CompareNoCase(_T("german"))  == 0 ||
          p_language.CompareNoCase(_T("deutsch")) == 0 ||
          p_language.CompareNoCase(_T("de"))      == 0)
  {
    return LANGUAGE_DEUTSCH;
  }

  // Fallback to first character
  switch(p_language.GetAt(0))
  {
    case _T('N'):
    case _T('n'): return LANGUAGE_NEDERLANDS; break;
    case _T('E'):
    case _T('e'): return LANGUAGE_ENGLISH;    break;
    case _T('F'):
    case _T('f'): return LANGUAGE_FRANCAIS;   break;
    case _T('D'):
    case _T('d'): return LANGUAGE_DEUTSCH;    break;
  }
  return -1;
}

