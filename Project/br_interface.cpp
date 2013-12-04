#include "stdafx.h"

#include "br_interface.h"
#include "Poisson_Multigrid\poisson_multigrid.h"

// A min macro is clearly helpful, and not just obnoxious
#undef min

#include <openbr/openbr.h>
#include <openbr/openbr_plugin.h>

#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc.hpp>

using namespace br;

//        Globals->abbreviations.insert("RectFromStasmEyes","RectFromPoints([],0.15,5.3)");
//        Globals->abbreviations.insert("RectFromStasmBrow","RectFromPoints([16,17,18,19,20,21,22,23,24,25,26,27],0.15,5)");
//        Globals->abbreviations.insert("RectFromStasmNose","RectFromPoints([],0.15,1.15)");
//        Globals->abbreviations.insert("RectFromStasmMouth","RectFromPoints([],0.3,2)");
//        Globals->abbreviations.insert("RectFromStasmHair", "RectFromPoints([13,14,15],1.75,1.5)");


static int leyeIdx[] = {30, 31, 32, 33, 34, 35, 36, 37, 38};
static int reyeIdx[] = {39, 40, 41, 42, 43, 44, 45, 46, 47};
static int noseIdx[] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58};
static int mouthIdx[] ={59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};

static int * metaIndex[] = {leyeIdx, reyeIdx, noseIdx, mouthIdx};
static int indexSizes[] = {9, 9, 11, 18};

void componentBoundingBox(QList<QPointF> & points, int * index, int indexLength, CRect & bbox, int width, int height)
{
    bbox.left = width - 1;
    bbox.right = 0;
    bbox.bottom = height - 1;
    bbox.top = 0;
    int offset = 2;
    for (int i=0;i < indexLength; i++)
    {
        int idx = index[i] + offset;
        int pts_size = points.size();

        if (idx > pts_size)
        {
            std::ostringstream exOut;
            exOut << "idx " << idx << " vs. points size " << pts_size;
            throw exOut.str();
        }
        int xVal = points[idx].x();
        int yVal = points[idx].y();

         if (xVal < bbox.left)
             bbox.left = xVal;
         if (xVal > bbox.right)
             bbox.right = xVal;
         if (yVal < bbox.bottom)
             bbox.bottom = yVal;
         if (yVal > bbox.top)
             bbox.top = yVal;
    }

    std::ostringstream bOut;
    bOut << "bbox dimensions " << bbox.left << ',' << bbox.bottom << ',' << bbox.right << "," << bbox.top;

    ::OutputDebugStringA(bOut.str().c_str());
    //TRACE("bbox dimensions %d %d %d %d", bbox.left, bbox.bottom, bbox.right, bbox.top);
}
static int cnt=0;
void templateFromCImage(CImage & input, br::Template & output)
{
    int imHeight = input.GetHeight();
    int imWidth = input.GetWidth();
    int channel_bits = input.GetBPP();
    int channels = channel_bits / 8;

    
    int type = CV_MAKETYPE(CV_8U, channels);
    // Should be possible to create the mat directly from the CImage's bitmap, but really is it worth dealing with?
    cv::Mat img(imHeight, imWidth, type);

    for (int i=0;i < img.rows; i++)
    {
        for (int j=0; j < img.cols;j++)
        {

            img.at<cv::Vec3b>(i,j)[0] = GetBValue(input.GetPixel(j,i));
            img.at<cv::Vec3b>(i,j)[1] = GetGValue(input.GetPixel(j,i));
            img.at<cv::Vec3b>(i,j)[2]= GetRValue(input.GetPixel(j,i));


        }
    }

    output.clear();
    output.append(img);
    std::ostringstream nameStream;
    nameStream << "test_" << cnt++ << ".jpg";
    output.file.name = nameStream.str().c_str();
}

void brInterface::pointCorrespondence(CImage & src, CImage & dst, std::vector<CRect>  & srcRegions, std::vector<CRect> & dstRegions)
{
    if (keyPointDetector == NULL)
        this->keyPointDetector = Transform::make("SaveMat(orig)+Cvt(Gray)+Cascade(FrontalFace)+ASEFEyes+Stasm(pinEyes=[First_Eye, Second_Eye])+RestoreMat(orig)+Draw(inPlace=true)", NULL);
    srcRegions.clear();
    dstRegions.clear();

    Template tSrc;
    Template tDst;

    // Build templatles from the CImages
    templateFromCImage(src, tSrc);
    templateFromCImage(dst, tDst);

    TemplateList tList;
    tList.append(tSrc);
    tList.append(tDst);

    // Detect keypoints in both images
    keyPointDetector->project(tList, tList);
    
    Gallery * out = Gallery::make("e:/Charles/");
    Gallery * metaOut = Gallery::make("e:/Charles/test_output.txt");

    out->writeBlock(tList);
    metaOut->writeBlock(tList);

    delete out;
    delete metaOut;


    QList<QPointF> srcPoints = tList[0].file.points();
    QList<QPointF> dstPoints = tList[1].file.points();

    for (int i=0; i < 4; i++)
    {
        CRect srcBox;
        CRect dstBox;

        componentBoundingBox(srcPoints, metaIndex[i], indexSizes[i],srcBox, src.GetWidth(), src.GetHeight());
        componentBoundingBox(dstPoints, metaIndex[i], indexSizes[i],dstBox, dst.GetWidth(), dst.GetHeight());


        CPoint dstCenter = dstBox.CenterPoint();
        CPoint srcCenter = srcBox.CenterPoint();

        dstBox = srcBox - srcCenter;
        dstBox = dstBox + dstCenter;

        dstBox.NormalizeRect();
        std::ostringstream bOut;
        bOut << "\toutput bbox dimensions " << dstBox.left << ',' << dstBox.bottom << ',' << dstBox.right << "," << dstBox.top << std::endl;

        ::OutputDebugStringA(bOut.str().c_str());

        dstRegions.push_back(dstBox);
        // We use the size of the bounding box in the source image, and just align its center with the center of the bounding box in the destination image
        srcBox.NormalizeRect();
        srcRegions.push_back(srcBox);

    }
    
 
    std::ostringstream finalout;
    finalout << "src boxes size " << srcRegions.size() << " dst boxes size " << dstRegions.size() << std::endl;
    OutputDebugStringA(finalout.str().c_str());
}

