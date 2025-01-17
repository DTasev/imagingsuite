//<LICENCE>

#include <vector>
#include <iostream>
#include <algorithm>

#include "../../include/stltools/geodesicstl.h"
#include "../../include/stltools/watershedstl.h"

namespace akipl { namespace segmentation {

int Threshold_Watershed(vector<size_t> &hist, double height, size_t medianlength, vector<size_t> &thvec) {

	if ((height<0) || (height>1)) {
		std::cerr<<"WatershedThreshold: the height must be given in the interval [0,1]"<<std::endl;
		return -1;
	}
	
	std::vector<size_t> medHist, localMin, normHist;
	
	MedianFilter(hist,medHist,medianlength);
	int histMax=*std::max_element(medHist.begin(),medHist.end());

	vector<size_t>::iterator it;

	for (it=medHist.begin() ; it!=medHist.end() ; it++)
		normHist.push_back((histMax-(*it)));

	medHist.clear();

	kipl::STLmorphology::EMin(normHist,localMin,size_t(height*histMax), true);
	kipl::STLmorphology::MinImpose(normHist, localMin, medHist);

	std::vector<int> ws;
	kipl::STLmorphology::watershed(medHist,ws);

	vector<int>::iterator wsIt;
	int i;
	thvec.clear();
	for (wsIt=ws.begin(), it=localMin.begin(), i=0; wsIt!= ws.end(); wsIt++,it++, i++) 
		if (!(*wsIt)) {
			thvec.push_back(i);
			*wsIt=(int)height;
		}
		else {
			*wsIt=0;
		}

	return thvec.empty();
}

}}
