//<LICENSE>

#include <sstream>
#include <map>

#include <math/median.h>
#include <filters/stddevfilter.h>

#include "../include/ImagingException.h"

#include "../include/averageimage.h"
namespace ImagingAlgorithms {

AverageImage::AverageImage() :
    WindowSize(5)
{

}

AverageImage::~AverageImage()
{

}

void AverageImage::setWindowSize(int n)
{
    WindowSize=n;
}

int AverageImage::windowSize()
{
    return WindowSize;
}

kipl::base::TImage<float,2> AverageImage::operator()(kipl::base::TImage<float,3> &img,
                                                     eAverageMethod method,
                                                     std::vector<float> weights)
{
    kipl::base::TImage<float,2> res(img.Dims());
    kipl::base::TImage<float,3> wimg;
    if (!weights.empty()) {
        wimg=WeightImages(img,weights);
    }
    else {
        wimg=img;
    }

    switch (method) {
        case ImageSum:              res=ComputeSum(wimg); break;
        case ImageAverage:          res=ComputeAverage(wimg); break;
        case ImageMedian:           res=ComputeMedian(wimg); break;
        case ImageWeightedAverage:  res=ComputeWeightedAverage(wimg); break;
        case ImageMin:              res=ComputeMin(wimg); break;
        case ImageMax:              res=ComputeMax(wimg); break;
        default : throw ImagingException("Unknown average method in AverageImage",__FILE__, __LINE__);
    }

    return res;
}

kipl::base::TImage<float,3> AverageImage::WeightImages(kipl::base::TImage<float,3> &img, std::vector<float> &weights)
{
    kipl::base::TImage<float,3> res=img;
    res.Clone();

    if (weights.size()!=img.Size(2))
        throw ImagingException("WeightImages failed as the size(weight)!=number of planes in img",__FILE__,__LINE__);

    for (size_t i=0; i<img.Size(2); i++) {
        float *pRes=res.GetLinePtr(0,i);

        for (size_t j=0; j<img.Size(0)*img.Size(1); j++) {
            pRes[j]*=weights[i];
        }

    }

    return res;
}

kipl::base::TImage<float,2> AverageImage::ComputeSum(kipl::base::TImage<float,3> &img)
{
    kipl::base::TImage<float,2> res(img.Dims());

    res=0.0f;

    for (size_t i=0; i<img.Size(2); i++) {
        float *pImg=img.GetLinePtr(0,i);
        for (size_t j=0; j<res.Size(); j++) {
            res[j]+=pImg[j];
        }
    }

    return res;
}

kipl::base::TImage<float,2> AverageImage::ComputeAverage(kipl::base::TImage<float,3> & img)
{
    kipl::base::TImage<float,2> res=ComputeSum(img);

    float q=1.0f/img.Size(2);
    for (size_t j=0; j<img.Size(0)*img.Size(1); j++) {
        res[j]*=q;
    }

    return res;
}

kipl::base::TImage<float,2> AverageImage::ComputeMedian(kipl::base::TImage<float,3> & img)
{
    kipl::base::TImage<float,2> res(img.Dims());
    size_t N=img.Size(2);
    size_t M=res.Size();
    float * buffer = new float[N];
    float *pImg=img.GetDataPtr();
    float *pRes=res.GetDataPtr();
    for (size_t i=0; i<M; i++, pImg++, pRes++) {
        GetColumn(img,i,buffer);
        kipl::math::median_STL(buffer,N,pRes);
    }
    delete [] buffer;
    return res;
}

kipl::base::TImage<float,2> AverageImage::ComputeWeightedAverage(kipl::base::TImage<float,3> & img)
{
    kipl::base::TImage<float,2> res(img.Dims());
    kipl::base::TImage<float,2> tmp(img.Dims());

    kipl::base::TImage<float,3> weights(img.Dims());
    kipl::filters::StdDevFilter stddev;

    float *pRes=nullptr;
    float *pImg=nullptr;

    const size_t N=res.Size();
    const size_t M=img.Size(2);
    // Prepare weights
    for (size_t i=0; i<img.Size(2); i++) {
        memcpy(tmp.GetDataPtr(),img.GetLinePtr(0,i),N*sizeof(float));
        res=stddev(tmp);
        memcpy(weights.GetLinePtr(0,i),res.GetDataPtr(),N*sizeof(float));
    }

    pImg=img.GetDataPtr();
    pRes=res.GetDataPtr();
    float weight=0.0f;
    float weight_sum=0.0f;

    float *wv=new float[M];
    float *iv=new float[M];

    for (size_t i=0; i<N; i++) {
        weight_sum=0.0f;
        pRes[i]=0.0f;
        GetColumn(img,i,iv);
        GetColumn(weights,i,wv);

        for (size_t j=0; j<M; j++) {
            weight=wv[j]!=0.0f ? 1.0f/wv[j] : 1.0f/N;
            weight_sum+=weight;

            pRes[i]+=weight*iv[j];
        }
        pRes[i]/=weight_sum;
    }

    delete [] wv;
    delete [] iv;

    return res;
}

kipl::base::TImage<float,2> AverageImage::ComputeMin(kipl::base::TImage<float,3> & img)
{
    kipl::base::TImage<float,2> res(img.Dims());

    const size_t N=res.Size();
    const size_t M=img.Size(2);

    memcpy(res.GetDataPtr(),img.GetDataPtr(),res.Size()*sizeof(float));

    float *pRes=res.GetDataPtr();
    for (size_t i=1; i<M; i++) {
        float *pImg=img.GetLinePtr(0,i);
        for (size_t j=0; j<N; j++) {
            pRes[j]=min(pRes[j],pImg[j]);
        }
    }

    return res;
}

kipl::base::TImage<float,2> AverageImage::ComputeMax(kipl::base::TImage<float,3> & img)
{
    kipl::base::TImage<float,2> res(img.Dims());

    memcpy(res.GetDataPtr(),img.GetDataPtr(),res.Size()*sizeof(float));

    for (size_t i=1; i<img.Size(2); i++) {
        float *pImg=img.GetLinePtr(0,i);
        for (size_t j=0; j<res.Size(); j++) {
            res[j]=max(res[j],pImg[j]);
        }
    }

    return res;
}

void AverageImage::GetColumn(kipl::base::TImage<float,3> &img, size_t idx, float *data)
{
    float *pImg=img.GetDataPtr()+idx;
    size_t N=img.Size(0)*img.Size(1);

    for (size_t i=0; i<img.Size(2); i++)
    {
        data[i]=pImg[i*N];
    }
}

}

