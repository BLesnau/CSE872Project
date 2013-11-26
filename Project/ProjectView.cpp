
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
END_MESSAGE_MAP()

// CProjectView construction/destruction

CProjectView::CProjectView()
{
   m_bValidImage = FALSE;
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
      drawnImage.Create( m_image.GetWidth(), m_image.GetHeight(), m_image.GetBPP() );
      m_image.BitBlt( drawnImage.GetDC(), 0, 0 );

      drawnImage.Draw( pDC->GetSafeHdc(), 0, 0 );

      drawnImage.ReleaseDC();
   }
}

void CProjectView::Draw()
{
   
}

void CProjectView::LoadNewImage( CString strFilePath )
{
   if( m_bValidImage )
   {
      m_image.Destroy();
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

   this->RedrawWindow();
}

void CProjectView::OnImageOpen()
{
   // TODO: Add your command handler code here
   //CImage image;
   //CString strFilter;
   //CSimpleArray<GUID> arrayGuidTypes;
   //HRESULT hResult = image.GetExporterFilterString(strFilter, arrayGuidTypes, _T( "All Image Files" ));

   //if(FAILED(hResult))
   //{
   //   //CString strError;
   //   //strError.Format("GetExporterFilter failed:\n%x - %s", hResult, _com_error(hResult).ErrorMessage());
   //   AfxMessageBox(L"Could not open image");
   //   return;
   //}

   CFileDialog fileDlg(true, NULL, NULL, OFN_FILEMUSTEXIST, L"Bitmap format|*.bmp|JPEG format|*.jpg;*.jpeg|GIF format|*.gif|PNG format|*.png|TIFF format|*.tif;*.tiff|");

   if(fileDlg.DoModal() != IDOK)
   {
      return;
   }

   LoadNewImage( fileDlg.GetPathName() );
}


void CProjectView::OnImageReset()
{

}


void CProjectView::OnSelectionClear()
{

}


void CProjectView::OnProcessEntireImage()
{

}

void CProjectView::OnProcessSelection()
{

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