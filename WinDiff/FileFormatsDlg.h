#pragma once

#include "FileFormat.h"

class CFileFormatsDlg: public CDialog {
private:
    FileFormatList m_nameList, m_origList;
    void updateListCtrl();
    int  getSelectedIndex();
    int  getListSize();
    void ajourButtons(int selected = -1);
    bool anythingChanged() const;
public:
  CFileFormatsDlg(CWnd *pParent = NULL);
  String m_selected;

    enum { IDD = IDD_DIALOGLISTFORMATNAMES };
    CListCtrl   m_nameListCtrl;


protected:
  virtual void DoDataExchange(CDataExchange* pDX);

protected:

    virtual void OnOK();
    afx_msg void OnButtonRename();
    afx_msg void OnButtonDelete();
    virtual BOOL OnInitDialog();
    afx_msg void OnItemChangedList( NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
  DECLARE_MESSAGE_MAP()
};

