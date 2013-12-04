
// ProjectView.h : interface of the CProjectView class
//

#pragma once
#include <atlimage.h>
#include <vector>
#include "Selection.h"
#include "Selection.h"

class CSelection;

class CProjectView : public CView
{
protected: // create from serialization only
   CProjectView();
   DECLARE_DYNCREATE(CProjectView)

   // Attributes
public:
   CProjectDoc* GetDocument() const;
   CImage m_image;
   CImage m_origImage;
   BOOL m_bValidImage;
   CImage m_image2;
   CImage m_origImage2;
   BOOL m_bValidImage2;
   CSelection* m_dragSelection;
   CSelection::DragState m_dragState;
   std::vector<CSelection*> m_selections;
   std::vector<CSelection*> m_selections2;
   std::vector<COLORREF> m_colors;

   brInterface detection;

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

   BOOL LoadNewImage( CString strFilePath, BOOL bFirstImage );
   void CopyImage( CImage* src, CImage* dest );
   void CorrectDragRect( CRect* rect );
   void BoundRect( CRect* rect, BOOL bFirstImage );

protected:

   // Generated message map functions
protected:
   DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnImageOpen();
   afx_msg void OnImageReset();
   afx_msg void OnSelectionClear();
   afx_msg void OnProcessSelection();
   afx_msg void OnSelectionAutoselect();

   afx_msg void OnMouseMove( UINT nFlags, CPoint point );
   afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
   afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
   afx_msg void OnImageOpendestination();
};

#ifndef _DEBUG  // debug version in ProjectView.cpp
inline CProjectDoc* CProjectView::GetDocument() const
{ return reinterpret_cast<CProjectDoc*>(m_pDocument); }
#endif

