
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
#include "br_interface.h"
#include <sstream>
#include "Selection.h"
#include "RectSelection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProjectView

IMPLEMENT_DYNCREATE(CProjectView, CView)

   BEGIN_MESSAGE_MAP(CProjectView, CView)
      ON_COMMAND(ID_IMAGE_OPEN, &CProjectView::OnImageOpen)
      ON_COMMAND(ID_IMAGE_RESET, &CProjectView::OnImageReset)
      ON_COMMAND(ID_SELECTION_CLEAR, &CProjectView::OnSelectionClear)
      ON_COMMAND(ID_PROCESS_PROCESS_SELECTION, &CProjectView::OnProcessSelection)
      ON_COMMAND(ID_SELECTION_AUTOSELECT, &CProjectView::OnSelectionAutoselect)
      ON_WM_MOUSEMOVE()
      ON_WM_LBUTTONDOWN()
      ON_WM_LBUTTONUP()
      ON_COMMAND(ID_IMAGE_OPENDESTINATION, &CProjectView::OnImageOpendestination)
   END_MESSAGE_MAP()

   // CProjectView construction/destruction

   CProjectView::CProjectView()
   {
      m_bValidImage = FALSE;
      m_bValidImage2 = FALSE;
      m_dragState = CSelection::IDLE;

      m_colors.push_back( RGB( 255, 0, 0 ) );
      m_colors.push_back( RGB( 0, 255, 0 ) );
      m_colors.push_back( RGB( 0, 0, 255 ) );
      m_colors.push_back( RGB( 255, 255, 0 ) );
      m_colors.push_back( RGB( 0, 255, 255 ) );
      m_colors.push_back( RGB( 255, 0, 255 ) );
   }

   CProjectView::~CProjectView()
   {
      delete m_dragSelection;

      for( int i=0; i<(int)m_selections.size(); i++ )
      {
         auto selection = m_selections.at( i );
         delete selection;
      }

      for( int i=0; i<(int)m_selections2.size(); i++ )
      {
         auto selection = m_selections2.at( i );
         delete selection;
      }

      m_selections.clear();
      m_selections2.clear();
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
            std::ostringstream boxOut; 
            boxOut << "Total: " << m_selections.size() << " Drawing: " << i << "\t" << selection.left << " " << selection.right << " " << selection.bottom << " " << selection.top << std::endl;
            OutputDebugStringA(boxOut.str().c_str());

         	selection->OnDraw( &drawnImage, m_colors.at( i % m_colors.size() ) );
         }

         for( int i=0; i<(int)m_selections2.size(); i++ )
         {
            auto selection = m_selections2.at( i );
            selection->OnDraw( &drawnImage2, m_colors.at( i % m_colors.size() ) );
         }

         if( m_dragState == CSelection::DRAGGING )
         {
            m_dragSelection->OnDraw( &drawnImage, m_colors.at( m_selections.size() % m_colors.size() ) );
         }
         else if( m_dragState == CSelection::FIXED )
         {
            m_dragSelection->OnDraw( &drawnImage2, m_colors.at( m_selections2.size() % m_colors.size() ) );
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

      Invalidate();

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

   void CProjectView::OnImageOpendestination()
   {
      CFileDialog fileDlg(true, NULL, NULL, OFN_FILEMUSTEXIST, L"Bitmap format|*.bmp|JPEG format|*.jpg;*.jpeg|GIF format|*.gif|PNG format|*.png|TIFF format|*.tif;*.tiff|");

      if(fileDlg.DoModal() != IDOK)
      {
         return;
      }

      LoadNewImage( fileDlg.GetPathName(), FALSE );
   }

   void CProjectView::OnImageReset()
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      CopyImage( &m_origImage, &m_image );
      CopyImage( &m_origImage2, &m_image2 );

      Invalidate();
   }

   // auto-select -- use openbr to do some feature detection
   void CProjectView::OnSelectionAutoselect()
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      if( m_dragState != CSelection::IDLE )
      {
         return;
      }

      detection.pointCorrespondence(m_image, m_image2, m_selections, m_selections2);

      std::ostringstream postCallOut;
      postCallOut << "received " << m_selections.size() << " vs. " << m_selections2.size() << " output" << std::endl;
      ::OutputDebugStringA(postCallOut.str().c_str());

/*      auto left = min(m_image.GetWidth()-1, 25);
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
      }*/
      
      Invalidate();
   }

   void CProjectView::OnSelectionClear()
   {
      m_selections.clear();
      m_selections2.clear();

      Invalidate();
   }

   // process selection -- todo -- link poisson code
   void CProjectView::OnProcessSelection()
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         AfxMessageBox( L"At least one valid image is not loaded" );
         return;
      }

      if( m_selections.size() != m_selections2.size() )
      {
         AfxMessageBox( L"Each image has a different number of selections" );
         return;
      }

      clone2(m_image, m_image2, m_selections, m_selections2);

      /*
      CRect outRect;
      for( int i=0; i<m_selections.size(); i++ )
      {
         auto srcSel = m_selections.at(i);
         auto dstSel = m_selections2.at(i);

         for( int w=0; w<=srcSel.Width(); w++ )
         {
            for( int h=0; h<=srcSel.Height(); h++ )
            {
               auto clr = m_image.GetPixel( srcSel.left + w, srcSel.top + h );
               m_image2.SetPixel( dstSel.left + w, dstSel.top + h, clr );
            }
         }
      }
      //*/

      OnSelectionClear();
   }

   void CProjectView::OnMouseMove( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage )
      {
         return;
      }

      if( m_dragState == CSelection::IDLE )
      {
         return;
      }

      m_dragSelection->OnMouseMove( m_dragState, point, &m_image, &m_image2 );

      Invalidate(FALSE);
   }

   void CProjectView::OnLButtonDown( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      if( m_dragState == CSelection::IDLE )
      {
         if( point.x >= m_image.GetWidth() || point.y >= m_image.GetHeight() )
         {
            return;
         }

         m_dragState = CSelection::DRAGGING;
         m_dragSelection = new CRectSelection( max( point.x, 1 ), max( point.y, 1 ),
                                               max( point.x, 1 ), max( point.y, 1 ) );
      }

      Invalidate();
   }

   void CProjectView::OnLButtonUp( UINT nFlags, CPoint point )
   {
      if( !m_bValidImage || !m_bValidImage2 )
      {
         return;
      }

      if( m_dragState == CSelection::IDLE )
      {
         return;
      }

      if( m_dragState == CSelection::DRAGGING )
      {     
         m_dragSelection->OnLButtonUp( m_dragState, point, &m_image, m_selections );

         m_dragSelection = m_dragSelection->Copy();
         m_dragSelection->Normalize();
         m_dragState = CSelection::FIXED;
      }
      else if( m_dragState == CSelection::FIXED )
      {
         m_dragSelection->OnLButtonUp( m_dragState, point, &m_image2, m_selections2 );

         m_dragSelection = NULL;
         m_dragState = CSelection::IDLE;
      }

      Invalidate();
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