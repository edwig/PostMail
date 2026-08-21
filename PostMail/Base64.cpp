/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Base64.cpp
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
#include "Base64.h"
#include "Message.h"
#include <atlenc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char Base64::m_base64tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "abcdefghijklmnopqrstuvwxyz0123456789+/";

int 
Base64::Base64BufferSize(int nInputSize)
{
  int nOutSize = (nInputSize+2)/3*4;                    // 3:4 conversion ratio
  nOutSize += (int)strlen(EOL)*nOutSize/BASE64_MAXLINE + 3;  // Space for newlines and NUL
  return nOutSize;
}

BOOL 
Base64::EncodeBase64(const BYTE* pszIn
                    ,int         nInLen
                    ,BYTE*       pszOut
                    ,int         nOutSize
                    ,int*        nOutLen)
{
  //Input Parameter validation
  if(pszIn==NULL || pszOut==NULL || nOutSize==0 || nOutSize < Base64BufferSize(nInLen))
  {
    //MESS_NOMEM_ATTACH Not enough memory to encode the attachment in MIME-Base64
    theApp.Log(3,g_message[MESS_NOMEM_ATTACH][g_lang]);
    return FALSE;
  }

  //Set up the parameters prior to the main encoding loop
  int nInPos  = 0;
  int nOutPos = 0;
  int nLineLen = 0;

  // Get three characters at a time from the input buffer and encode them
  for (int i=0; i<nInLen/3; ++i) 
  {
    //Get the next 2 characters
    int c1 = pszIn[nInPos++] & 0xFF;
    int c2 = pszIn[nInPos++] & 0xFF;
    int c3 = pszIn[nInPos++] & 0xFF;

    //Encode into the 4 6 bit characters
    pszOut[nOutPos++] = m_base64tab[(c1 & 0xFC) >> 2];
    pszOut[nOutPos++] = m_base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
    pszOut[nOutPos++] = m_base64tab[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
    pszOut[nOutPos++] = m_base64tab[c3 & 0x3F];
    nLineLen += 4;

    //Handle the case where we have gone over the max line boundary
    if (nLineLen >= BASE64_MAXLINE-3) 
    {
      char* cp = EOL;
      pszOut[nOutPos++] = *cp++;
      if (*cp)
      {
        pszOut[nOutPos++] = *cp;
      }
      nLineLen = 0;
    }
  }

  // Encode the remaining one or two characters in the input buffer
  char* cp;
  int   c1;
  int   c2;
  switch (nInLen % 3) 
  {
    case 0:   cp = EOL;
              pszOut[nOutPos++] = *cp++;
              if (*cp)
              {
                pszOut[nOutPos++] = *cp;
              }
              break;
    case 1:   c1 = pszIn[nInPos] & 0xFF;
              pszOut[nOutPos++] = m_base64tab[(c1 & 0xFC) >> 2];
              pszOut[nOutPos++] = m_base64tab[((c1 & 0x03) << 4)];
              pszOut[nOutPos++] = '=';
              pszOut[nOutPos++] = '=';
              cp = EOL;
              pszOut[nOutPos++] = *cp++;
              if (*cp)
              {
                pszOut[nOutPos++] = *cp;
              }
              break;
    case 2:   c1 = pszIn[nInPos++] & 0xFF;
              c2 = pszIn[nInPos  ] & 0xFF;
              pszOut[nOutPos++] = m_base64tab[(c1 & 0xFC) >> 2];
              pszOut[nOutPos++] = m_base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
              pszOut[nOutPos++] = m_base64tab[((c2 & 0x0F) << 2)];
              pszOut[nOutPos++] = '=';
              cp = EOL;
              pszOut[nOutPos++] = *cp++;
              if (*cp)
              {
                pszOut[nOutPos++] = *cp;
              }
              break;
  }
  pszOut[nOutPos] = 0;
  *nOutLen = nOutPos;
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// Alternative Base64 encoder/decoder class
//
//////////////////////////////////////////////////////////////////////////


SMPTBase64Encode::SMPTBase64Encode() 
                 :m_pBuf(NULL)
                 ,m_nSize(0)
{
}

SMPTBase64Encode::~SMPTBase64Encode()
{
  if (m_pBuf)
  {
    delete [] m_pBuf;
  }
}

bool
SMPTBase64Encode::Encode(const BYTE* pData, int nSize, DWORD dwFlags)
{
  //Tidy up any heap memory we have been using
  if (m_pBuf)
  {
    delete [] m_pBuf;
  }
  // Calculate and allocate the buffer to store the encoded data
  // We allocate an extra byte so that we can null terminate the result
  m_nSize = ATL::Base64EncodeGetRequiredLength(nSize, dwFlags) + 1; 
  m_pBuf = new char[m_nSize + 1];

  //Finally do the encoding
  if(!ATL::Base64Encode(pData, nSize, m_pBuf, &m_nSize, dwFlags))
  {
    return false;
  }

  //Null terminate the data
  m_pBuf[m_nSize] = '\0';
  return true;
}

bool
SMPTBase64Encode::Decode(LPCSTR pData, int nSize)
{
  //Tidy up any heap memory we have been using
  if (m_pBuf)
  {
    delete [] m_pBuf;
  }
  //Calculate and allocate the buffer to store the encoded data
  m_nSize = ATL::Base64DecodeGetRequiredLength(nSize) + 1;
  m_pBuf = new char[m_nSize];

  //Finally do the encoding
  if (!ATL::Base64Decode(pData, nSize, reinterpret_cast<BYTE*>(m_pBuf), &m_nSize))
  {
    return false;
  }

  //Null terminate the data
  m_pBuf[m_nSize] = '\0';
  return true;
}

bool
SMPTBase64Encode::Encode(LPCSTR pszMessage, DWORD dwFlags)
{
  return Encode(reinterpret_cast<const BYTE*>(pszMessage), static_cast<int>(strlen(pszMessage)), dwFlags);
}

bool
SMPTBase64Encode::Decode(LPCSTR pszMessage)
{
  return Decode(pszMessage, static_cast<int>(strlen(pszMessage)));
}

