#include "StdAfx.h"
#include "PolySelection.h"

CPolySelection::CPolySelection()
{
}

CPolySelection::~CPolySelection()
{
}

BOOL CPolySelection::IsPointInSelection( int x, int y )
{
   return TRUE;
}

CPoint CPolySelection::GetBasePoint()
{
   CPoint p( 0, 0 );
   return p;
}

void CPolySelection::OnLButtonDown( DragState dragState, CPoint point )
{
   
}

void CPolySelection::OnLButtonUp( DragState dragState, CPoint point, CImage* pImage, std::vector<CSelection*>& selections )
{
   
}

void CPolySelection::OnMouseMove( DragState dragState, CPoint point, CImage* pImage, CImage* pImage2 )
{
   
}

void CPolySelection::OnDraw( CImage* pImage, COLORREF color )
{

}

void CPolySelection::Normalize()
{
   
}

CSelection* CPolySelection::Copy()
{
   return NULL;
}