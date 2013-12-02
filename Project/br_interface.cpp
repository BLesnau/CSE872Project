#include "stdafx.h"

#include "br_interface.h"

// A min macro is clearly helpful, and not just obnoxious
#undef min

#include <openbr/openbr.h>
#include <openbr/openbr_plugin.h>

#include <opencv2/core/types_c.h>

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
static int count=0;
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
    nameStream << "test_" << count++ << ".jpg";
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