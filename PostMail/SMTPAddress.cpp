/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPAddress.cpp
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
#include "SMTPAddress.h"
#include "PostMail.h"
#include "Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 6284)

SMTPAddress::SMTPAddress() 
{
}

SMTPAddress::SMTPAddress(const SMTPAddress& address)
{
  *this = address;
}

SMTPAddress::SMTPAddress(const XString& sAddress)
            :m_emailAddress(sAddress) 
{
  if(!m_emailAddress.GetLength())
  {
    // MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(3,g_message[MESS_NOADDRESS][g_lang]);
  }
}

SMTPAddress::SMTPAddress(const XString& sFriendly, const XString& sAddress) 
            :m_friendlyName(sFriendly)
            ,m_emailAddress(sAddress) 
{
  if(!m_emailAddress.GetLength())
  {
    // MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(3,g_message[MESS_NOADDRESS][g_lang]);
  }
}

SMTPAddress& 
SMTPAddress::operator=(const SMTPAddress& r) 
{ 
  // Check if we are not assigning ourselves
  if(&r == this)
  {
    return *this;
  }
  // Do the copy
  m_friendlyName = r.m_friendlyName; 
  m_emailAddress = r.m_emailAddress; 
  return *this;
}

XString 
SMTPAddress::GetRegularFormat(bool p_emtpyIsError /*= true*/) const
{
  if(!m_emailAddress.GetLength() && p_emtpyIsError)
  {
    // MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(3,g_message[MESS_NOADDRESS][g_lang]);
  }
  XString sAddress;
  if (m_friendlyName.IsEmpty())
  {
    sAddress = m_emailAddress;  //Just transfer the address across directly
  }
  else
  {
    sAddress.Format(_T("%s <%s>")
                   ,m_friendlyName.GetString()
                   ,m_emailAddress.GetString());
  }
  return sAddress;
}

XString
SMTPAddress::GetOriginalRecepientsFormat() const
{
  if(!m_emailAddress.GetLength())
  {
    // MESS_NOADDRESS An empty e-mail address is not allowed
    theApp.Log(3,g_message[MESS_NOADDRESS][g_lang]);
  }
  XString sAddress(_T("rfc822;"));
  sAddress += m_emailAddress;
  return sAddress;
}
