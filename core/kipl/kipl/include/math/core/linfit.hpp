//<LICENSE>
#ifndef LINFIT_HPP
#define LINFIT_HPP
#include <algorithm>
#include <map>
#include <iostream>

#if !defined(NO_QT)
#include <QDebug>
#endif
#include "../../base/KiplException.h"

namespace kipl { namespace math {

template <typename T, typename S> void LinearLSFit(T *x, S *y, int N, double *q)
{
    LinearLSFit(x, y, N, q, q+1, q+2);
}

template <typename T, typename S> void LinearLSFit(std::vector<std::pair<T,S>> &data, float &m, float &k, float *R2)
{
    double dm=0.0;
    double dk=0.0;
    double dR2=0.0;

    LinearLSFit(data,dm,dk,&dR2);

    m=static_cast<float>(dm);
    k=static_cast<float>(dk);

    if (R2!=nullptr)
        *R2=static_cast<float>(dR2);

}

template <typename T, typename S> void LinearLSFit(std::vector<std::pair<T,S>> &data, double &m, double &k, double *R2)
{
    int N= static_cast<int>(data.size());
    T *x = new T[N];
    S *y = new S[N];

    int i=0;
    for (auto &point : data)
    {
        x[i]=point.first;
        y[i]=point.second;
        ++i;
    }
    LinearLSFit(x,y,N,&m,&k,R2);

    delete [] x;
    delete [] y;
}

template <typename T, typename S> void LinearLSFit(T *x, S *y, int N, double *m, double *k, double *R2)
{
    double sx=0.0;
    double sx2=0.0;
    for (int i=0; i<N; i++) {
        double xx=static_cast<double>(x[i]);
        sx+=xx;
        sx2+=xx*xx;
    }

    double det=(N*sx2-sx*sx);

    *m=0.0;
    *k=0.0;

    for (int i=0; i<N; i++) {
        double xx=static_cast<double>(x[i]);
        double yy=static_cast<double>(y[i]);
        *m+=yy*(sx2-xx*sx);
        *k+=yy*(N*xx-sx);
    }

    *m/=det;
    *k/=det;

    if (R2!=nullptr) {
        double sstot=0.0;
        double ssres=0.0;
        double sy=0.0;
        for (int i=0; i<N; i++) {
            double yy=static_cast<double>(y[i]);
            sy+=yy;
        }

        double my=sy/N;

        for (int i=0; i<N; i++) {
            double yy=static_cast<double>(y[i]);
            double xx=static_cast<double>(x[i]);

            sstot+=(yy-my)*(yy-my);

            ssres+=(yy-(*m+*k * xx)) * (yy-(*m+*k * xx));
        }

        *R2 = 1-ssres/sstot;
    }
}

template <typename T, typename S>
void LinearLSFit(T *x, S *y, int N, double *m, double *k, double *R2, double fraction)
{
    LinearLSFit(x,y,N,m,k,R2);


    std::map< double,std::pair<double,double> > sorted;
    if ((fraction<0.0) || (1.0<fraction))
        throw kipl::base::KiplException("Fraction outside the interval [0,1]",__FILE__,__LINE__);

    int N2=static_cast<int>(N*fraction);

    for (int i=0; i<N; ++i)
        sorted.insert(std::make_pair(fabs(y[i]-(*k*x[i]+*m)),std::make_pair(static_cast<double>(x[i]),static_cast<double>(y[i]))));


    auto it=sorted.begin();

    T *x2=new T[N2];
    S *y2=new S[N2];

    for (int i=0; i<N2; ++i, ++it) {
        x2[i]=it->second.first;
        y2[i]=it->second.second;
    }

    LinearLSFit(x2,y2,N2,m,k,R2);

    delete [] x2;
    delete [] y2;

}

}}
#endif // LINFIT_HPP
