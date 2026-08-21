/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPAddress.h
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
#pragma once

class SMTPAddress
{
public: 
  // Constructors / Destructors
  SMTPAddress();
  SMTPAddress(const SMTPAddress& address);
	SMTPAddress(const XString& sAddress);
	SMTPAddress(const XString& sFriendly, const XString& sAddress);
	SMTPAddress& operator=(const SMTPAddress& r);

  // Methods
  XString GetRegularFormat(bool p_emtpyIsError = true) const;
  XString GetOriginalRecepientsFormat() const;

  // Data members
	XString  m_friendlyName; // Would set it to contain  something like "Edwig Huisman"
  XString  m_emailAddress; // Would set it to contains something like "weh@organisation.eu"
};
