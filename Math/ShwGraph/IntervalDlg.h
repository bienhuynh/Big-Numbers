#pragma once

class IntervalDlg : public CDialog {
public:
    IntervalDlg(CWnd* pParent = NULL);

    double m_minx, m_maxx, m_miny, m_maxy;

    enum { IDD = IDD_INTERVAL_DIALOG };
    CString m_maxxstr;
    CString m_maxystr;
    CString m_minxstr;
    CString m_minystr;
    BOOL    m_autoscaley;

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};