void string2enum(std::string str, ImagingAlgorithms::AverageImage::eAverageMethod &eam)
{
    std::map<std::string,ImagingAlgorithms::AverageImage::eAverageMethod> methods;

    methods["ImageSum"]=ImagingAlgorithms::AverageImage::ImageSum;
    methods["ImageAverage"]=ImagingAlgorithms::AverageImage::ImageAverage;
    methods["ImageMedian"]=ImagingAlgorithms::AverageImage::ImageMedian;
    methods["ImageWeightedAverage"]=ImagingAlgorithms::AverageImage::ImageWeightedAverage;
    methods["ImageMin"]=ImagingAlgorithms::AverageImage::ImageMin;
    methods["ImageMax"]=ImagingAlgorithms::AverageImage::ImageMax;

    if (methods.count(str)==0)
        throw ImagingException("The key string does not exist for eAverageMethod",__FILE__,__LINE__);

    eam=methods[str];
}

std::string enum2string(ImagingAlgorithms::AverageImage::eAverageMethod eam)
{
    std::string str;

    switch (eam) {
        case ImagingAlgorithms::AverageImage::ImageSum: str="ImageSum"; break;
        case ImagingAlgorithms::AverageImage::ImageAverage: str="ImageAverage"; break;
        case ImagingAlgorithms::AverageImage::ImageMedian: str="ImageMedian"; break;
        case ImagingAlgorithms::AverageImage::ImageWeightedAverage: str="ImageWeightedAverage"; break;
        case ImagingAlgorithms::AverageImage::ImageMin: str="ImageMin"; break;
        case ImagingAlgorithms::AverageImage::ImageMax: str="ImageMax"; break;
        default: throw ImagingException("Unknown average method in enum2string for eAverageMethod",__FILE__,__LINE__);
    }
    return  str;
}

std::ostream & operator<<(ostream & s, ImagingAlgorithms::AverageImage::eAverageMethod eam)
{
    s<<enum2string(eam);

    return s;
}


#ifdef HAVEPYBIND11
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bindAverageImage(py::module &m)
{
    py::class_<ImagingAlgorithms::AverageImage> avgClass(m, "AverageImage");
    avgClass.def(py::init());
    avgClass.def("windowSize", &ImagingAlgorithms::AverageImage::windowSize,"Returns the size of the filter window used by the weighted average");
    avgClass.def("setWindowSize", &ImagingAlgorithms::AverageImage::setWindowSize,"Set the size of the filter window used by the weighted average",py::arg("size")=5);
    avgClass.def("process",
                 [](ImagingAlgorithms::AverageImage &a,
                 py::array_t<float> &x,
                 ImagingAlgorithms::AverageImage::eAverageMethod method,
                 std::vector<float> weights = {})
    {
        auto r = x.unchecked<3>(); // x must have ndim = 3; can be non-writeable

        py::buffer_info buf1 = x.request();

        size_t dims[]={static_cast<size_t>(buf1.shape[2]),
                       static_cast<size_t>(buf1.shape[1]),
                       static_cast<size_t>(buf1.shape[0])};
        kipl::base::TImage<float,3> stack(static_cast<float*>(buf1.ptr),dims);

        kipl::base::TImage<float,2> res=a(stack,method,weights);

        py::array_t<float> avg = py::array_t<float>(res.Size());
        avg.resize({res.Size(1),res.Size(0)});
        kipl::base::TImage<float,2> avgimg(static_cast<float*>(avg.request().ptr),dims);
        std::copy_n(res.GetDataPtr(),res.Size(),avgimg.GetDataPtr());
        return avg;
    },"Computes a combined image using the selected method",py::arg("img"),py::arg("method"),py::arg("weights")=std::vector<float>());


    py::enum_<ImagingAlgorithms::AverageImage::eAverageMethod>(avgClass,"eAverageMethod")
            .value("ImageSum",             ImagingAlgorithms::AverageImage::ImageSum)
            .value("ImageAverage",         ImagingAlgorithms::AverageImage::ImageAverage)
            .value("ImageMedian",          ImagingAlgorithms::AverageImage::ImageMedian)
            .value("ImageWeightedAverage", ImagingAlgorithms::AverageImage::ImageWeightedAverage)
            .value("ImageMin",             ImagingAlgorithms::AverageImage::ImageMin)
            .value("ImageMax",             ImagingAlgorithms::AverageImage::ImageMax)
            .export_values();

}

#endif
