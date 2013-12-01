#include "stdafx.h"

#include "br_interface.h"

// A min macro is clearly helpful, and not just obnoxious
#undef min

#include <openbr/openbr.h>

//#undef min
#include <openbr/openbr_plugin.h>

#include <opencv2/core/types_c.h>

using namespace br;

void templateFromCImage(CImage & input, br::Template & output)
{
    // Calling detach gives us the HBITMAP instead of the iplimage
 //   HBITMAP hbmp = input.Detach();

    // Using getobject, we can retrieve the bitmap
  //  BITMAP bmp;
  //  GetObject(hbmp,sizeof(BITMAP),&bmp);

  //  int    nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel/8;

    //input.
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
    Template out2;

    output.file.name = "test.jpg";

    Transform * tform = Transform::make("SaveMat(orig)+Cvt(Gray)+Cascade(FrontalFace)+RestoreMat(orig)+Draw(inPlace=true)", NULL);

    tform->project(output,out2);

    Gallery * out = Gallery::make("e:/Charles/");

    out->write(out2);
    
    int hello=0;
    out2.clear();

    hello++;

    delete tform;
    delete out;
}


void detectPoints(CImage & img, CRect & region)
{
    br::Template in;
    templateFromCImage(img, in);

}


void init()
{
    int argc=0;
    br_initialize(argc,NULL, "E:/Charles/872_project/CSE872Project/Project/openbr");
}

void destruct()
{
    br_finalize();
}