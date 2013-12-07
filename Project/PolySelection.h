#pragma once
#include "selection.h"

class CPolySelection : public CSelection
{
public:
   CPolySelection( int x, int y );
   ~CPolySelection();

   virtual BOOL IsPointInSelection( int x, int y );
   virtual CPoint GetBasePoint();
   virtual void Normalize();
   virtual CSelection* Copy();
   virtual CRect GetBoundingBox();
   virtual void OnLButtonDown( DragState dragState, CPoint point );
   virtual void OnLButtonUp( DragState dragState, CPoint point, CImage* pImage, std::vector<CSelection*>& selections );
   virtual void OnMouseMove( DragState dragState, CPoint point, CImage* pImage, CImage* pImage2);
   virtual void OnDraw( CImage* pImage, COLORREF color );

  std::vector<CPoint> m_verts;
  CPoint m_nextPoint;
  BOOL m_bIsFinished;
};