//<LICENCE>

#include "../../../include/kipl_global.h"

#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <map>

#include "../../../include/math/sums.h"
#include "../../../include/base/thistogram.h"
#ifndef NO_TIFF
#include "../../../include/io/io_tiff.h"
#endif

namespace kipl { namespace base {
//int Histogram(float const * const data, size_t nData, size_t * const hist, const size_t nBins, float lo, float hi, float * const pAxis)

int KIPLSHARED_EXPORT Histogram(float * data, size_t nData, size_t * hist, size_t nBins, float lo, float hi, float * pAxis)
{

    float start=0;
    float stop=1.0f;
   
    if (lo==hi) {
        start = *std::min_element(data,data+nData);
        stop  = *std::max_element(data,data+nData);
    }
    else {
		start=std::min(lo,hi);
		stop=std::max(lo,hi);
    }

    memset(hist,0,sizeof(size_t)*nBins);
    
	float scale=(nBins)/(stop-start);
    #pragma omp parallel firstprivate(nData,start,scale)
    {
		int index;
		long long int i=0;	
		size_t *temp_hist=new size_t[nBins];
		memset(temp_hist,0,nBins*sizeof(size_t));
		const ptrdiff_t snData=static_cast<ptrdiff_t>(nData);
		const ptrdiff_t snBins=static_cast<ptrdiff_t>(nBins);

		#pragma omp for
		for (i=0; i<snData; i++) {
			index=static_cast<int>((data[i]-start)*scale);
			if ((index<snBins) && (0<=index))
				temp_hist[index]++;
		}
		#pragma omp critical
		{
			for (i=0; i<snBins; i++)
				hist[i]+=temp_hist[i];
		}
		delete [] temp_hist;
    }
    scale=(stop-start)/nBins;
    if (pAxis!=nullptr) {
        pAxis[0]=start+scale/2;
        for (size_t i=1; i<nBins; i++)
            pAxis[i]=pAxis[i-1]+scale;
    }

	return 0;
}

std::map<float, size_t> ExactHistogram(float const * const data, size_t Ndata)
{
	std::map<float, size_t> hist;

	for (size_t i=0; i<Ndata; i++) {
		hist[data[i]]++;
	}

	return hist;
}


double  KIPLSHARED_EXPORT Entropy(size_t const * const hist, size_t N)
{
	double p=0.0;
	double entropy=0.0;

	size_t histsum=kipl::math::sum(hist,N);

	for (size_t i=0; i<N; i++) {
		if (hist[i]!=0) {
			p=static_cast<double>(hist[i])/static_cast<double>(histsum);
			entropy-=p*std::log10(p);
		}
	}

	return entropy;
}

int  KIPLSHARED_EXPORT FindLimits(size_t const * const hist, size_t N, float percentage, size_t * lo, size_t * hi)
{
	ptrdiff_t *cumulated=new ptrdiff_t[N];
	memset(cumulated,0,sizeof(ptrdiff_t)*N);

	cumulated[0]=hist[0];
	for (size_t i=1; i<N; i++) {
		cumulated[i]=cumulated[i-1]+hist[i];
	}

    if ((lo!=nullptr) && (hi!=nullptr)) {
		*lo=0;
		*hi=0;

		float fraction=(100.0f-percentage)/200.0f;
		ptrdiff_t lowlevel  = static_cast<ptrdiff_t>(cumulated[N-1]*fraction);
		ptrdiff_t highlevel = static_cast<ptrdiff_t>(cumulated[N-1]*(1-fraction));

		ptrdiff_t lowdiff  = cumulated[N-1];
		ptrdiff_t highdiff = cumulated[N-1];

		ptrdiff_t diff=cumulated[N-1];
		for (size_t i=0; i<N; i++){
			diff=static_cast<ptrdiff_t>(std::abs(static_cast<double>(cumulated[i]-lowlevel)));
			if (diff<lowdiff) {
				lowdiff=diff;
				*lo=i;
			}

			diff=static_cast<ptrdiff_t>(std::abs(static_cast<double>(cumulated[i]-highlevel)));
			if (diff<highdiff) {
				highdiff=diff;
				*hi=i;
			}
		}
	}
	return 0;
}
//------------------------------------------------------------------
// Bivariate histogram class
BivariateHistogram::BivariateHistogram() :
    logger("BivariateHistogram"),
    m_scalingA(1.0f,0.0f),
    m_scalingB(1.0f,0.0f),
    m_limitsA(0.0f,1.0f),
    m_limitsB(0.0f,1.0f)
{

}

BivariateHistogram::~BivariateHistogram()
{

}

/// \brief Initialize the histogram using limits
/// \param loA smallest accepted value for data set A
/// \param hiA greatest accepted value for data set A
/// \param binsA number of bins for data set A
/// \param loB smallest accepted value for data set B
/// \param hiB greatest accepted value for data set B
/// \param binsB number of bins for data set B
void BivariateHistogram::Initialize(float loA, float hiA, size_t binsA,
                float loB, float hiB, size_t binsB)
{
    m_limitsA.first   = min(loA,hiA);
    m_limitsA.second  = max(loA,hiA);

    m_limitsB.first   = min(loB,hiB);
    m_limitsB.second  = max(loB,hiB);

    m_nbins.first     = binsA;
    m_nbins.second    = binsB;

    m_scalingA.first  = static_cast<float>(m_nbins.first) / (m_limitsA.second-m_limitsA.first);
    m_scalingA.second = m_limitsA.first;

    m_scalingB.first  = static_cast<float>(m_nbins.second) / (m_limitsB.second-m_limitsB.first);
    m_scalingB.second = m_limitsB.first;

    size_t dims[2]={m_nbins.first,m_nbins.second};
    m_bins.Resize(dims);
}

/// \brief Initialize the histogram using data
/// \param pA reference to data set A
/// \param binsA number of bins for data set A
/// \param pB reference to data set B
/// \param binsB number of bins for data set B
/// \param N number of data elements in the data sets
void BivariateHistogram::Initialize(float *pA, size_t binsA, float *pB, size_t binsB, size_t N)
{
    m_limitsA.first = *std::min_element(pA,pA+N);
    m_limitsA.second = *std::max_element(pA,pA+N);

    m_limitsB.first = *std::min_element(pB,pB+N);
    m_limitsB.second = *std::max_element(pB,pB+N);

    Initialize(m_limitsA.first,m_limitsA.second, binsA,
               m_limitsB.first,m_limitsB.second, binsB);
}

/// \brief Add single data pair to the histogram
/// \param a value from data set A
/// \param b value from data set B
void BivariateHistogram::AddData(float a, float b)
{
    m_bins(ComputePos(a,m_scalingA,m_limitsA,m_nbins.first),
           ComputePos(b,m_scalingB,m_limitsB,m_nbins.second))++;
}

/// \brief Add single data pair to the histogram
/// \param a pointer to data set A
/// \param b pointer to data set B
/// \param N number of data points
void BivariateHistogram::AddData(float *a, float *b, size_t N)
{
    for (size_t i=0; i<N; i++) {
        m_bins(ComputePos(a[i],m_scalingA,m_limitsA,m_nbins.first),
               ComputePos(b[i],m_scalingB,m_limitsB,m_nbins.second))++;
    }
}

/// \brief Get counts at the bin closest to the coordinates
/// \param a Coordinate in data set A
/// \param b Coordinate in data set B
BivariateHistogram::BinInfo BivariateHistogram::GetBin(float a, float b)
{
    int posA=ComputePos(a,m_scalingA,m_limitsA,m_nbins.first);
    int posB=ComputePos(b,m_scalingB,m_limitsB,m_nbins.second);


    return BinInfo(m_bins(posA,posB),
                   posA/m_scalingA.first+m_scalingA.second+m_scalingA.first*0.5f,
                   posB/m_scalingB.first+m_scalingB.second+m_scalingB.first*0.5f,
                   posA,posB);

}

/// \brief Get counts at the bin closest to the coordinates
/// \param a Coordinate in data set A
/// \param b Coordinate in data set B
BivariateHistogram::BinInfo BivariateHistogram::GetBin(int posA, int posB)
{
    return BinInfo(m_bins(posA,posB),
                   posA/m_scalingA.first+m_scalingA.second+m_scalingA.first*0.5f,
                   posB/m_scalingB.first+m_scalingB.second+m_scalingB.first*0.5f,
                   posA,posB);
}

/// \brief Get axis ticks for data set A
/// \returns the pointer to the axis ticks
const float * BivariateHistogram::GetAxisA()
{
    return nullptr;
}

/// \brief Get axis ticks for data set B
/// \returns the pointer to the axis ticks
float const * BivariateHistogram::GetAxisB()
{
    return nullptr;
}

kipl::base::TImage<size_t,2> & BivariateHistogram::Bins()
{
    return m_bins;
}

size_t const *   BivariateHistogram::Dims()
{
    return m_bins.Dims();
}

/// \brief Compute index to a histogram bin. This is the generic version.
/// \param x
/// \param scaling
/// \param limits
/// \param nBins
inline int BivariateHistogram::ComputePos(float x, std::pair<float, float> &scaling, std::pair<float, float> &limits, size_t nBins)
{
    int selector=(x<limits.first)+2*(limits.second<=x);

    switch (selector) {
        case 0: return static_cast<int>((x-scaling.second)*scaling.first); break;
        case 1: return 0;
        case 2: return nBins-1;
        sdefault: logger(logger.LogWarning, "Strange selector value in ComputePos"); break;
    }

    return 0;
}

std::pair<float,float> BivariateHistogram::GetLimits(int n)
{
    switch (n) {
    case 0: return m_limitsA; break;
    case 1: return m_limitsB; break;
    default : throw kipl::base::KiplException("No existing axis selected in GetLimits",__FILE__,__LINE__);
    }

    return m_limitsA;
}

std::map<float, map<float,size_t> > BivariateHistogram::CompressedHistogram(kipl::base::eImageAxes mainaxis, size_t threshold)
{
    std::map<float, std::map<float,size_t> > hist;

    size_t *pLine;
    switch (mainaxis) {
    case kipl::base::ImageAxisX:
        for (size_t i=0; i<m_bins.Size(1); i++) {
            pLine=m_bins.GetLinePtr(i);
            // todo: fix the code
//            std::map<float,size_t> listline;
//            for (size_t j=0; j<m_Bins.Size(0); j++)
//            {
//                if (threshold<=pLine[j])
//                {
//                 //   listline.insert(make_pair())
//                }
//            }
//            hist.insert(make_pair(binval,listline));
        }

        break;
    case kipl::base::ImageAxisY:
    case kipl::base::ImageAxisZ:
    default:
        throw kipl::base::KiplException("Unsupported axis was selected in compressedhistogram.",__FILE__,__LINE__);
        break;
    }

    return hist;
}

void BivariateHistogram::Write(string fname)
{
#ifndef NO_TIFF
    kipl::base::TImage<float,2> img(m_bins.Dims());

    float cnt=static_cast<float>(kipl::math::sum(m_bins.GetDataPtr(),m_bins.Size()));

    for (size_t i=0; i<m_bins.Size(); i++) {
        img[i]=static_cast<float>(m_bins[i]);
    }

    kipl::io::WriteTIFF32(img,fname.c_str());
#endif
}

}}
