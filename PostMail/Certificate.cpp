/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Certificate.cpp
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
#include "Certificate.h"
#include "CreateCertificate.h"
#include "Logging.h"
#include "SSLUtilities.h"
#include <wincrypt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Given a pointer to a certificate context, return the certificate name 
// (the friendly name if there is one, the subject name otherwise).
//
CString GetCertName(PCCERT_CONTEXT pCertContext)
{
  CString certName;
  auto good = CertGetNameString(pCertContext
                               ,CERT_NAME_FRIENDLY_DISPLAY_TYPE
                               ,0
                               ,NULL
                               ,certName.GetBuffer(128)
                               ,certName.GetAllocLength() - 1);
  certName.ReleaseBuffer();
  if(good)
  {
    return certName;
  }
  else
  {
    return _T("<unknown>");
  }
}

// Function to evaluate the certificate returned from the server
// if you want to keep it around call CertDuplicateCertificateContext, then CertFreeCertificateContext to free it
bool CertAcceptable(PCCERT_CONTEXT pCertContext,const bool trusted,const bool matchingName)
{
  CString status = trusted ? _T("A trusted") : _T("An untrusted");

  status += _T(" server certificate was returned with a name ");
  status += matchingName ? _T("match") : _T("mismatch");

  // wcout for WCHAR* handling
  status += _T(" called \"") + GetCertName(pCertContext) + _T("\"");
  DebugMsg(status);

#ifdef _DEBUG
  if(SSL_socket_logging && pCertContext)
  {
    ShowCertInfo(pCertContext,_T("Client Received Server Certificate"));
  }
#endif
  return true; // Any certificate will do
}

// This will get called once, or twice, the first call with "Required" false, which can return any
// certificate it likes, or none at all. If it returns one, that will be sent to the server.
// If that call did not return an acceptable certificate, the procedure may be called again if the server requests a 
// client certificate, whatever is returned on the first call (including null) is sent to the server which gets to decide
// whether or not it is acceptable. If there is a second call (which will have "Required" true and may have 
// pIssuerListInfo non-NULL) it MUST return a certificate or the handshake will fail.

SECURITY_STATUS SelectClientCertificate(PCCERT_CONTEXT& pCertContext,SecPkgContext_IssuerListInfoEx* pIssuerListInfo,bool Required)
{
  SECURITY_STATUS retval = SEC_E_CERT_UNKNOWN;
  CString status;

  if(Required)
  {
    // A client certificate must be returned or the handshake will fail
    if(pIssuerListInfo && pIssuerListInfo->cIssuers == 0)
    {
      status = _T("Client certificate required, issuer list is empty");
    }
    else if(pIssuerListInfo)
    {
      status = _T("Client certificate required, issuer list provided");
      retval = CertFindFromIssuerList(pCertContext,*pIssuerListInfo);
      if(!pCertContext)
      {
        status += _T(" but no certificates matched");
      }
    }
    if(!pCertContext)
    {
      // Select any valid certificate, regardless of issuer
      retval = CertFindClient(pCertContext);
    }
    // If a search for a required client certificate failed, just make one
    if(!pCertContext)
    {
      status += _T(", none found, creating one");
      pCertContext = CreateCertificate(false,GetUserName() + _T(" at ") + GetHostName());
      if(pCertContext)
      {
        retval = S_OK;
      }
      else
      {
        DWORD LastError = GetLastError();
        DebugMsg(status);
        status.Format(_T("**** Error 0x%X in CreateCertificate for a client certificate"),LastError);
        retval = HRESULT_FROM_WIN32(LastError);
      }
    }
    DebugMsg(status);
  }
  else
  {
    DebugMsg(_T("Optional client certificate requested (without issuer list)"));
    // Enable the next line to preemptively guess at an appropriate certificate 
    // Maybe we can guess and find a certificate
    if(FAILED(retval))
    {
      retval = CertFindClient(pCertContext); // Select any valid certificate
    }
  }
  if(pCertContext)
  {
    status = _T("Selected name: ") + GetCertName(pCertContext);
    DebugMsg(status);
  }
  else
  {
    DebugMsg(_T("No certificate found."));
  }
#ifdef _DEBUG
  if(SSL_socket_logging && pCertContext)
  {
    ShowCertInfo(pCertContext,_T("Client certificate being returned"));
  }
#endif
  return retval;
}
