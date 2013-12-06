#pragma once

#include <atlimage.h>
#include <vector>
#include "qsharedpointer.h"

namespace br
{
    class Transform;
}

class brInterface
{
public:
    brInterface();

    static void init();

    static void destruct();

    QSharedPointer<br::Transform> keyPointDetector;

    
    void pointCorrespondence(CImage & src, CImage & dst, std::vector<CRect> & srcRegions, std::vector<CRect> & dstRegions);

//    void clone(CImage & src, CImage & dst, std::vector<CRect> & srcRegions, std::vector<CRect> & dstRegions);

};

void clone(CImage & src, CImage & dst, std::vector<CRect> & srcRegions, std::vector<CRect> & dstRegions);
void clone2(CImage & src, CImage & dst, std::vector<CRect> & srcRegions, std::vector<CRect> & dstRegions);


