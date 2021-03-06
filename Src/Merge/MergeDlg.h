
#pragma once

// CMergeDlg dialog
class CMergeDlg : public CDialogEx
{
// Construction
public:
	CMergeDlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CMergeDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MERGE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


public:
	vector<string> m_srcDirs;
	string m_dstDir;

	int m_curCnt;
	int m_totalCnt;
	bool m_isSearchLoop;
	std::thread m_searchThread;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_totalProgress;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CString m_progressStr;
};