void clone(CImage & src, CImage & dst, std::vector<CRect> & srcRegions, std::vector<CRect> & dstRegions)
{
    Template tSrc;
    templateFromCImage(src, tSrc);
    Template tDst;
    templateFromCImage(dst, tDst);

    cv::Mat srcIm = tSrc.m();
    cv::Mat dstIm = tDst.m();

    std::vector<cv::Mat> srcChannels;
    std::vector<cv::Mat> dstChannels;

    cv::split(srcIm, srcChannels);
    cv::split(dstIm, dstChannels);

    for (int channel=0; channel < srcChannels.size(); channel++)
    {
        // Compute the laplacian of the source image
        cv::Mat laplacian;
        cv::Mat dstIm = dstChannels[channel];
        cv::Laplacian(srcChannels[channel], laplacian, CV_32F);

        for (int patchIdx=0; patchIdx < srcRegions.size() ;patchIdx++)
        {
            int nPixels = srcRegions[patchIdx].Width() * srcRegions[patchIdx].Height();

            // Set up A and b for the current region
            cpt::Matrix<double,2> A(nPixels, nPixels);;
            cpt::Matrix<double,2> b(nPixels, 1);
            cpt::Matrix<double,2> x(nPixels, 1);

            int src_x = srcRegions[patchIdx].left;
            int src_y = srcRegions[patchIdx].bottom;

            CPoint srcOrigin = srcRegions[patchIdx].TopLeft();
            CPoint dstOrigin = dstRegions[patchIdx].TopLeft();

            for (int i = 0; i < srcRegions[patchIdx].Width() ;i++)
            {
                for (int j = 0; j < srcRegions[patchIdx].Height(); j++)
                {
                    // Index of the current point, in a row-major ordering 
                    int idx = i + j * srcRegions[patchIdx].Width();

                    // discrete laplacian:
                    // 0  1  0
                    // 1 -4  1
                    // 0  1  0
                    A(idx, idx) = -4;
                    // retrieve the value of the laplacian for this pixel
                    b(idx,1) = laplacian.at<float>(i + srcOrigin.x, j + srcOrigin.y);
                    // i + 1
                    if ((i + 1) < srcRegions[patchIdx].Width())
                        A(idx, i + 1 + j * srcRegions[patchIdx].Width());
                    else
                        b(idx,1) -= dstIm.at<unsigned char> (i+1 + dstOrigin.x, j + dstOrigin.y);
                    // i - 1
                    if ((i-1) >= 0)
                        A(idx, i - 1 + j * srcRegions[patchIdx].Width() );
                    else
                        b(idx,1) -= dstIm.at<unsigned char> (i-1 + dstOrigin.x, j + dstOrigin.y);
                    // j + 1
                    if ((j+1) < srcRegions[patchIdx].Height())
                        A(idx, i + (j+1) * srcRegions[patchIdx].Width() );
                    else
                        b(idx,1) -= dstIm.at<unsigned char>(i + dstOrigin.x,j+1 + dstOrigin.y);
                    // j - 1
                    if ((j-1) >= 0)
                        A(idx, i + (j-1) * srcRegions[patchIdx].Width() );
                    else
                        b(idx,1) -= dstIm.at<unsigned char>(i + dstOrigin.x, j-1 + dstOrigin.y);

                }
            }

        
            // Solve for x (new values of the pixels in the current channel of the dst image)
            solveEquations(A, b, x);

            // Copy the new values back into dst
            for (int i=dstRegions[patchIdx].top; i >= dstRegions[patchIdx].bottom; i--)
            {
                for (int j=dstRegions[patchIdx].left; j < dstRegions[patchIdx].right; j++)
                {
                    int idx = i + j * srcRegions[patchIdx].Width();

                    double val = x(idx,1);
                    COLORREF current = dst.GetPixel(j,i);
                    int bgr[3];
                    bgr[2] = GetRValue(current);
                    bgr[1] = GetGValue(current);
                    bgr[0] = GetBValue(current);
                    bgr[channel] = val;
                    dst.SetPixel(j,i, RGB(bgr[2], bgr[1], bgr[0]));
                }
            }// dstRegions[patchIdx]
        } // patches
    } // channels

}

brInterface::brInterface()
{
    brInterface::init();

    keyPointDetector = NULL;
}

static int argc=3;
static char * arg1 = "-useGui";
static char * arg2 = "0";
static char * arg0 = "dummy";
static char * argv[] = {arg0, arg1, arg2};

void brInterface::init()
{
    br_initialize(argc,argv, "E:/Charles/872_project/CSE872Project/Project/openbr");
}

void brInterface::destruct()
{
    br_finalize();
}