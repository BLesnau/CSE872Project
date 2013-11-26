
// ProjectView.h : interface of the CProjectView class
//

#pragma once
#include <atlimage.h>


class CProjectView : public CView
{
protected: // create from serialization only
	CProjectView();
	DECLARE_DYNCREATE(CProjectView)

// Attributes
public:
	CProjectDoc* GetDocument() const;
   CImage m_image;
   BOOL m_bValidImage;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
   virtual void OnInitialUpdate();
   
protected:

// Implementation
public:
	virtual ~CProjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   void LoadNewImage( CString strFilePath );

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnImageOpen();
   afx_msg void OnImageReset();
   afx_msg void OnSelectionClear();
   afx_msg void OnProcessEntireImage();
   afx_msg void OnProcessSelection();
};

#ifndef _DEBUG  // debug version in ProjectView.cpp
inline CProjectDoc* CProjectView::GetDocument() const
   { return reinterpret_cast<CProjectDoc*>(m_pDocument); }
#endif

