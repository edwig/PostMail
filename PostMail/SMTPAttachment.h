/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: SMTPAttachment.h
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

class SMTPAttachment
{
public:
  // Constructors / Destructors
	SMTPAttachment();
 ~SMTPAttachment();

  // Methods
  bool        Attachment(const XString& sFilename);
  XString     GetTitle()         const { return m_title;       }
  XString     GetFilename()      const { return m_filename;    }
  long        GetFileSize()      const { return m_fileSize;    }
  const BYTE* GetEncodedBuffer();
  const BYTE* ReadInBuffer();
  int         GetEncodedSize();
  XString     PrintableSize();

protected:
  bool        ReadAndEncode(bool p_tryonly = false);
  bool        ReadBuffer   (bool p_tryonly = false);

  XString     m_filename;    // The filename you want to send
  XString     m_title;       // What it is to be known as when emailed
  bool        m_readFile;    // We have read the file
  BYTE*       m_encoded;     // The encoded representation of the file
  int         m_encodedSize; // size of the encoded string
  long        m_fileSize;    // Size of original file
};
