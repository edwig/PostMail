/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Base64.h
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
#include "PostMail.h"

// Enlarge max-line boundary so long paswords can be sent
#define BASE64_MAXLINE  256
#define EOL  "\r\n"

class Base64
{
public:
  static int  Base64BufferSize(int nInputSize);
  static BOOL EncodeBase64(const BYTE* pszIn
                          ,int         nInLen
                          ,BYTE*       pszOut
                          ,int         nOutSize
                          ,int*        nOutLen);
  static char m_base64tab[];
};


class SMPTBase64Encode
{
public:
  SMPTBase64Encode();
 ~SMPTBase64Encode();

  //Methods
  bool  Encode(const BYTE* pbyData, int nSize, DWORD dwFlags);
  bool	Decode(LPCSTR pData, int nSize);
  bool	Encode(LPCSTR pszMessage, DWORD dwFlags);
  bool	Decode(LPCSTR sMessage);

  LPSTR Result()     const { return m_pBuf;  };
  int	  ResultSize() const { return m_nSize; };

protected:
  char* m_pBuf;
  int   m_nSize;
};

