/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: ADSI.cpp
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
#include "ADSI.h"
#include <StringUtilities.h>

#define SECURITY_WIN32
#include <sspi.h>
#include <secext.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static XString organization;

XString GetOrganization()
{
  return organization;
}

XString GetLoginName()
{
  TCHAR name[MAX_PATH + 1];
  unsigned long lengte = MAX_PATH;
  GetUserNameEx(NameSamCompatible,name,&lengte);

  return XString(name);
}

XString GetSender()
{
  // The User Principal Name (UPN) corresponds to the MS Entra ID
  // (formerly Azure AD) email address for domain-joined or
  // Azure AD-joined users.
  TCHAR upn[MAX_PATH + 1];
  unsigned long length = MAX_PATH;
  if(GetUserNameEx(NameUserPrincipal,upn,&length) == FALSE)
  {
    return _T("");
  }

  // Find the organization
  XString mail(upn);
  organization.Empty();
  int pos = mail.Find('@');
  if(pos > 0)
  {
    organization = mail.Mid(pos + 1);
  }
  return mail.c_str();

}
