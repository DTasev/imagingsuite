//<LICENSE>

#include "profileextractor.h"
#include <filters/filter.h>
#include <math/statistics.h>

#include <QDebug>
#include <filters/filter.h>
#include <io/io_tiff.h>
#include <base/tprofile.h>
#include <math/mathconstants.h>
#include <math/findpeaks.h>

namespace ImagingQAAlgorithms {
ProfileExtractor::ProfileExtractor() :
    mPrecision(1.0f)
{
    std::fill_n(lineCoeffs,2,0.0f);
}

void ProfileExtractor::setPrecision(float p)
{
    mPrecision=fabs(p);
}

float ProfileExtractor::precision()
{
    return mPrecision;
}

std::map<float, float> ProfileExtractor::getProfile(kipl::base::TImage<float, 2> &img,size_t *roi)
{
    (void)roi;
    std::map<float, kipl::math::Statistics> profile_stats;
    profile_stats.clear();

    auto de=diffEdge(img);

    computeEdgeEquation(de);

    kipl::io::WriteTIFF32(de,"PE_diffEdge.tif");
    int nx=static_cast<int>(de.Size(0));
    int ny=static_cast<int>(de.Size(1));

    for (int y=0; y<ny; ++y)
    {
        for (int x=0; x<nx; ++x)
        {
            float dist=distanceToLine(x,y);
            if (profile_stats.find(dist)!=profile_stats.end())
                profile_stats[dist].put(img(x,y));
            else {
                kipl::math::Statistics s;
                s.put(img(x,y));
                profile_stats.insert(std::make_pair(dist,s));
            }
        }
    }

    std::map<float, float> profile;
    for (auto it=profile_stats.begin(); it!=profile_stats.end(); ++it)
        profile.insert(std::make_pair(it->first,static_cast<float>(it->second.E())));

    return profile;
}

void ProfileExtractor::computeEdgeEquation(kipl::base::TImage<float, 2> &img)
{

    float *rawProfile=new float[img.Size(0)];

    float x=0.0f;
    float x2=0.0f;
    float y=0.0f;
    float y2=0.0f;
    float xy=0.0f;

    size_t Nx=img.Size(0);
    size_t Ny=img.Size(1);
    float cnt=0.0f;
    for (size_t yy=1; yy<Ny; ++yy)
    {
        float *pLine=img.GetLinePtr(yy);

        float maxpos=kipl::math::findPeakCOG(pLine,Nx,true,false);
        x  += maxpos;
        x2 += maxpos*maxpos;
        y  += yy;
        y2 += yy*yy;
        xy += maxpos*yy;
        cnt++;
    }

    lineCoeffs[0]=(x*y2-y*xy)/(cnt*y2-y*y);
    lineCoeffs[1]=-(cnt*xy-x*y)/(cnt*y2-y*y);

    qDebug() << "m="<<lineCoeffs[0]<<"; k="<<lineCoeffs[1]<<", atan(k)"<<atan(lineCoeffs[1])*180/fPi;

    delete [] rawProfile;
}

kipl::base::TImage<float, 2> ProfileExtractor::diffEdge(kipl::base::TImage<float, 2> &img)
{

    float kx[9]={-3,0,3,
                -10,0,10,
                -3,0,3};
    size_t dims[2]={3,3};
    kipl::filters::TFilter<float,2> dx(kx,dims);

    float ky[9]={-3,-10,-3,
                0,0,0,
                3,10,3};

    kipl::filters::TFilter<float,2> dy(ky,dims);

    auto dximg=dx(img,kipl::filters::FilterBase::EdgeMirror);
    auto dyimg=dy(img,kipl::filters::FilterBase::EdgeMirror);

    kipl::base::TImage<float,2> absgrad(img.Dims());

    float *pA=absgrad.GetDataPtr();
    float *pX=dximg.GetDataPtr();
    float *pY=dyimg.GetDataPtr();

    for (size_t i=0; i<img.Size(); ++i,++pA,++pX,++pY)
    {
        *pA=sqrt((*pX)*(*pX)+(*pY)*(*pY));
    }

    return absgrad;

}

float ProfileExtractor::distanceToLine(int x, int y)
{
  float d=(x*lineCoeffs[0]-y+lineCoeffs[1])/sqrt(lineCoeffs[0]*lineCoeffs[0]+1.0f);

  d = floor(d/mPrecision)*mPrecision;

  return d;
}

}
