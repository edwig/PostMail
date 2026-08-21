/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPAttachment.cpp
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
#include "SMTPAttachment.h"
#include "PostMail.h"
#include "Message.h"
#include "Base64.h"
#include "Office365.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

SMTPAttachment::SMTPAttachment()
{
  m_encoded     = nullptr;
  m_encodedSize = 0;
  m_fileSize    = 0;
  m_readFile    = false;
}

SMTPAttachment::~SMTPAttachment()
{
  // free up any memory we allocated
  if(m_encoded)
  {
    delete [] m_encoded;
    m_encoded = nullptr;
  }
}

bool
SMTPAttachment::Attachment(const XString& p_filename)
{
  if(!p_filename.GetLength())
  {
    // MESS_NOFILENAME An empty filename for an attachment is not allowed
    XString mess = g_message[MESS_NOFILENAME][g_lang];
    theApp.Log(3,mess);
    theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
    return false;
  }

  // Hive away the filename
  TCHAR path [_MAX_PATH];
  TCHAR fname[_MAX_FNAME];
  TCHAR ext  [_MAX_EXT];
  _tsplitpath_s(p_filename, NULL, 0, NULL, 0, fname,_MAX_FNAME,ext,_MAX_EXT);
  _tmakepath_s(path, _MAX_PATH, NULL, NULL, fname, ext);
  m_filename = p_filename;
  m_title    = path;

  return true;
}

XString
SMTPAttachment::PrintableSize()
{
  if(!m_encoded && !ReadAndEncode(true))
  {
    return _T("Unknown");
  }

  XString size;
  if(m_fileSize < 1024)
  {
    // Measured in bytes
    size.Format(_T("%d Bytes"),m_fileSize);
  }
  else if(m_fileSize < (1024*1024))
  {
    // Measured in KiloBytes
    size.Format(_T("%d KiloBytes"), 1 + (m_fileSize / 1024));
  }
  else
  {
    // Measured in MegaBytes
    size.Format(_T("%d MegaBytes"),1 + (m_fileSize / 1024 / 1024));
  }
  return size;
}

const BYTE* 
SMTPAttachment::GetEncodedBuffer() 
{ 
  if(!m_encoded)
  {
    ReadAndEncode(true);
  }
  return m_encoded; 
};

const BYTE* 
SMTPAttachment::ReadInBuffer()
{
  if(!m_encoded)
  {
    ReadBuffer(true);
  }
  return m_encoded;
}


int
SMTPAttachment::GetEncodedSize()
{
  if(!m_encoded)
  {
    ReadAndEncode(true);
  }
  return m_encodedSize;
};

//////////////////////////////////////////////////////////////////////////
//
// PRIVATE
//
//////////////////////////////////////////////////////////////////////////

bool
SMTPAttachment::ReadAndEncode(bool p_tryonly /*=false*/)
{
  // free up any memory we previously allocated
  if (m_encoded)
  {
    delete [] m_encoded;
    m_encoded = nullptr;
    m_encodedSize = 0;
  }

  // determine the file size
  WinFile infile(m_filename);
  size_t  size = infile.GetFileSize();
  if(size == INVALID_FILE_SIZE || infile.GetLastError())
  {
    if(!p_tryonly)
    {
      // MESS_NOFILE "Failed to get the status for file %s, it probably does not exist
      XString mess;
      mess.Format(g_message[MESS_NOFILE][g_lang],m_filename.GetString());
      mess += _T("\nOS Error: ");
      mess += infile.GetLastErrorString();
      theApp.Log(3,mess);
      theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
    }
    return false;
  }
  m_fileSize = (long)size;

  // Open up the file for reading in
  if(!infile.Open(FFlag::open_if_exists   | FFlag::open_read | 
                  FFlag::open_shared_read | FFlag::open_trans_binary))
  {
    if(!p_tryonly)
    {
      // MESS_OPENFILE "Failed to open file to be attached: %s"
      XString mess;
      mess.Format(g_message[MESS_OPENFILE][g_lang],m_filename.GetString());
      theApp.Log(3,mess);
      theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
    }
    return false;
  }

  // read in the contents of the input file
  int didread(0);
  BYTE* content = new BYTE[size];
  if(!infile.Read(content,size,didread))
  {
    // MESS_ATTACHMENT_IN_USE "Cannot open an attachment. Is it still opened in a viewer?\nAttachment: "
    XString mess;
    mess.Format(g_message[MESS_ATTACHMENT_IN_USE][g_lang],m_filename.GetString());
    theApp.Log(3,mess);
    theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK | MB_ICONERROR);
    return false;
  }

  // Allocate the encoded buffer
  int outSize = Base64::Base64BufferSize((int)size);
  m_encoded = new BYTE[outSize];

  // Do the encoding
  Base64::EncodeBase64(content, (int)size, m_encoded, outSize, &m_encodedSize);

  // delete the input buffer
  delete [] content;

  // Close the input file
  infile.Close();

  return true;
}

bool
SMTPAttachment::ReadBuffer(bool p_tryonly /*= false*/)
{
  // free up any memory we previously allocated
  if(m_encoded)
  {
    delete [] m_encoded;
    m_encoded = nullptr;
    m_encodedSize = 0;
  }

  // determine the file size
  WinFile infile(m_filename);
  size_t  size = infile.GetFileSize();
  if(size == INVALID_FILE_SIZE || infile.GetLastError())
  {
    if(!p_tryonly)
    {
      // MESS_NOFILE "Failed to get the status for file %s, it probably does not exist
      XString mess;
      mess.Format(g_message[MESS_NOFILE][g_lang],m_filename.GetString());
      mess += _T("\nOS Error: ");
      mess += infile.GetLastErrorString();
      theApp.Log(3, mess);
      theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
    }
    return false;
  }
  m_fileSize = (long)size;

  // Open up the file for reading in
  if(!infile.Open(FFlag::open_if_exists   | FFlag::open_read | 
                  FFlag::open_shared_read | FFlag::open_trans_binary))
  {
    if(!p_tryonly)
    {
      // MESS_OPENFILE "Failed to open file to be attached: %s"
      XString mess;
      mess.Format(g_message[MESS_OPENFILE][g_lang],m_filename.GetString());
      theApp.Log(3,mess);
      theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK|MB_ICONERROR);
    }
    return false;
  }

  if(m_fileSize < OFFICE365_INLINE_MAX)
  {
    // read in the contents of the input file
    int didread(0);
    BYTE* content = new BYTE[size];
    if(!infile.Read(content,size,didread))
    {
      // MESS_ATTACHMENT_IN_USE "Cannot open an attachment. Is it still opened in a viewer?\nAttachment: "
      XString mess;
      mess.Format(g_message[MESS_ATTACHMENT_IN_USE][g_lang],m_filename.GetString());
      theApp.Log(3,mess);
      theApp.WideMessageBox(NULL,mess,WhoAmI,MB_OK | MB_ICONERROR);
      return false;
    }

    // Allocate the encoded buffer
    int outSize = Base64::Base64BufferSize(m_fileSize);
    m_encoded = new BYTE[outSize];

    // Do the encoding
    Base64::EncodeBase64(content,m_fileSize,m_encoded,outSize,&m_encodedSize);

    // delete the input buffer
    delete[] content;
  }
  else
  {
    // read in the contents of the input file
    // Will be sent as binary data!
    int didread(0);
    m_encoded = new BYTE[size];
    infile.Read(m_encoded,size,didread);
  }
  // Close the input file
  infile.Close();

  return true;
}
