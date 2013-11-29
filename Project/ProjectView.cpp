
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
      m_bValidImage2 = FALSE;
      m_dragState = IDLE;

      m_colors.push_back( RGB( 255, 0, 0 ) );
      m_colors.push_back( RGB( 0, 255, 0 ) );
      m_colors.push_back( RGB( 0, 0, 255 ) );
      m_colors.push_back( RGB( 255, 255, 0 ) );
      m_colors.push_back( RGB( 0, 255, 255 ) );
      m_colors.push_back( RGB( 255, 0, 255 ) );
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
      LoadNewImage( L"DefaultPic.jpeg", TRUE );
      LoadNewImage( L"DefaultPic2.jpeg", FALSE );
   }

   // CProjectView drawing
   void CProjectView::OnDraw(CDC* pDC)
   {
      CProjectDoc* pDoc = GetDocument();
      ASSERT_VALID(pDoc);
      if (!pDoc)
         return;

      if( m_bValidImage && m_bValidImage2 )
      {
         CImage drawnImage;
         CopyImage( &m_image, &drawnImage );

         CImage drawnImage2;
         CopyImage( &m_image2, &drawnImage2 );

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
                     drawnImage.SetPixel( x, y, m_colors.at( i % m_colors.size() ) );
                  }
               }
            }
         }

         for( int i=0; i<(int)m_selections2.size(); i++ )
         {
            auto selection = m_selections2.at( i );
            for( int x=selection.left; x<selection.right; x++ )
            {
               for( int y=selection.top; y<selection.bottom; y++ )
               {
                  if( x == selection.left || x == selection.right - 1 ||
                     y == selection.top || y == selection.bottom - 1)
                  {
                     drawnImage2.SetPixel( x, y, m_colors.at( i % m_colors.size() ) );
                  }
               }
            }
         }

         if( m_dragState == DRAGGING )
         {
            CRect tmpSelection = m_dragSelection;
            CorrectDragRect( &tmpSelection );
            BoundRect( &tmpSelection, TRUE );

            if( tmpSelection.Width() >= 4 && tmpSelection.Height() >= 4 )
            {
               for( int x=tmpSelection.left; x<tmpSelection.right; x++ )
               {
                  for( int y=tmpSelection.top; y<tmpSelection.bottom; y++ )
                  {
                     if( x == tmpSelection.left || x == tmpSelection.right - 1 ||
                        y == tmpSelection.top || y == tmpSelection.bottom - 1)
                     {
                        drawnImage.SetPixel( x, y, m_colors.at( m_selections.size() % m_colors.size() ) );
                     }
                  }
               }
            }
         }
         else if( m_dragState == FIXED )
         {
            CRect tmpSelection = m_dragSelection;
            CorrectDragRect( &tmpSelection );
            BoundRect( &tmpSelection, FALSE );

            if( tmpSelection.Width() >= 4 && tmpSelection.Height() >= 4 )
            {
               for( int x=tmpSelection.left; x<tmpSelection.right; x++ )
               {
                  for( int y=tmpSelection.top; y<tmpSelection.bottom; y++ )
                  {
                     if( x == tmpSelection.left || x == tmpSelection.right - 1 ||
                        y == tmpSelection.top || y == tmpSelection.bottom - 1)
                     {
                        drawnImage2.SetPixel( x, y, m_colors.at( m_selections2.size() % m_colors.size() ) );
                     }
                  }
               }
            }
         }

         drawnImage.Draw( pDC->GetSafeHdc(), 0, 0 );
         drawnImage2.Draw( pDC->GetSafeHdc(), drawnImage.GetWidth(), 0 );
      }
   }

   void CProjectView::CopyImage( CImage* src, CImage* dest )
   {
      dest->Destroy();

      dest->Create( src->GetWidth(), src->GetHeight(), src->GetBPP() );
      src->BitBlt( dest->GetDC(), 0, 0 );
      dest->ReleaseDC();
   }

   BOOL CProjectView::LoadNewImage( CString strFilePath, BOOL bFirstImage )
   {
      if( bFirstImage && m_bValidImage )
      {
         m_image.Destroy();
         m_origImage.Destroy();
      }
      else if( m_bValidImage2 )
      {
         m_image2.Destroy();
         m_origImage2.Destroy();
      }

      if( bFirstImage )
      {
         m_bValidImage = FALSE;
      }
      else
      {
         m_bValidImage2 = FALSE;
      }

      CImage *image = NULL;
      CImage *origImage = NULL;
      if( bFirstImage )
      {
         image = &m_image;
         origImage = &m_origImage;
      }
      else
      {
         image = &m_image2;
         origImage = &m_origImage2;
      }

      if( image->Load( strFilePath ) == S_OK )
      {
         if( bFirstImage )
         {
            m_bValidImage = TRUE;
         }
         else
         {
            m_bValidImage2 = TRUE;
         }

         CopyImage( image, origImage );
      }
      else
      {
         AfxMessageBox(L"Could not open image");
         return FALSE;
      }

      Invalidate(FALSE);

      return TRUE;
   }

   void CProjectView::OnImageOpen()
   {
      CFileDialog fileDlg(true, NULL, NULL, OFN_FILEMUSTEXIST, L"Bitmap format|*.bmp|JPEG format|*.jpg;*.jpeg|GIF format|*.gif|PNG format|*.png|TIFF format|*.tif;*.tiff|");

      if(fileDlg.DoModal() != IDOK)
      {
         return;
      }

      LoadNewImage( fileDlg.GetPathName(), TRUE );
   }

   void CProjectView::OnImageReset()
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      CopyImage( &m_origImage, &m_image );
      CopyImage( &m_origImage2, &m_image2 );

      Invalidate(FALSE);
   }

   void CProjectView::OnSelectionAutoselect()
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      if( m_dragState != IDLE )
      {
         return;
      }

      auto left = min(m_image.GetWidth()-1, 25);
      auto top = min(m_image.GetHeight()-1, 25);

      if( m_image.GetWidth() - 1 >= left + 50 && m_image.GetHeight() - 1 >= top + 50 )
      {
         if( m_image2.GetWidth() - 1 >= 50 && m_image.GetHeight() >= 50 )
         {
            auto rect = CRect( left, top, left + 50, top + 50 );
            m_selections.push_back( rect );

            auto rect2 = CRect( 0, 0, 50, 50 );
            m_selections2.push_back( rect2 );
         }
      }

      Invalidate(FALSE);
   }

   void CProjectView::OnSelectionClear()
   {
      m_selections.clear();
      m_selections2.clear();

      Invalidate(FALSE);
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

      if( m_dragState == IDLE )
      {
         return;
      }

      if( m_dragState == DRAGGING )
      {
         m_dragSelection.right = min( point.x, m_image.GetWidth()-1 );
         m_dragSelection.bottom = min( point.y, m_image.GetHeight()-1 );
      }
      else if( m_dragState == FIXED )
      {
         point.x -= m_image.GetWidth();

         if( point.x >= 0 && point.y >= 0 )
         {
            m_dragSelection.SetRect( point.x, point.y, point.x + m_dragSelection.Width(),  point.y + m_dragSelection.Height() );

            if( m_dragSelection.right > m_image2.GetWidth() )
            {
               m_dragSelection.SetRect( m_image2.GetWidth() - 1 - m_dragSelection.Width(), m_dragSelection.top, m_image2.GetWidth() - 1, m_dragSelection.bottom );
            }

            if( m_dragSelection.bottom > m_image2.GetHeight() )
            {
               m_dragSelection.SetRect( m_dragSelection.left, m_image2.GetHeight() - 1 - m_dragSelection.Height(), m_dragSelection.right, m_image2.GetHeight() - 1 );
            }
         }       
      }

      Invalidate(FALSE);
   }

   void CProjectView::OnLButtonDown( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      if( m_dragState == IDLE )
      {
         if( point.x >= m_image.GetWidth() || point.y >= m_image.GetHeight() )
         {
            return;
         }

         m_dragState = DRAGGING;
         m_dragSelection.left = max( point.x, 1 );
         m_dragSelection.top = max( point.y, 1 );
         m_dragSelection.right = m_dragSelection.left;
         m_dragSelection.bottom = m_dragSelection.top;
      }

      Invalidate(FALSE);
   }

   void CProjectView::OnLButtonUp( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      if( m_dragState == IDLE )
      {
         return;
      }

      if( m_dragState == DRAGGING )
      {     
         m_dragSelection.right = min( point.x, m_image.GetWidth()-1 );
         m_dragSelection.bottom = min( point.y, m_image.GetHeight()-1 );

         CorrectDragRect( &m_dragSelection );
         BoundRect( &m_dragSelection, TRUE );

         if( m_dragSelection.Width() >= 4 && m_dragSelection.Height() >= 4 )
         {
            m_selections.push_back( m_dragSelection );
         }

         m_dragSelection.SetRect( 0, 0, m_dragSelection.Width(), m_dragSelection.Height() );
         m_dragState = FIXED;
      }
      else if( m_dragState == FIXED )
      {
         if( m_dragSelection.Width() >= 4 && m_dragSelection.Height() >= 4 )
         {
            m_selections2.push_back( m_dragSelection );
         }

         m_dragSelection.SetRect(0, 0, 0, 0);
         m_dragState = IDLE;
      }

      Invalidate(FALSE);
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

   void CProjectView::BoundRect( CRect* rect, BOOL bFirstImage )
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      CImage *image = NULL;
      if( bFirstImage )
      {
         image = &m_image;
      }
      else
      {
         image = &m_image2;
      }

      rect->left = max(rect->left, 1);
      rect->top = max(rect->top, 1);
      rect->right = min(rect->right, image->GetWidth()-1);
      rect->bottom = min(rect->bottom, image->GetHeight()-1);
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