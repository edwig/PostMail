/////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: RelayDlg.h
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
#pragma  once
#include "resource.h"

class RelayDlg : public StyleDialog
{
public:
   RelayDlg(CDialog* p_parent,bool show);
  ~RelayDlg();
   // Dialog Data
	 enum { IDD = IDD_RELAY };
  
public:
   void DoTheInit();
   BOOL OnInitDialog();
   void Reset();

   void SetText(XString p_text);    // Set text under the gauge
   void SetCurrent(int p_current);  // Set current position
   void SetMax(int p_max);          // Set max positions
   int  GetMax();
   void Increment();                // Increment current position
   void SetAtEnd();                 // Set gauge at end

   void DoDataExchange(CDataExchange* pDX);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void   OnTimer(UINT_PTR nIDEvent);

private:
   void Repaint();
   void SetBkColor(COLORREF p_colorref);

   StyleEdit     m_editText;  // Text control
   CProgressCtrl m_gauge;     // The gauge progress control
   CString       m_text;      // Text under the gauge
   int           m_current;   // Current position
   int           m_max;       // Max positions
   int           m_show;      // Show / Hide dialog
   CBrush*       m_brush;
};

inline int
RelayDlg::GetMax()
{
  return m_max;
}