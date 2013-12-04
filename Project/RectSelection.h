#pragma once
#include "selection.h"

class CRectSelection : public CSelection
{
public:
   CRectSelection( int l, int t, int r, int b );
   ~CRectSelection();

   virtual BOOL IsPointInSelection( int x, int y );
   virtual CPoint GetBasePoint();
   virtual void Normalize();
   virtual CSelection* Copy();
   virtual void OnLButtonDown( DragState dragState, CPoint point );
   virtual void OnLButtonUp( DragState dragState, CPoint point, CImage* pImage, std::vector<CSelection*>& selections );
   virtual void OnMouseMove( DragState dragState, CPoint point, CImage* pImage, CImage* pImage2 );
   virtual void OnDraw( CImage* pImage, COLORREF color );

private:
   void BoundRect( CImage* pImage, CRect* rect );
   void CorrectDragRect( CRect* rect );

   CRect m_rect;
};