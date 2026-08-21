/////////////////////////////////////////////////////////////////////////////
// 
// CredentialHandle
//
// Original idea:
// David Maw: https://www.codeproject.com/Articles/1000189/A-Working-TCP-Client-and-Server-With-SSL
//
#include "stdafx.h"
#include "SecurityHandle.h"
#include "SecureClientSocket.h"

#ifdef _SSL_socket_logging
#define new SSL_socket_logging_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = __FILE__;
#endif

void CredentialHandle::Close() noexcept
{
   if (*this)
   {
      SecureClientSocket::g_pSSPI->FreeCredentialsHandle(&m_value);
   }
}

void SecurityContextHandle::Close() noexcept
{
   if (*this)
   {
      SecureClientSocket::g_pSSPI->DeleteSecurityContext(&m_value);
   }
}