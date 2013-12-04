#pragma once
#include "selection.h"

class CPolySelection : public CSelection
{
public:
   CPolySelection();
   ~CPolySelection();

   virtual BOOL IsPointInSelection( int x, int y );
   virtual CPoint GetBasePoint();
   virtual void Normalize();
   virtual CSelection* Copy();
   virtual void OnLButtonDown( DragState dragState, CPoint point );
   virtual void OnLButtonUp( DragState dragState, CPoint point, CImage* pImage, std::vector<CSelection*>& selections );
   virtual void OnMouseMove( DragState dragState, CPoint point, CImage* pImage, CImage* pImage2);
   virtual void OnDraw( CImage* pImage, COLORREF color );
};