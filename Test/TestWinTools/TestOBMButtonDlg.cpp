#include "stdafx.h"
#include <MyUtil.h>
#include <MFCUtil/WinTools.h>
#include "testwintools.h"
#include "TestOBMButtonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTestOBMButtonDlg::CTestOBMButtonDlg(CWnd* pParent /*=NULL*/) : CDialog(CTestOBMButtonDlg::IDD, pParent) {
    //{{AFX_DATA_INIT(CTestOBMButtonDlg)
	m_buttonsEnabled = TRUE;
	//}}AFX_DATA_INIT
}

void CTestOBMButtonDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTestOBMButtonDlg)
	DDX_Check(pDX, IDC_CHECK_ENABLEBUTTONS, m_buttonsEnabled);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestOBMButtonDlg, CDialog)
    //{{AFX_MSG_MAP(CTestOBMButtonDlg)
	ON_BN_CLICKED(IDC_CHECK_ENABLEBUTTONS, OnCheckEnableButtons)
    ON_BN_CLICKED(IDC_DNARROW_BUTTON     , OnButtonDNARROW)
    ON_BN_CLICKED(IDC_LFARROW_BUTTON     , OnButtonLFARROW)
    ON_BN_CLICKED(IDC_RGARROW_BUTTON     , OnButtonRGARROW)
    ON_BN_CLICKED(IDC_UPARROW_BUTTON     , OnButtonUPARROW)
    ON_BN_CLICKED(IDC_ZOOM_BUTTON        , OnButtonZOOM   )
    ON_BN_CLICKED(IDC_REDUCE_BUTTON      , OnButtonREDUCE )
    ON_BN_CLICKED(IDC_RESTORE_BUTTON     , OnButtonRESTORE)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CTestOBMButtonDlg::OnOK() {
}

BOOL CTestOBMButtonDlg::OnInitDialog() {
  CDialog::OnInitDialog();
    
  CPoint p(20, 20);

  m_dnArrayButton.Create(this, OBMIMAGE(DNARROW), p, IDC_DNARROW_BUTTON, true); p.y += 20;
  m_lfArrowButton.Create(this, OBMIMAGE(LFARROW), p, IDC_LFARROW_BUTTON, true); p.y += 20;
  m_rgArrowbutton.Create(this, OBMIMAGE(RGARROW), p, IDC_RGARROW_BUTTON, true); p.y += 20;
  m_upArrowButton.Create(this, OBMIMAGE(UPARROW), p, IDC_UPARROW_BUTTON, true); p.y += 20;
  m_zoomButton.Create(   this, OBMIMAGE(ZOOM   ), p, IDC_ZOOM_BUTTON   , true); p.y += 20;
  m_reduceButton.Create( this, OBMIMAGE(REDUCE ), p, IDC_REDUCE_BUTTON , true); p.y += 20;
  m_restoreButton.Create(this, OBMIMAGE(RESTORE), p, IDC_RESTORE_BUTTON, true); p.y += 20;

  return TRUE;
}

void CTestOBMButtonDlg::OnCheckEnableButtons() {
  BOOL enabled = IsDlgButtonChecked(IDC_CHECK_ENABLEBUTTONS);
  m_dnArrayButton.EnableWindow(enabled);
  m_lfArrowButton.EnableWindow(enabled);
  m_rgArrowbutton.EnableWindow(enabled);
  m_upArrowButton.EnableWindow(enabled);
  m_zoomButton.EnableWindow(   enabled);
  m_reduceButton.EnableWindow( enabled);
  m_restoreButton.EnableWindow(enabled);
}

void CTestOBMButtonDlg::OnButtonDNARROW() {
  MessageBox(_T("Down-ARROW pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}

void CTestOBMButtonDlg::OnButtonLFARROW() {
  MessageBox(_T("Left-ARROW pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}

void CTestOBMButtonDlg::OnButtonRGARROW() {
  MessageBox(_T("Right-ARROW pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}

void CTestOBMButtonDlg::OnButtonUPARROW() {
  MessageBox(_T("Up-ARROW pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}

void CTestOBMButtonDlg::OnButtonZOOM() {
  MessageBox(_T("ZOOM-button pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}

void CTestOBMButtonDlg::OnButtonREDUCE() {
  MessageBox(_T("REDUCE-button pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}

void CTestOBMButtonDlg::OnButtonRESTORE() {
  MessageBox(_T("RESTORE-button pushed"), _T("Button pushed"), MB_ICONINFORMATION);
}
