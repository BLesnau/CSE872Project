#include "stdafx.h"

#include "br_interface.h"

// A min macro is clearly helpful, and not just obnoxious
#undef min

#include <openbr/openbr.h>
#include <openbr/openbr_plugin.h>

#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Poisson_Multigrid\poisson_multigrid.h"

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/IterativeLinearSolvers>

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

void boundingSquare(CRect & boundingBox)
{
    // find the major dimension of the bounding box
    double major = boundingBox.Width() > boundingBox.Height() ? boundingBox.Width() : boundingBox.Height();

    // log base 2 of the major dimension
    float two_power = log(major) / log(2.0);

    int majorPower = std::ceil(two_power);

    int side_size = pow(2.0, majorPower) + 2;

    // We expand the bounding box to side_size x side_size while maintaining the same center

    CPoint center = boundingBox.CenterPoint();

    // Difference between the current and desired dimensions
    int width_delta = side_size - boundingBox.Width();
    int height_delta = side_size - boundingBox.Height();

    int half_width = width_delta / 2;
    int half_height= height_delta / 2;

    // Adjust the origin
    CPoint initialOrigin = boundingBox.TopLeft();
    boundingBox.top = initialOrigin.y - half_height;
    boundingBox.left= initialOrigin.x - half_width;

    // And the far sides of the bounding box
    boundingBox.bottom = boundingBox.top + side_size;
    boundingBox.right = boundingBox.left + side_size;

}

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
    bOut << "bbox dimensions: " << bbox.left << ',' << bbox.bottom << ',' << bbox.right << "," << bbox.top << std::endl;;

    ::OutputDebugStringA(bOut.str().c_str());
    boundingSquare(bbox);

    std::ostringstream b2;
    b2 << "Squared bbox dimensions: " << bbox.left << ',' << bbox.bottom << ',' << bbox.right << "," << bbox.top << std::endl << std::endl;
    ::OutputDebugStringA(b2.str().c_str());
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
    
    Gallery * out = Gallery::make("./");
    Gallery * metaOut = Gallery::make("./test_output.txt");

    out->writeBlock(tList);
    metaOut->writeBlock(tList);

    delete out;
    delete metaOut;


    QList<QPointF> srcPoints = tList[0].file.points();
    QList<QPointF> dstPoints = tList[1].file.points();

    for (int i=0; i < 4; i++)
    {
        // skipping the nose
        if (i==2)
            continue;

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

void clone2(CImage & src, CImage & dst, std::vector<CRect> & srcRegions, std::vector<CRect> & dstRegions)
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
        cv::Mat srcIm = srcChannels[channel];

        laplacian.create(srcIm.rows, srcIm.cols, CV_64FC1);

        for(int i=1; i < srcIm.rows-1; i++)
        {
            for (int j=1; j < srcIm.cols-1;j++)
            {
                // Discrete laplacian:
                // 0  1  0
                // 1 -4  1
                // 0  1  0
                double value = -4.0 * double(srcIm.at<unsigned char>(i,j));
                value += double(srcIm.at<unsigned char>(i+1,j));
                value += double(srcIm.at<unsigned char>(i-1,j));
                value += double(srcIm.at<unsigned char>(i,j+1));
                value += double(srcIm.at<unsigned char>(i,j-1));
                laplacian.at<double>(i,j) = value;
            }
        }

        //cv::Laplacian(srcChannels[channel], laplacian, CV_64F);

        for (int patchIdx=0; patchIdx < srcRegions.size(); patchIdx++)
        {
            int nPixels = srcRegions[patchIdx].Width() * srcRegions[patchIdx].Height();

            // Construct a problem of  the form Ax = b
            // Here, A is nPixels x nPixels and encodes the laplacian
            // x is an nPixels long vector of unknowns (the new values of the dst image that
            // we will solve for), and b is the target values (laplacian of the src image, adjusted
            // at the boundaries). We encode the pixels in row-major order
            
            // A is a sparse matrix (discrete laplacian only touches self, and 4 adjacent pixels)
            Eigen::SparseMatrix<double> A(nPixels, nPixels);
            Eigen::VectorXd b(nPixels);
            Eigen::VectorXd x(nPixels);
            
            CPoint dstOrigin = dstRegions[patchIdx].TopLeft();
            CPoint srcOrigin = srcRegions[patchIdx].TopLeft();

            for (int i=0; i < srcRegions[patchIdx].Height(); i++)
            {
                for (int j=0; j < srcRegions[patchIdx].Width();j++)
                {
                    int idx = j + i * srcRegions[patchIdx].Width();

                    // b values are the laplacian of the src image (adjusted at boundaries)
                    b(idx) = laplacian.at<double>(i + srcOrigin.y, j + srcOrigin.x);

                    // the main diagonal is -4
                    A.insert(idx,idx) = -4.0;

                    // i + 1
                    if ((i + 1) < srcRegions[patchIdx].Height())
                        A.insert(idx, j + (i+1) * srcRegions[patchIdx].Width()) = 1;
                    else
                        b(idx) = b(idx) - dstIm.at<unsigned char>(i + 1 + dstOrigin.y, j + dstOrigin.x);

                    // i - 1
                    if ((i-1) >= 0)
                        A.insert(idx, j + (i-1) * srcRegions[patchIdx].Width()) = 1;
                    else
                        b(idx) = b(idx) - dstIm.at<unsigned char>(i -1 + dstOrigin.y, j + dstOrigin.x);

                    // j + 1
                    if ((j + 1) < srcRegions[patchIdx].Width())
                        A.insert(idx, j + 1 + i * srcRegions[patchIdx].Width()) = 1;
                    else
                        b(idx) = b(idx) - dstIm.at<unsigned char>(i + dstOrigin.y, j + 1 + dstOrigin.x);

                    
                    // j - 1
                    if (j - 1 >= 0)
                        A.insert(idx, j - 1 + i * srcRegions[patchIdx].Width()) = 1;
                    else
                        b(idx) = b(idx) - dstIm.at<unsigned char>(i + dstOrigin.y, j - 1 + dstOrigin.x);

                }
            }

            // Setup solver. A is symmetric, positive definite so we have quite a few options for
            // solvers
            //Eigen::ConjugateGradient<Eigen::SparseMatrix<double> > solver;
            //Eigen::BiCGSTAB<Eigen::SparseMatrix<double> > solver;
            Eigen::SimplicialCholesky<Eigen::SparseMatrix<double> > solver; solver.setMode(Eigen::SimplicialCholeskyLDLT);
            //Eigen::SimplicialCholesky<Eigen::SparseMatrix<double> > solver; solver.setMode(Eigen::SimplicialCholeskyLLT);

            solver.compute(A);
            if(solver.info()!=Eigen::Success) {
              // decomposition failed
                OutputDebugStringA("decomposition failed!");
              return;
            }

            // actually solve Ax=b
            x = solver.solve(b);
            if(solver.info()!=Eigen::Success) {
              // solving failed
                OutputDebugStringA("optimization failed!");
                continue;
            }

            
            // Copy the new values back into dst
            for (int i=0; i < dstRegions[patchIdx].Height(); i++)
            {
                for (int j=0; j < dstRegions[patchIdx].Width(); j++)
                {
                    int idx = j + i * dstRegions[patchIdx].Width();

                    COLORREF current = dst.GetPixel(j + dstOrigin.x, i + dstOrigin.y);
                    int bgr[3];
                    bgr[2] = GetRValue(current);
                    bgr[1] = GetGValue(current);
                    bgr[0] = GetBValue(current);
                    double val = x(idx);
                    if (val < 0)
                        val = 0;
                    if (val >= 255)
                        val = 255;
                    bgr[channel] = val;
                    dst.SetPixel(j + dstOrigin.x,i + dstOrigin.y, RGB(bgr[2], bgr[1], bgr[0]));
                }
            }// dstRegions[patchIdx]
        } // patches
    } // channels

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
        cv::Mat srcIm = srcChannels[channel];

        laplacian.create(srcIm.rows, srcIm.cols, CV_64FC1);

        for(int i=1; i < srcIm.rows-1; i++)
        {
            for (int j=1; j < srcIm.cols-1;j++)
            {
                // Discrete laplacian:
                // 0  1  0
                // 1 -4  1
                // 0  1  0
                double value = -4.0 * srcIm.at<unsigned char>(i,j);
                value += double(srcIm.at<unsigned char>(i+1,j));
                value += double(srcIm.at<unsigned char>(i-1,j));
                value += double(srcIm.at<unsigned char>(i,j+1));
                value += double(srcIm.at<unsigned char>(i,j-1));
                laplacian.at<double>(i,j) = value;
            }
        }

        //cv::Laplacian(srcChannels[channel], laplacian, CV_32F);

        for (int patchIdx=0; patchIdx < srcRegions.size(); patchIdx++)
        {
            int nPixels = srcRegions[patchIdx].Width() * srcRegions[patchIdx].Height();




            // Set up A and b for the current region
            cpt::Matrix<double,2> laplacianPatch(srcRegions[patchIdx].Width(), srcRegions[patchIdx].Height());
            cpt::Matrix<double,2> destinationPatch(dstRegions[patchIdx].Width(), dstRegions[patchIdx].Height());

            CPoint dstOrigin = dstRegions[patchIdx].TopLeft();
            CPoint srcOrigin = srcRegions[patchIdx].TopLeft();
            for (int i = 0; i < srcRegions[patchIdx].Height() ;i++)
            {
                for (int j = 0; j < srcRegions[patchIdx].Width(); j++)
                {
                    laplacianPatch(j,i) = laplacian.at<double>(i,j);
                    destinationPatch(j,i) = dstIm.at<unsigned char> (i + dstOrigin.y,j + dstOrigin.x);
                    // direct clone
                    //destinationPatch(j,i) = srcIm.at<unsigned char> (i + srcOrigin.y,j + srcOrigin.x);
                    srcIm.at<unsigned char> (i + srcOrigin.y,j + srcOrigin.x) = 255;
                }
            }


            cv::imwrite("channel_highlight.png", srcIm);
            cv::imwrite("channel_laplacian.png", laplacian);
            solveMultigrid(laplacianPatch, destinationPatch);

            // Copy the new values back into dst
            for (int i=0; i < destinationPatch.dim2(); i++)
            {
                for (int j=0; j < destinationPatch.dim1(); j++)
                {
                    COLORREF current = dst.GetPixel(j + dstOrigin.x, i + dstOrigin.y);
                    int bgr[3];
                    bgr[2] = GetRValue(current);
                    bgr[1] = GetGValue(current);
                    bgr[0] = GetBValue(current);
                    bgr[channel] = destinationPatch(j,i);
                    dst.SetPixel(j + dstOrigin.x,i + dstOrigin.y, RGB(bgr[2], bgr[1], bgr[0]));
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
    // get the location of the .exe
    char szDirectory[MAX_PATH]="";
    GetModuleFileNameA(NULL, szDirectory, MAX_PATH);
    std::string path = szDirectory;
    int slash_idx = path.rfind("\\");
    // remove application name/first slash
    path = path.substr(0, slash_idx);
    // remove build type
    slash_idx = path.rfind("\\");
    path = path.substr(0, slash_idx);
    path += "/openbr/";

    ::OutputDebugStringA(path.c_str());
    br_initialize(argc,argv, path.c_str());
}

void brInterface::destruct()
{
    br_finalize();
}