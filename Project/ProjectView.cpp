
// ProjectView.cpp : implementation of the CProjectView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Project.h"
#endif

#include "ProjectDoc.h"
#include "ProjectView.h"
#include <direct.h>
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProjectView

IMPLEMENT_DYNCREATE(CProjectView, CView)

   BEGIN_MESSAGE_MAP(CProjectView, CView)
      ON_COMMAND(ID_IMAGE_OPEN, &CProjectView::OnImageOpen)
      ON_COMMAND(ID_IMAGE_RESET, &CProjectView::OnImageReset)
      ON_COMMAND(ID_SELECTION_CLEAR, &CProjectView::OnSelectionClear)
      ON_COMMAND(ID_PROCESS_PROCESS_ENTIRE_IMAGE, &CProjectView::OnProcessEntireImage)
      ON_COMMAND(ID_PROCESS_PROCESS_SELECTION, &CProjectView::OnProcessSelection)
      ON_COMMAND(ID_SELECTION_AUTOSELECT, &CProjectView::OnSelectionAutoselect)
      ON_WM_MOUSEMOVE()
      ON_WM_LBUTTONDOWN()
      ON_WM_LBUTTONUP()
   END_MESSAGE_MAP()

   // CProjectView construction/destruction

   CProjectView::CProjectView()
   {
      m_bValidImage = FALSE;
      m_bDragging = FALSE;
   }

   CProjectView::~CProjectView()
   {
   }

   BOOL CProjectView::PreCreateWindow(CREATESTRUCT& cs)
   {
      // TODO: Modify the Window class or styles here by modifying
      //  the CREATESTRUCT cs

      return CView::PreCreateWindow(cs);
   }

   void CProjectView::OnInitialUpdate()
   {
      LoadNewImage( L"DefaultPic.jpeg" );
   }

   // CProjectView drawing
   void CProjectView::OnDraw(CDC* pDC)
   {
      CProjectDoc* pDoc = GetDocument();
      ASSERT_VALID(pDoc);
      if (!pDoc)
         return;

      if( m_bValidImage )
      {
         CImage drawnImage;
         CopyImage( &m_image, &drawnImage );

         // Draw selections
         for( int i=0; i<(int)m_selections.size(); i++ )
         {
            auto selection = m_selections.at( i );
            for( int x=selection.left; x<selection.right; x++ )
            {
               for( int y=selection.top; y<selection.bottom; y++ )
               {
                  if( x == selection.left || x == selection.right - 1 ||
                        y == selection.top || y == selection.bottom - 1)
                  {
                     drawnImage.SetPixel( x, y, RGB( 255, 0, 0 ) );
                  }
               }
            }
         }

         if( m_bDragging )
         {
            CRect tmpSelection = m_dragSelection;
            CorrectDragRect( &tmpSelection );
            BoundRect( &tmpSelection );

            if( tmpSelection.Width() >= 4 && tmpSelection.Height() >= 4 )
            {
               for( int x=tmpSelection.left; x<tmpSelection.right; x++ )
               {
                  for( int y=tmpSelection.top; y<tmpSelection.bottom; y++ )
                  {
                     if( x == tmpSelection.left || x == tmpSelection.right - 1 ||
                        y == tmpSelection.top || y == tmpSelection.bottom - 1)
                     {
                        drawnImage.SetPixel( x, y, RGB( 255, 0, 0 ) );
                     }
                  }
               }
            }
         }

         drawnImage.Draw( pDC->GetSafeHdc(), 0, 0 );
      }
   }

   void CProjectView::CopyImage( CImage* src, CImage* dest )
   {
      dest->Destroy();

      dest->Create( src->GetWidth(), src->GetHeight(), src->GetBPP() );
      src->BitBlt( dest->GetDC(), 0, 0 );
      dest->ReleaseDC();
   }

   void CProjectView::LoadNewImage( CString strFilePath )
   {
      if( m_bValidImage )
      {
         m_image.Destroy();
         m_origImage.Destroy();
      }

      m_bValidImage = FALSE;

      if( m_image.Load( strFilePath ) == S_OK )
      {
         m_bValidImage = TRUE;
      }
      else
      {
         AfxMessageBox(L"Could not open image");
      }

      CopyImage( &m_image, &m_origImage );

      Invalidate();
   }

   void CProjectView::OnImageOpen()
   {
      CFileDialog fileDlg(true, NULL, NULL, OFN_FILEMUSTEXIST, L"Bitmap format|*.bmp|JPEG format|*.jpg;*.jpeg|GIF format|*.gif|PNG format|*.png|TIFF format|*.tif;*.tiff|");

      if(fileDlg.DoModal() != IDOK)
      {
         return;
      }

      LoadNewImage( fileDlg.GetPathName() );
   }

   void CProjectView::OnImageReset()
   {
      if( !m_bValidImage )
      {
         return;
      }

      CopyImage( &m_origImage, &m_image );

      Invalidate();
   }

   void CProjectView::OnSelectionAutoselect()
   {
      if( !m_bValidImage )
      {
         return;
      }

      auto left = min(m_image.GetWidth()-1, 25);
      auto top = min(m_image.GetHeight()-1, 25);

      if( m_image.GetWidth() - 1 >= left + 50 && m_image.GetHeight() - 1 >= top + 50  )
      {
         auto rect = CRect( left, top, left + 50, top + 50 );
         m_selections.push_back( rect );
      }

      Invalidate();
   }

   void CProjectView::OnSelectionClear()
   {
      m_selections.clear();

      Invalidate();
   }

   void CProjectView::OnProcessEntireImage()
   {

   }

   void CProjectView::OnProcessSelection()
   {

   }

   void CProjectView::OnMouseMove( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage )
      {
         return;
      }

      if( !m_bDragging )
      {
         return;
      }

      m_dragSelection.right = min( point.x, m_image.GetWidth()-1 );
      m_dragSelection.bottom = min( point.y, m_image.GetHeight()-1 );

      Invalidate();
   }

   void CProjectView::OnLButtonDown( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage )
      {
         return;
      }

      m_bDragging = TRUE;
      m_dragSelection.left = max( point.x, 1 );
      m_dragSelection.top = max( point.y, 1 );
      m_dragSelection.right = m_dragSelection.left;
      m_dragSelection.bottom = m_dragSelection.top;

      Invalidate();
   }

   void CProjectView::OnLButtonUp( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage )
      {
         return;
      }

      if( !m_bDragging )
      {
         return;
      }

      m_dragSelection.right = min( point.x, m_image.GetWidth()-1 );
      m_dragSelection.bottom = min( point.y, m_image.GetHeight()-1 );

      CorrectDragRect( &m_dragSelection );
      BoundRect( &m_dragSelection );

      if( m_dragSelection.Width() >= 4 && m_dragSelection.Height() >= 4 )
      {
         m_selections.push_back( m_dragSelection );
      }

      m_bDragging = FALSE;
      m_dragSelection.SetRect(0, 0, 0, 0);

      Invalidate();
   }

   void CProjectView::CorrectDragRect( CRect* rect )
   {
      if( rect->left > rect->right )
      {
         auto tmp = rect->left;
         rect->left = rect->right;
         rect->right = tmp;
      }

      if( rect->top > rect->bottom )
      {
         auto tmp = rect->top;
         rect->top = rect->bottom;
         rect->bottom = tmp;
      }
   }

   void CProjectView::BoundRect( CRect* rect )
   {
      if( !m_bValidImage )
      {
         return;
      }

      rect->left = max(rect->left, 1);
      rect->top = max(rect->top, 1);
      rect->right = min(rect->right, m_image.GetWidth()-1);
      rect->bottom = min(rect->bottom, m_image.GetHeight()-1);
   }

   // CProjectView diagnostics
#ifdef _DEBUG
   void CProjectView::AssertValid() const
   {
      CView::AssertValid();
   }

   void CProjectView::Dump(CDumpContext& dc) const
   {
      CView::Dump(dc);
   }

   CProjectDoc* CProjectView::GetDocument() const // non-debug version is inline
   {
      ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProjectDoc)));
      return (CProjectDoc*)m_pDocument;
   }
#endif //_DEBUG