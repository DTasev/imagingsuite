//<LICENSE>

#include "stdafx.h"
#include <iostream>
#include <string.h>

#include <logging/logger.h>
#include <base/timage.h>
#include <strings/miscstring.h>
#include <base/textractor.h>
#include <algorithms/datavalidator.h>

#include <ParameterHandling.h>
#include <ModuleException.h>

#include "../include/ReconEngine.h"
#include "../include/ReconException.h"
#include "../include/ReconHelpers.h"

ReconEngine::ReconEngine(std::string name, kipl::interactors::InteractionBase *interactor) :
	logger(name),
	m_FirstSlice(0),
    m_ProjectionMargin(0),
	m_ProjectionReader(interactor),
    m_BackProjector(nullptr),
    nProcessedBlocks(0),
	nProcessedProjections(0),
	nTotalProcessedProjections(0),
	nTotalBlocks(0),
	m_bCancel(false),
	m_Interactor(interactor)
{
    logger(kipl::logging::Logger::LogMessage,"C'tor Recon engine");
    if (m_Interactor!=nullptr) {
		logger(kipl::logging::Logger::LogMessage,"Got an interactor");
	}
	else {
		logger(kipl::logging::Logger::LogMessage,"An interactor was not provided");

	}

}

ReconEngine::~ReconEngine(void)
{
	std::ostringstream msg;
    logger(logger.LogVerbose,"Enter destructor");
	while (!m_PreprocList.empty()) {
		msg.str("");
		msg<<"Removing "<<m_PreprocList.front()->GetModule()->ModuleName()<<" from the module list ("<<m_PreprocList.size()<<")";
		logger(kipl::logging::Logger::LogMessage,msg.str());
        if (m_PreprocList.front()!=nullptr) {
			delete m_PreprocList.front();
		}
		msg.str("");
		msg<<"Removed the module ("<<m_PreprocList.size()<<")";
        logger(logger.LogVerbose,msg.str());

		m_PreprocList.pop_front();
	}

    if (m_BackProjector!=nullptr)
		delete m_BackProjector;
}

void ReconEngine::SetConfig(ReconConfig &config)
{
    std::ostringstream msg;
    config.SanityCheck();



	m_Config=config;


    m_ProjectionMargin = config.ProjectionInfo.nMargin;
    std::string fname,ext;

    msg.str(""); msg<<"set config.roi=["<<m_Config.ProjectionInfo.roi[0]<<","
                <<m_Config.ProjectionInfo.roi[1]<<","
                <<m_Config.ProjectionInfo.roi[2]<<","
                <<m_Config.ProjectionInfo.roi[3]<<"]";
    logger.message(msg.str());

    kipl::strings::filenames::MakeFileName(m_Config.ProjectionInfo.sFileMask,m_Config.ProjectionInfo.nFirstIndex,fname,ext,'#','0');

    msg.str(""); msg<<m_Config.ProjectionInfo.sFileMask<<", "<<m_Config.ProjectionInfo.nFirstIndex<<", "<<fname<<", "<<ext;
    logger(logger.LogMessage,msg.str());

    try {
        m_ProjectionReader.GetImageSize(fname,m_Config.ProjectionInfo.fBinning,m_Config.ProjectionInfo.nDims);
    }
    catch (ReconException &e) {
        logger(logger.LogError,"Failed to get image size while configuring recon engine.");
        throw ReconException(e.what());
    }
    catch (kipl::base::KiplException &e) {
        logger(logger.LogError,"Failed to get image size while configuring recon engine.");
        throw kipl::base::KiplException(e.what());
    }
    catch (exception &e) {
        logger(logger.LogError,"Failed to get image size while configuring recon engine.");
        throw std::runtime_error(e.what());
    }

    // check if I am writing to disk:
    if (m_Config.MatrixInfo.bAutomaticSerialize==true) {

        float res=0.0f;
        if (m_Config.ProjectionInfo.beamgeometry==ReconConfig::cProjections::BeamGeometry_Parallel)
        {
            res = m_Config.ProjectionInfo.fResolution[0];
        }

        if (m_Config.ProjectionInfo.beamgeometry==ReconConfig::cProjections::BeamGeometry_Cone)
        {
            res = m_Config.MatrixInfo.fVoxelSize[0];
        }


        if (m_Config.MatrixInfo.FileType==kipl::io::NeXusfloat)
            {

            // todo here: add the MatrixRoi option
            size_t dims[3];

            if (m_Config.MatrixInfo.bUseROI){
                dims[0] = m_Config.MatrixInfo.roi[2]-m_Config.MatrixInfo.roi[0]+1;
                dims[1] = m_Config.MatrixInfo.roi[3]-m_Config.MatrixInfo.roi[1]+1;
            }
            else
            {
                dims[0] = (m_Config.ProjectionInfo.projection_roi[2]-m_Config.ProjectionInfo.projection_roi[0]);
                dims[1] = dims[0];
            }
            dims[2] =  (m_Config.ProjectionInfo.roi[3]-m_Config.ProjectionInfo.roi[1]); // it is not necessarelly the entire dataset

            kipl::base::TImage<float, 3> img;


            std::stringstream str;
            str.str("");
            str<<m_Config.MatrixInfo.sDestinationPath<<m_Config.MatrixInfo.sFileMask;

            try
            {

                kipl::io::PrepareNeXusFileFloat(str.str().c_str(), dims, res, img);

            }
            catch (ReconException &e)
            {
                logger(logger.LogError,"Failed to PrepareNeXusFile while configuring recon engine.");
                throw ReconException(e.what());
            }
            catch (kipl::base::KiplException &e)
            {
                logger(logger.LogError,"Failed to PrepareNeXusFile while configuring recon engine.");
                throw kipl::base::KiplException(e.what());
            }
            catch (exception &e)
            {
                logger(logger.LogError,"Failed to PrepareNeXusFile while configuring recon engine.");
                throw std::runtime_error(e.what());
            }
         }

        if (m_Config.MatrixInfo.FileType==kipl::io::NeXus16bits)
        {

            // todo here: add the MatrixRoi option
            size_t dims[3];

            if (m_Config.MatrixInfo.bUseROI)
            {
                dims[0] = m_Config.MatrixInfo.roi[2]-m_Config.MatrixInfo.roi[0]+1;
                dims[1] = m_Config.MatrixInfo.roi[3]-m_Config.MatrixInfo.roi[1]+1;
            }
            else
            {
                dims[0] = (m_Config.ProjectionInfo.projection_roi[2]-m_Config.ProjectionInfo.projection_roi[0]);
                dims[1] = dims[0];
            }

            dims[2] =  (m_Config.ProjectionInfo.roi[3]-m_Config.ProjectionInfo.roi[1]); // it is not necessarelly the entire dataset

            kipl::base::TImage<float, 3> img;


            std::stringstream str;
            str.str("");
            str<<m_Config.MatrixInfo.sDestinationPath<<m_Config.MatrixInfo.sFileMask;

            try
            {

                kipl::io::PrepareNeXusFile16bit(str.str().c_str(), dims, res, img);

            }
            catch (ReconException &e)
            {
                logger(logger.LogError,"Failed to PrepareNeXusFile while configuring recon engine.");
                throw ReconException(e.what());
            }
            catch (kipl::base::KiplException &e)
            {
                logger(logger.LogError,"Failed to PrepareNeXusFile while configuring recon engine.");
                throw kipl::base::KiplException(e.what());
            }
            catch (exception &e)
            {
                logger(logger.LogError,"Failed to PrepareNeXusFile while configuring recon engine.");
                throw std::runtime_error(e.what());
            }
         }

    }
}

size_t ReconEngine::AddPreProcModule(ModuleItem *module)
{
    if (module!=nullptr)
    {
		if (module->Valid())
			m_PreprocList.push_back(module);
	}
	else
		throw ReconException("Failed to add module",__FILE__,__LINE__);

	return m_PreprocList.size();
}

void ReconEngine::SetBackProjector(BackProjItem *module)
{
    if (m_BackProjector!=nullptr)
        delete m_BackProjector;

    if (module!=nullptr)
    {
		if (module->Valid())
			m_BackProjector=module;
	}
	else
		throw ReconException("Failed to add back projector module",__FILE__,__LINE__);
}

int ReconEngine::Run()
{

	std::stringstream msg;

	BuildFileList(&m_Config,&m_ProjectionList);

	size_t roi[4]={
		m_Config.ProjectionInfo.roi[0],
		m_Config.ProjectionInfo.roi[1],
		m_Config.ProjectionInfo.roi[2],
		m_Config.ProjectionInfo.roi[3]
    };

    size_t voi[6] = {
        m_Config.MatrixInfo.voi[0],
        m_Config.MatrixInfo.voi[1],
        m_Config.MatrixInfo.voi[2],
        m_Config.MatrixInfo.voi[3],
        m_Config.MatrixInfo.voi[4],
        m_Config.MatrixInfo.voi[5]
    };


    size_t totalSlices=0;

    if (m_Config.ProjectionInfo.imagetype==ReconConfig::cProjections::ImageType_Proj_RepeatSinogram)
    {
        m_Config.MatrixInfo.nDims[2] = roi[3];
        totalSlices=roi[3];
    }
    else
    {
        m_Config.MatrixInfo.nDims[2] = roi[3]-roi[1]+1;
        totalSlices=roi[3]-roi[1];
    }
        m_Volume.Resize(m_Config.MatrixInfo.nDims);

	msg.str("");
	msg<<"ROI=["<<roi[0]<<" "<<roi[1]<<" "<<roi[2]<<" "<<roi[3]<<"]";
    logger(kipl::logging::Logger::LogVerbose,msg.str());

    switch (m_Config.ProjectionInfo.beamgeometry)
    {
        case ReconConfig::cProjections::BeamGeometry_Parallel:
            m_FirstSlice=roi[1];
            break;
        case ReconConfig::cProjections::BeamGeometry_Cone:
            m_FirstSlice=roi[3]-voi[5];
            break;
        case ReconConfig::cProjections::BeamGeometry_Helix:
            logger(logger.LogError,"Helix is not supported by the engine.");
            throw ReconException("Helix is not supported by the engine",__FILE__,__LINE__);
            break;
        default:
            logger(logger.LogError,"Unsupported geometry type.");
            throw ReconException("Unsupported geometry type.",__FILE__,__LINE__);
            break;
    }

	kipl::profile::Timer totalTimer;

	totalTimer.Tic();


	size_t nSliceBlock=GetIntParameter(m_Config.backprojector.parameters,"SliceBlock");
	nTotalBlocks=totalSlices/nSliceBlock;

	msg.str("");
	msg<<"Interval "<<totalSlices
			<<" is divided into "<<nTotalBlocks
			<<" blocks ("<<(totalSlices)
			<<"/"<<nSliceBlock<<")";

	logger(kipl::logging::Logger::LogMessage,msg.str());
	m_bCancel=false;
	int result=0;
    float radius = (static_cast<float>(m_Config.ProjectionInfo.roi[2])-static_cast<float>(m_Config.ProjectionInfo.roi[0]))/2;


	try {
        for (nProcessedBlocks=0; (nProcessedBlocks<nTotalBlocks) && (m_bCancel==false); nProcessedBlocks++)
        {
            nProcessedProjections=0;
            m_Config.ProjectionInfo.roi[3]=m_Config.ProjectionInfo.roi[1]+nSliceBlock;

            msg.str("");
            msg<<__FUNCTION__<<" Fk "<<nProcessedBlocks<<" ["
                <<m_Config.ProjectionInfo.roi[0]<<", "
                <<m_Config.ProjectionInfo.roi[1]<<", "
                <<m_Config.ProjectionInfo.roi[2]<<", "
                <<m_Config.ProjectionInfo.roi[3]<<"]";
            logger(kipl::logging::Logger::LogMessage,msg.str());

            result=Process(m_Config.ProjectionInfo.roi);
            m_Config.ProjectionInfo.roi[1]=m_Config.ProjectionInfo.roi[3];

		}

        if (totalSlices!=nSliceBlock*nTotalBlocks)
        {
            nProcessedProjections=0;
            m_Config.ProjectionInfo.roi[3]=roi[3];
            msg.str("");
            msg<<__FUNCTION__<<" Processing block "<<nProcessedBlocks<<" ["
                <<m_Config.ProjectionInfo.roi[0]<<", "
                <<m_Config.ProjectionInfo.roi[1]<<", "
                <<m_Config.ProjectionInfo.roi[2]<<", "
                <<m_Config.ProjectionInfo.roi[3]<<"]";
            logger(kipl::logging::Logger::LogMessage,msg.str());

            result=Process(m_Config.ProjectionInfo.roi);
		}
	}
    catch (ReconException &e)
    {
		msg.str("");
		msg<<"The reconstruction failed with "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (ModuleException &e)
    {
            msg.str("");
			msg<<"The reconstruction failed with "<<e.what();
			throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
		msg.str("");
		msg<<"The reconstruction failed with "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

    catch (std::exception &e)
    {
		msg.str("");
		msg<<"The reconstruction failed with "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (...)
    {
		msg.str("");
		msg<<"The reconstruction failed with an unknown error";
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}


	totalTimer.Toc();
	msg.str("");
    msg<<"Totals for "<<totalSlices<<" slices"<<std::endl
		<<totalTimer<<" ("<<totalTimer.WallTime()/static_cast<double>(totalSlices)<<" s/slice)";

	logger(kipl::logging::Logger::LogMessage,msg.str());

//	status = bCancel ? ReconStatusCancelled : ReconStatusFinished;

	return result;
}

int ReconEngine::Process(size_t *roi)
{
	std::stringstream msg;
	m_bCancel=false;
	//status=ReconStatusRunning;
    size_t margin=0;
    size_t extroi[4]={roi[0],roi[1],roi[2],roi[3]};

    extroi[1]  = margin<extroi[1] ? extroi[1] : extroi[1]-margin;
    extroi[3]  = margin+extroi[3] < m_Config.ProjectionInfo.nDims[1] ? margin+extroi[3] : extroi[3];

	std::list<ModuleItem *>::iterator it_Module;
	msg<<"Processing ROI ["<<roi[0]<<", "<<roi[1]<<", "<<roi[2]<<", "<<roi[3]<<"]";
	logger(kipl::logging::Logger::LogMessage,msg.str());
	// Initialize the plugins with the current ROI
	try {
		for (it_Module=m_PreprocList.begin();
			it_Module!=m_PreprocList.end(); it_Module++)
		{
			msg.str("");
			msg<<"Setting ROI for module "<<(*it_Module)->GetModule()->ModuleName();
			logger(kipl::logging::Logger::LogVerbose,msg.str());
            (*it_Module)->GetModule()->SetROI(extroi);
		}
	}
    catch (ReconException &e)
    {
		msg.str("");
		msg<<"SetROI failed with a ReconException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
		msg.str("");
		msg<<"SetROI failed with a KiplException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
		msg.str("");
		msg<<"SetROI failed with an STL-exception for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (...)
    {
		msg.str("");
		msg<<"SetROI failed with an unknown exception for "<<(*it_Module)->GetModule()->ModuleName();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

    m_BackProjector->GetModule()->SetROI(roi);

	std::map<std::string, std::string> parameters;

	// Start processing
	kipl::base::TImage<float,2> projection;
	kipl::profile::Timer timer;
    msg.str("");
    msg<<"Allocated preprocessors "<<m_PreprocList.size()<<" using "<<m_BackProjector->GetModule()->Name()<<std::endl
        <<"Arc=["<<m_Config.ProjectionInfo.fScanArc[0]<<", "<<m_Config.ProjectionInfo.fScanArc[1]<<"]"<<std::endl
		<<"Target matrix "<<m_BackProjector->GetModule()->GetVolume();

	logger(kipl::logging::Logger::LogMessage,msg.str());
	msg.str("");

	timer.Tic();
	std::map<float,ProjectionInfo>::iterator it_Proj;
	size_t i=0;
    for (it_Proj=m_ProjectionList.begin();
         (it_Proj!=m_ProjectionList.end()) && (m_bCancel==false) ;
         ++it_Proj, ++i)
	{
		float fWeight=it_Proj->second.weight/(0<m_Config.ProjectionInfo.fResolution[0] ? m_Config.ProjectionInfo.fResolution[0]*0.1f : 1.0f);
		float fAngle=it_Proj->second.angle+m_Config.MatrixInfo.fRotation;

		nProcessedProjections=i;

		projection=m_ProjectionReader.Read(it_Proj->second.name,
				m_Config.ProjectionInfo.eFlip,
				m_Config.ProjectionInfo.eRotate,
				m_Config.ProjectionInfo.fBinning,
                extroi);
		parameters["dose"]=kipl::strings::value2string(
				m_ProjectionReader.GetProjectionDose(it_Proj->second.name,
						m_Config.ProjectionInfo.eFlip,
						m_Config.ProjectionInfo.eRotate,
						m_Config.ProjectionInfo.fBinning,
						m_Config.ProjectionInfo.dose_roi));

		msg.str("");
		msg<<"Block "<<nProcessedBlocks<<", Projection "<<i<<" (weight="<<fWeight<<", angle="<<fAngle<<")";
		logger(kipl::logging::Logger::LogVerbose, msg.str());

        for (it_Module=m_PreprocList.begin(); it_Module!=m_PreprocList.end(); it_Module++)
        {
			(*it_Module)->GetModule()->Process(projection,parameters);
		}


		m_BackProjector->GetModule()->Process(projection, fAngle, fWeight,m_ProjectionList.size()<(i+1));
	}

    if (m_bCancel==true)
    {
		logger(kipl::logging::Logger::LogVerbose,"Reconstruction was cancelled by the user.");
		return 1;
	}
    else
    {
		logger(kipl::logging::Logger::LogVerbose,"Reconstruction finished");

		size_t dims[3];


		if (m_Config.MatrixInfo.bAutomaticSerialize==true)
            Serialize(dims);
        else
        {
			TransferMatrix(roi);
		}
	}

	return 0;
}

bool ReconEngine::TransferMatrix(size_t *dims)
{
    std::ostringstream msg;
	bool bTransposed=false;


	kipl::base::TImage<float,2> slice;

    try {

        for (size_t i=0; i<(dims[3]-dims[1]); i++)
        {
            slice=m_BackProjector->GetModule()->GetSlice(i);
            float *pVol=m_Volume.GetLinePtr(0,dims[1]-m_FirstSlice+i);
            memcpy(pVol,slice.GetDataPtr(),slice.Size()*sizeof(float));
        }
    }
    catch (ReconException &e)
    {
        msg.str("");
        msg<<"Transfer matrix from backprojector failed with ReconException ("<<m_Config.ProjectionInfo.beamgeometry<<")"<<std::endl<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);

    }
    catch (kipl::base::KiplException &e)
    {
        msg.str("");
        msg<<"Transfer matrix from backprojector failed with KiplException ("<<m_Config.ProjectionInfo.beamgeometry<<")"<<std::endl<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (std::exception &e)
    {
        msg.str("");
        msg<<"Transfer matrix from backprojector failed stl exception ("<<m_Config.ProjectionInfo.beamgeometry<<")"<<std::endl<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (...)
    {
        msg<<"Transfer matrix from backprojector failed with unhandled exception ("<<m_Config.ProjectionInfo.beamgeometry<<")"<<std::endl;
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }

	return bTransposed;
}

bool ReconEngine::Serialize(size_t *dims)
{


	std::stringstream msg;

	std::stringstream str;
	kipl::base::TImage<float,3> img=m_BackProjector->GetModule()->GetVolume();
	img.info.SetMetricX(m_Config.ProjectionInfo.fResolution[0]);
	img.info.SetMetricY(m_Config.ProjectionInfo.fResolution[1]);
	img.info.sArtist=m_Config.UserInformation.sOperator;
	img.info.sCopyright=m_Config.UserInformation.sOperator;
    img.info.sSoftware="MuhRec3 CT reconstructor";
	img.info.sDescription=m_Config.UserInformation.sSample;

	str.str("");
	str<<m_Config.MatrixInfo.sDestinationPath<<m_Config.MatrixInfo.sFileMask;
	
	bool bTransposed=false;

   if (m_Config.MatrixInfo.FileType==kipl::io::NeXusfloat) {

       kipl::base::eImagePlanes plane=kipl::base::ImagePlaneXY;

       if (m_BackProjector->GetModule()->MatrixAlignment == BackProjectorModuleBase::MatrixZXY)
           plane=kipl::base::ImagePlaneYZ;

       size_t nSlices=0;
       nSlices=m_BackProjector->GetModule()->GetNSlices();
       size_t nSliceBlock=GetIntParameter(m_Config.backprojector.parameters,"SliceBlock");

       size_t Start = nSliceBlock*nProcessedBlocks;
       if (m_Config.MatrixInfo.bUseROI){
            kipl::io::WriteNeXusStack(img, str.str().c_str(), Start,nSlices, plane, m_Config.MatrixInfo.roi);
       }
       else {
            kipl::io::WriteNeXusStack(img, str.str().c_str(), Start,nSlices, plane, nullptr);
       }

	}
    else if (m_Config.MatrixInfo.FileType==kipl::io::NeXus16bits) {

       kipl::base::eImagePlanes plane=kipl::base::ImagePlaneXY;

       if (m_BackProjector->GetModule()->MatrixAlignment == BackProjectorModuleBase::MatrixZXY)
           plane=kipl::base::ImagePlaneYZ;

       size_t nSlices=0;
       nSlices=m_BackProjector->GetModule()->GetNSlices();
       size_t nSliceBlock=GetIntParameter(m_Config.backprojector.parameters,"SliceBlock");

       size_t Start = nSliceBlock*nProcessedBlocks;
       if (m_Config.MatrixInfo.bUseROI){
            kipl::io::WriteNeXusStack16bit(img, str.str().c_str(), Start,nSlices, m_Config.MatrixInfo.fGrayInterval[0],m_Config.MatrixInfo.fGrayInterval[1], plane, m_Config.MatrixInfo.roi);
       }
       else {
            kipl::io::WriteNeXusStack16bit(img, str.str().c_str(), Start,nSlices,m_Config.MatrixInfo.fGrayInterval[0],m_Config.MatrixInfo.fGrayInterval[1], plane, nullptr);
       }

    }
	else {
		kipl::base::eImagePlanes plane=kipl::base::ImagePlaneXY;

		if (m_BackProjector->GetModule()->MatrixAlignment == BackProjectorModuleBase::MatrixZXY)
			plane=kipl::base::ImagePlaneYZ;

		size_t nSlices=0;
		nSlices=m_BackProjector->GetModule()->GetNSlices();

		msg.str("");
		msg<<"Serializing "<<nSlices<<" slices to "<<m_Config.MatrixInfo.sDestinationPath;
		logger(kipl::logging::Logger::LogMessage,msg.str());
        msg.str("");
        try {
            if (m_Config.MatrixInfo.bUseROI)
            {
                logger(kipl::logging::Logger::LogMessage,"Serializing matrix with ROI");
                if (m_Config.ProjectionInfo.beamgeometry==m_Config.ProjectionInfo.BeamGeometry_Parallel)
                {
                    kipl::io::WriteImageStack(img,
                        str.str(),
                        m_Config.MatrixInfo.fGrayInterval[0],m_Config.MatrixInfo.fGrayInterval[1],
                        0,nSlices,m_Config.ProjectionInfo.roi[1],m_Config.MatrixInfo.FileType,plane,m_Config.MatrixInfo.roi);
                }
                else if (m_Config.ProjectionInfo.beamgeometry == m_Config.ProjectionInfo.BeamGeometry_Cone)
                {       kipl::io::WriteImageStack(img,
                                                  str.str(),
                                                  m_Config.MatrixInfo.fGrayInterval[0],m_Config.MatrixInfo.fGrayInterval[1],
                                                  0,nSlices, CBroi[1], m_Config.MatrixInfo.FileType,plane,m_Config.MatrixInfo.roi);
                }
            }
            else
            {
                logger(kipl::logging::Logger::LogMessage,"Serializing full matrix");
                if (m_Config.ProjectionInfo.beamgeometry==m_Config.ProjectionInfo.BeamGeometry_Parallel)
                {
                    kipl::io::WriteImageStack(img,
                        str.str(),
                        m_Config.MatrixInfo.fGrayInterval[0],m_Config.MatrixInfo.fGrayInterval[1],
                        0,nSlices,m_Config.ProjectionInfo.roi[1],m_Config.MatrixInfo.FileType,plane,nullptr);
                }
                else if (m_Config.ProjectionInfo.beamgeometry == m_Config.ProjectionInfo.BeamGeometry_Cone)
                {
                    kipl::io::WriteImageStack(img,
                        str.str(),
                        m_Config.MatrixInfo.fGrayInterval[0],m_Config.MatrixInfo.fGrayInterval[1],
                        0,nSlices, CBroi[1], m_Config.MatrixInfo.FileType,plane,nullptr);
                }
            }
        }
        catch (ReconException & e)
        {
            msg<<"Serializing failed with a ReconException: "<<e.what();
            throw ReconException(msg.str(),__FILE__,__LINE__);
        }
        catch (kipl::base::KiplException & e)
        {
            msg<<"Serializing failed with a KiplException: "<<e.what();
            throw ReconException(msg.str(),__FILE__,__LINE__);
        }
        catch (std::exception & e)
        {
            msg<<"Serializing failed with an STL exception: "<<e.what();
            throw ReconException(msg.str(),__FILE__,__LINE__);
        }
        catch (...)
        {
            throw ReconException("An unhandled exception was thrown.",__FILE__,__LINE__);
        }

	}



    if (dims!=nullptr)
		memcpy(dims,img.Dims(),3*sizeof(size_t));


	return bTransposed;
}

kipl::base::TImage<float,2> ReconEngine::GetSlice(size_t index, kipl::base::eImagePlanes plane)
{
	kipl::base::TImage<float,2> img;

    if (m_Volume.Size()!=0UL)
        img=kipl::base::ExtractSlice(m_Volume,index,plane,nullptr);

	return img;
}

size_t ReconEngine::GetHistogram(float *axis, size_t *hist, size_t nBins)
{
	m_BackProjector->GetModule()->GetHistogram(axis,hist,nBins);

	return nBins;
}

bool ReconEngine::Serialize(ReconConfig::cMatrix *matrixconfig)
{

	std::stringstream msg;
	std::stringstream str;

	m_Volume.info.SetMetricX(m_Config.ProjectionInfo.fResolution[0]);
	m_Volume.info.SetMetricY(m_Config.ProjectionInfo.fResolution[1]);
	m_Volume.info.sArtist=m_Config.UserInformation.sOperator;
	m_Volume.info.sCopyright=m_Config.UserInformation.sOperator;
	m_Volume.info.sSoftware="MUHRec CT reconstructor";
	m_Volume.info.sDescription=m_Config.UserInformation.sSample;

	str.str("");
	str<<matrixconfig->sDestinationPath<<matrixconfig->sFileMask;

	bool bTransposed=false;
/*	if (matrixconfig->FileType==kipl::io::MatlabVolume) {
		logger(kipl::logging::Logger::LogVerbose,"Serializing matrix");
        std::string path,name;
		std::vector<std::string> ext;
		kipl::strings::filenames::StripFileName(str.str(),path,name,ext);
		kipl::io::WriteMAT(m_Volume,str.str().c_str(),name.c_str());
	}
    else */

    float res=0.0f;
    if (m_Config.ProjectionInfo.beamgeometry==ReconConfig::cProjections::BeamGeometry_Parallel)
    {
        res = m_Config.ProjectionInfo.fResolution[0];
    }

    if (m_Config.ProjectionInfo.beamgeometry==ReconConfig::cProjections::BeamGeometry_Cone)
    {
        res = m_Config.MatrixInfo.fVoxelSize[0];
    }

    if (matrixconfig->FileType==kipl::io::NeXusfloat){
       kipl::io::WriteNexusFloat(m_Volume,str.str().c_str(),res);
    }
    else if (matrixconfig->FileType==kipl::io::NeXus16bits){
        kipl::io::WriteNexus16bits(m_Volume, str.str().c_str(),matrixconfig->fGrayInterval[0],matrixconfig->fGrayInterval[1], res);
    }
    else
    {
		kipl::base::eImagePlanes plane=kipl::base::ImagePlaneXY;
		size_t nSlices=0;
		nSlices=m_Volume.Size(2);
		msg.str("");
		msg<<"Serializing "<<nSlices<<" slices to "<<str.str();
		logger(kipl::logging::Logger::LogMessage,msg.str());

		kipl::io::WriteImageStack(m_Volume,
				str.str(),
				matrixconfig->fGrayInterval[0],matrixconfig->fGrayInterval[1],
				0,nSlices,m_FirstSlice,matrixconfig->FileType,plane);
	}

	return bTransposed;
}

int ReconEngine::Run3D(bool bRerunBackproj)
{
    std::stringstream msg;

    int res=0;
    msg<<"Rerun backproj: "<<(bRerunBackproj ? "true" : "false")<<", status projection blocks "<<(m_ProjectionBlocks.empty() ? "empty" : "has data");
    logger(kipl::logging::Logger::LogMessage,msg.str());
    try {
        msg.str(""); msg<<"run3d "<<m_Config.ProjectionInfo.beamgeometry;
        logger.message(msg.str());
        if ((bRerunBackproj==true) && (m_ProjectionBlocks.empty()==false))
            res=Run3DBackProjOnly();
        else
            res=Run3DFull();
    }
    catch (ReconException &e)
    {
        msg.str("");
        msg<<"Run3D failed with "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (kipl::base::KiplException &e)
    {
        msg.str("");
        msg<<"Run3D failed with "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (std::exception &e)
    {
        msg.str("");
        msg<<"Run3D failed with "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (...)
    {
        msg.str("");
        msg<<"Run3D failed with an unknown error";
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }

    return res;
}

int ReconEngine::Run3DFull()
{
	std::stringstream msg;

    logger(kipl::logging::Logger::LogVerbose,"Entering Run3DFull");
    m_ProjectionBlocks.clear();
	size_t roi[4]={
		m_Config.ProjectionInfo.roi[0],
		m_Config.ProjectionInfo.roi[1],
		m_Config.ProjectionInfo.roi[2],
		m_Config.ProjectionInfo.roi[3]
	};

    CBroi[0] = m_Config.ProjectionInfo.roi[0];
    CBroi[1] = m_Config.ProjectionInfo.roi[1];
    CBroi[2] = m_Config.ProjectionInfo.roi[2];
    CBroi[3] = m_Config.ProjectionInfo.roi[3];

    size_t voi[6] = {
        m_Config.MatrixInfo.voi[0],
        m_Config.MatrixInfo.voi[1],
        m_Config.MatrixInfo.voi[2],
        m_Config.MatrixInfo.voi[3],
        m_Config.MatrixInfo.voi[4],
        m_Config.MatrixInfo.voi[5]
    };


    size_t totalSlices=0;

    if (m_Config.MatrixInfo.bUseROI)
    {
        m_Config.MatrixInfo.nDims[0] = m_Config.MatrixInfo.roi[2]-m_Config.MatrixInfo.roi[0]+1;
        m_Config.MatrixInfo.nDims[1] = m_Config.MatrixInfo.roi[3]-m_Config.MatrixInfo.roi[1]+1;
    }
    else
    {
        m_Config.MatrixInfo.nDims[0] = roi[2]-roi[0];
        m_Config.MatrixInfo.nDims[1] = m_Config.MatrixInfo.nDims[0];
    }

    if (m_Config.ProjectionInfo.imagetype==ReconConfig::cProjections::ImageType_Proj_RepeatSinogram) {
        m_Config.MatrixInfo.nDims[2] = roi[3];
        totalSlices=roi[3];
    }
    else
    {
        m_Config.MatrixInfo.nDims[2] = roi[3]-roi[1];
    }

    totalSlices=m_Config.MatrixInfo.nDims[2];


	msg.str("");
    if (!m_Config.MatrixInfo.bAutomaticSerialize)
    {
        try
        {
                m_Volume.Resize(m_Config.MatrixInfo.nDims);
                m_Volume = 0.0f;
		}
        catch (kipl::base::KiplException &e)
        {
			msg<<"Failed to allocate target matrix with dimensions "
					<<m_Config.MatrixInfo.nDims[0]<<"x"
					<<m_Config.MatrixInfo.nDims[1]<<"x"
					<<m_Config.MatrixInfo.nDims[2]<<"\n"<<e.what();
			throw ReconException(msg.str(),__FILE__,__LINE__);

		}
	}

	msg.str("");
    msg<<": ROI=["<<roi[0]<<" "<<roi[1]<<" "<<roi[2]<<" "<<roi[3]<<"]";
	logger(kipl::logging::Logger::LogVerbose,msg.str());


     m_FirstSlice=roi[1];


	kipl::profile::Timer totalTimer;

	totalTimer.Tic();


	size_t nSliceBlock=GetIntParameter(m_Config.backprojector.parameters,"SliceBlock");
	nTotalBlocks=totalSlices/nSliceBlock;

	msg.str("");
    msg<<": Interval "<<totalSlices
			<<" is divided into "<<nTotalBlocks
			<<" blocks ("<<(totalSlices)
			<<"/"<<nSliceBlock<<")";


	logger(kipl::logging::Logger::LogMessage,msg.str());
	m_bCancel=false;
	int result=0;
    float radius = static_cast<float>(m_Config.ProjectionInfo.roi[2]-m_Config.ProjectionInfo.roi[0])*m_Config.MatrixInfo.fVoxelSize[0]/2;

    msg.str("");

    msg<<"Run3DFull beam geometry: "<<m_Config.ProjectionInfo.beamgeometry;
    logger.message(msg.str());
    try {
        for (nProcessedBlocks=0;
             (nProcessedBlocks<nTotalBlocks) && (!UpdateProgress(static_cast<float>(nProcessedBlocks)/nTotalBlocks, "Blocks"));
             ++nProcessedBlocks)
        {
            msg.str("");
            msg<<__FUNCTION__<<"Run3DFull Processing block "<<nProcessedBlocks<<" ["
                <<m_Config.ProjectionInfo.roi[0]<<", "
                <<m_Config.ProjectionInfo.roi[1]<<", "
                <<m_Config.ProjectionInfo.roi[2]<<", "
                <<m_Config.ProjectionInfo.roi[3]<<"]";
            logger.message(msg.str());
            if (m_Config.ProjectionInfo.beamgeometry==m_Config.ProjectionInfo.BeamGeometry_Cone)
            {
                if (m_Interactor!=nullptr)
                    m_Interactor->SetOverallProgress(float(nProcessedBlocks/float(nTotalBlocks)));

                size_t CBCT_roi[4];
                CBCT_roi[0] = m_Config.ProjectionInfo.roi[0];
                CBCT_roi[2] = m_Config.ProjectionInfo.roi[2];

                nProcessedProjections=0;
                m_Config.ProjectionInfo.roi[3]=m_Config.ProjectionInfo.roi[1]+nSliceBlock;


                if (m_Config.ProjectionInfo.fpPoint[1]>=static_cast<float>(m_Config.ProjectionInfo.roi[1])
                        && m_Config.ProjectionInfo.fpPoint[1]>=static_cast<float>(m_Config.ProjectionInfo.roi[3]))
                {
                    CBCT_roi[3] = static_cast<size_t>(m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[3]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD+radius))/m_Config.ProjectionInfo.fResolution[0]);
                    float value = m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                    if(value<=0)
                        CBCT_roi[1] = 0;
                    else
                        CBCT_roi[1] = static_cast<size_t>(m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0]);
                }

                if (m_Config.ProjectionInfo.fpPoint[1]<static_cast<float>(m_Config.ProjectionInfo.roi[1]) && m_Config.ProjectionInfo.fpPoint[1]<static_cast<float>(m_Config.ProjectionInfo.roi[3]))
                {
                    float value = m_Config.ProjectionInfo.fpPoint[1]+((static_cast<float>(m_Config.ProjectionInfo.roi[1])-m_Config.ProjectionInfo.fpPoint[1])*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD+radius))/m_Config.ProjectionInfo.fResolution[0];
                     CBCT_roi[1] = static_cast<size_t>(value);
                     float value2 = m_Config.ProjectionInfo.fpPoint[1]+((static_cast<float>(m_Config.ProjectionInfo.roi[3])-m_Config.ProjectionInfo.fpPoint[1])*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                     if (value2>=m_Config.ProjectionInfo.projection_roi[3])
                         CBCT_roi[3] = m_Config.ProjectionInfo.projection_roi[3];
                     else
                         CBCT_roi[3] = static_cast<float>(value2);
                }

               if (m_Config.ProjectionInfo.fpPoint[1]>=static_cast<float>(m_Config.ProjectionInfo.roi[1]) && m_Config.ProjectionInfo.fpPoint[1]<static_cast<float>(m_Config.ProjectionInfo.roi[3]))
               {
                   float value = m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                   if(value<=0)
                       CBCT_roi[1] = 0;
                   else
                       CBCT_roi[1] = static_cast<size_t>(m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0]);

                   float value2 = m_Config.ProjectionInfo.fpPoint[1]+((static_cast<float>(m_Config.ProjectionInfo.roi[3])-m_Config.ProjectionInfo.fpPoint[1])*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                   if (value2>=m_Config.ProjectionInfo.projection_roi[3])
                       CBCT_roi[3] = m_Config.ProjectionInfo.projection_roi[3];
                   else
                       CBCT_roi[3] = static_cast<float>(value2);
               }


               if (CBCT_roi[1]-8>=0 && CBCT_roi[1]!=0)
                   CBCT_roi[1] -=8;
               if (CBCT_roi[3]+8<=m_Config.ProjectionInfo.projection_roi[3])
                   CBCT_roi[3] +=8;

//                std::cout << CBCT_roi[1] << " " << CBCT_roi[3] << std::endl;

                msg.str("");
                msg<<__FUNCTION__<<" CBCT Processing block "<<nProcessedBlocks<<" ["
                    <<m_Config.ProjectionInfo.roi[0]<<", "
                    <<m_Config.ProjectionInfo.roi[1]<<", "
                    <<m_Config.ProjectionInfo.roi[2]<<", "
                    <<m_Config.ProjectionInfo.roi[3]<<"]";
                logger(kipl::logging::Logger::LogMessage,msg.str());

                CBroi[0] = m_Config.ProjectionInfo.roi[0];
                CBroi[1] = m_Config.ProjectionInfo.roi[1];
                CBroi[2] = m_Config.ProjectionInfo.roi[2];
                CBroi[3] = m_Config.ProjectionInfo.roi[3];
                result=Process3D(CBCT_roi);
                m_Config.ProjectionInfo.roi[1]=m_Config.ProjectionInfo.roi[3];

            }
            else
            {
                    if (m_Interactor!=nullptr)
                        m_Interactor->SetOverallProgress(float(nProcessedBlocks/float(nTotalBlocks)));
                    nProcessedProjections=0;
                    m_Config.ProjectionInfo.roi[3]=m_Config.ProjectionInfo.roi[1]+nSliceBlock;

                    msg.str("");
                    msg<<__FUNCTION__<<" Processing block "<<nProcessedBlocks<<" ["
                        <<m_Config.ProjectionInfo.roi[0]<<", "
                        <<m_Config.ProjectionInfo.roi[1]<<", "
                        <<m_Config.ProjectionInfo.roi[2]<<", "
                        <<m_Config.ProjectionInfo.roi[3]<<"]";

                    logger.message(msg.str());


                    result=Process3D(m_Config.ProjectionInfo.roi);
                    m_Config.ProjectionInfo.roi[1]=m_Config.ProjectionInfo.roi[3];
            }
		}

        msg.str(""); msg<<"pre call proc3d "<<m_Config.ProjectionInfo.beamgeometry;
        logger.message(msg.str());
        if ((totalSlices!=nSliceBlock*nTotalBlocks) && !UpdateProgress(1.0f, "Last block"))
        {
            if (m_Config.ProjectionInfo.beamgeometry==m_Config.ProjectionInfo.BeamGeometry_Cone)
            {

                size_t CBCT_roi[4];
                CBCT_roi[0] = m_Config.ProjectionInfo.roi[0];
                CBCT_roi[2] = m_Config.ProjectionInfo.roi[2];

                nProcessedProjections=0;
                m_Config.ProjectionInfo.roi[3]=roi[3];

                CBroi[0] = m_Config.ProjectionInfo.roi[0];
                CBroi[1] = m_Config.ProjectionInfo.roi[1];
                CBroi[2] = m_Config.ProjectionInfo.roi[2];
                CBroi[3] = m_Config.ProjectionInfo.roi[3];

                if (m_Config.ProjectionInfo.fpPoint[1]>=static_cast<float>(m_Config.ProjectionInfo.roi[1]) && m_Config.ProjectionInfo.fpPoint[1]>=static_cast<float>(m_Config.ProjectionInfo.roi[3]))
                {
                    CBCT_roi[3] = static_cast<size_t>(m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[3]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD+radius))/m_Config.ProjectionInfo.fResolution[0]);
                    float value = m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                    if(value<=0)
                        CBCT_roi[1] = 0;
                    else
                        CBCT_roi[1] = static_cast<size_t>(m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0]);
                }

                if (m_Config.ProjectionInfo.fpPoint[1]<static_cast<float>(m_Config.ProjectionInfo.roi[1]) && m_Config.ProjectionInfo.fpPoint[1]<static_cast<float>(m_Config.ProjectionInfo.roi[3]))
                {
                    float value = m_Config.ProjectionInfo.fpPoint[1]+((static_cast<float>(m_Config.ProjectionInfo.roi[1])-m_Config.ProjectionInfo.fpPoint[1])*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD+radius))/m_Config.ProjectionInfo.fResolution[0];
                     CBCT_roi[1] = static_cast<size_t>(value);
                     float value2 = m_Config.ProjectionInfo.fpPoint[1]+((static_cast<float>(m_Config.ProjectionInfo.roi[3])-m_Config.ProjectionInfo.fpPoint[1])*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                     if (value2>=m_Config.ProjectionInfo.projection_roi[3])
                         CBCT_roi[3] = m_Config.ProjectionInfo.projection_roi[3];
                     else
                         CBCT_roi[3] = static_cast<size_t>(value2);
                }

               if (m_Config.ProjectionInfo.fpPoint[1]>=static_cast<float>(m_Config.ProjectionInfo.roi[1]) && m_Config.ProjectionInfo.fpPoint[1]<static_cast<float>(m_Config.ProjectionInfo.roi[3]))
               {
                   float value = m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                   if(value<=0)
                       CBCT_roi[1] = 0;
                   else
                       CBCT_roi[1] = static_cast<size_t>(m_Config.ProjectionInfo.fpPoint[1]-((m_Config.ProjectionInfo.fpPoint[1]-static_cast<float>(m_Config.ProjectionInfo.roi[1]))*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0]);

                   float value2 = m_Config.ProjectionInfo.fpPoint[1]+((static_cast<float>(m_Config.ProjectionInfo.roi[3])-m_Config.ProjectionInfo.fpPoint[1])*m_Config.MatrixInfo.fVoxelSize[0]*m_Config.ProjectionInfo.fSDD/(m_Config.ProjectionInfo.fSOD-radius))/m_Config.ProjectionInfo.fResolution[0];
                   if (value2>=m_Config.ProjectionInfo.projection_roi[3])
                       CBCT_roi[3] = m_Config.ProjectionInfo.projection_roi[3];
                   else
                       CBCT_roi[3] = static_cast<size_t>(value2);
               }

               if (CBCT_roi[1]-8>=0 && CBCT_roi[1]!=0)
                   CBCT_roi[1] -=8;
               if (CBCT_roi[3]+8<=m_Config.ProjectionInfo.projection_roi[3])
                   CBCT_roi[3] +=8;


                msg.str("");
                msg<<": Processing block "<<nProcessedBlocks<<" ["
                    <<m_Config.ProjectionInfo.roi[1]<<", "
                    <<m_Config.ProjectionInfo.roi[3]<<"]";
                logger(kipl::logging::Logger::LogMessage,msg.str());

                result=Process3D(CBCT_roi);
            }
            else
            {
                nProcessedProjections=0;
                m_Config.ProjectionInfo.roi[3]=roi[3];
                msg.str("");
                msg<<__FUNCTION__<<" Processing block "<<nProcessedBlocks<<" ["
                    <<m_Config.ProjectionInfo.roi[0]<<", "
                    <<m_Config.ProjectionInfo.roi[1]<<", "
                    <<m_Config.ProjectionInfo.roi[2]<<", "
                    <<m_Config.ProjectionInfo.roi[3]<<"]";

                logger(kipl::logging::Logger::LogMessage,msg.str());

                result=Process3D(m_Config.ProjectionInfo.roi);
            }
		}
	}
	catch (ReconException &e) {
		msg.str("");
		msg<<"The reconstruction failed with "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
	catch (kipl::base::KiplException &e) {
		msg.str("");
		msg<<"The reconstruction failed with "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
	catch (std::exception &e) {
		msg.str("");
		msg<<"The reconstruction failed with "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
	catch (...) {
		msg.str("");
		msg<<"The reconstruction failed with an unknown error";
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

    if (!UpdateProgress(static_cast<float>(nProcessedBlocks)/nTotalBlocks, "Finished"))
    {
		totalTimer.Toc();
		msg.str("");
        msg<<": Totals for "<<totalSlices<<" slices"<<std::endl
			<<totalTimer<<" ("<<totalTimer.ElapsedSeconds()/static_cast<double>(totalSlices)<<" s/slice)";

		logger(kipl::logging::Logger::LogMessage,msg.str());

		msg.str("");
		msg<<"\nModule process time:\n";
		std::list<ModuleItem *>::iterator it_Module;
        for (it_Module=m_PreprocList.begin(); it_Module!=m_PreprocList.end(); ++it_Module) {
			msg<<(*it_Module)->GetModule()->ModuleName()<<": "<<(*it_Module)->GetModule()->ExecTime()<<"s\n";
		}

		logger(kipl::logging::Logger::LogMessage,msg.str());

        logger(kipl::logging::Logger::LogMessage,"Run 3D done.");
		Done();
	}
    else
    {
        logger(kipl::logging::Logger::LogMessage,"Run 3D was canceled by the user.");
        m_ProjectionBlocks.clear();
	}

	return result;
}

int ReconEngine::Run3DBackProjOnly()
{
    logger(kipl::logging::Logger::LogMessage,"Running Back-projection only");
    m_BackProjector->GetModule()->Configure(m_Config,m_Config.backprojector.parameters);

    m_Volume=0.0f;
    int result=ProcessExistingProjections3D(nullptr);

    Done();
    return result;
}

kipl::base::TImage<float,3> ReconEngine::RunPreproc(size_t * roi, std::string sLastModule)
{
	std::stringstream msg;
	m_bCancel=false;

	std::list<ModuleItem *>::iterator it_Module;
	// Initialize the plug-ins with the current ROI
	try {
		for (it_Module=m_PreprocList.begin();
			it_Module!=m_PreprocList.end(); it_Module++)
		{
			msg.str("");
			msg<<"Setting ROI for module "<<(*it_Module)->GetModule()->ModuleName();
			logger(kipl::logging::Logger::LogVerbose,msg.str());
			(*it_Module)->GetModule()->SetROI(roi);
		}
	}
    catch (ReconException &e)
    {
		msg.str("");
		msg<<"SetROI failed with a ReconException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
		msg.str("");
		msg<<"SetROI failed with a KiplException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
		msg.str("");
		msg<<"SetROI failed with an STL-exception for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
	catch (...) {
		msg.str("");
		msg<<"SetROI failed with an unknown exception for "<<(*it_Module)->GetModule()->ModuleName();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

	std::map<std::string, std::string> parameters;

	// Start processing
	kipl::profile::Timer timer;

	msg<<"Allocated preprocessors "<<m_PreprocList.size()<<" using "<<m_BackProjector->GetModule()->Name()<<"\n"
		<<"Arc=["<<m_Config.ProjectionInfo.fScanArc[0]<<", "<<m_Config.ProjectionInfo.fScanArc[1]<<"]"<<"\n"
		<<"Target matrix "<<m_BackProjector->GetModule()->GetVolume();

	logger(kipl::logging::Logger::LogMessage,msg.str());
	msg.str("");

	timer.Tic();

	kipl::base::TImage<float,3> projections;

	msg.str("");

    try
    {
		projections=m_ProjectionReader.Read(m_Config,roi,parameters);
        validateImage(projections.GetDataPtr(), projections.Size(),"post read RunPreproc");
	}
    catch (ReconException &e)
    {
		msg<<"Reading projections failed with a recon exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
		msg<<"Reading projections failed with a kipl exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
		msg<<"Reading projections failed with a STL exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

	logger(kipl::logging::Logger::LogMessage,"Starting preprocessing");
    try
    {
        for (it_Module=m_PreprocList.begin(); (it_Module!=m_PreprocList.end()) && (*it_Module)->GetModule()->ModuleName()!=sLastModule; it_Module++)
        {
			msg.str("");
			msg<<"Processing: "<<(*it_Module)->GetModule()->ModuleName();
			logger(kipl::logging::Logger::LogMessage,msg.str());
			if (!(m_bCancel=UpdateProgress(0.0f, msg.str())))
				(*it_Module)->GetModule()->Process(projections,parameters);
			else
				break;

            validateImage(projections.GetDataPtr(), projections.Size(),(*it_Module)->GetModule()->ModuleName());
		}
	}
    catch (ReconException &e)
    {
		msg<<"Preprocessing failed with a recon exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
		msg<<"Preprocessing failed with a kipl exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
		msg<<"Preprocessing failed with an STL exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

	return projections;
}

int ReconEngine::Process3D(size_t *roi)
{
	std::stringstream msg;
	m_bCancel=false;

	std::list<ModuleItem *>::iterator it_Module;
    msg<<": Processing ROI in 3D mode ["<<roi[0]<<", "<<roi[1]<<", "<<roi[2]<<", "<<roi[3]<<"]";
	logger(kipl::logging::Logger::LogMessage,msg.str());
    size_t extroi[4]={roi[0],roi[1],roi[2],roi[3]};

    if (m_Config.ProjectionInfo.beamgeometry!=m_Config.ProjectionInfo.BeamGeometry_Cone)
    {
        if (m_ProjectionMargin<=roi[1])
            extroi[1]-=m_ProjectionMargin;

        extroi[3]  = m_ProjectionMargin+extroi[3] < m_Config.ProjectionInfo.nDims[1] ? m_ProjectionMargin+extroi[3] : extroi[3];
    }

    msg.str("");
    msg<<": Processing ext ROI ["<<extroi[0]<<", "<<extroi[1]<<", "<<extroi[2]<<", "<<extroi[3]<<"]";
    logger(kipl::logging::Logger::LogMessage,msg.str());

	// Initialize the plug-ins with the current ROI
    try
    {
		msg.str("");
        msg<<": Number of pre proc modules:"<<m_PreprocList.size();
		logger(kipl::logging::Logger::LogMessage,msg.str());
		for (it_Module=m_PreprocList.begin();
			it_Module!=m_PreprocList.end(); it_Module++)
		{
			msg.str("");
            msg<<": Setting ROI for module "<<(*it_Module)->GetModule()->ModuleName();

			logger(kipl::logging::Logger::LogMessage,msg.str());
            (*it_Module)->GetModule()->SetROI(extroi);
			logger(kipl::logging::Logger::LogMessage,"ROI set");
		}
	}
    catch (ReconException &e)
    {
		msg.str("");
		msg<<"SetROI failed with a ReconException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		logger(kipl::logging::Logger::LogError,msg.str());
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (ModuleException &e)
    {
			msg.str("");
			msg<<"SetROI failed with a ModuleException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
			logger(kipl::logging::Logger::LogError,msg.str());
			throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (kipl::base::KiplException &e)
    {
		msg.str("");
		msg<<"SetROI failed with a KiplException for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		logger(kipl::logging::Logger::LogError,msg.str());
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
		msg.str("");
		msg<<"SetROI failed with an STL-exception for "<<(*it_Module)->GetModule()->ModuleName()<<"\n"<<e.what();
		logger(kipl::logging::Logger::LogError,msg.str());
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (...)
    {
		msg.str("");
		msg<<"SetROI failed with an unknown exception for "<<(*it_Module)->GetModule()->ModuleName();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}

    try
    {
        // add switch on beam geometry
        switch (m_Config.ProjectionInfo.beamgeometry)
        {
            case ReconConfig::cProjections::BeamGeometry_Parallel:
                m_BackProjector->GetModule()->SetROI(roi);
                break;
            case ReconConfig::cProjections::BeamGeometry_Cone:
                m_BackProjector->GetModule()->SetROI(CBroi);
                break;
            case ReconConfig::cProjections::BeamGeometry_Helix:
                logger(logger.LogError,"Helix is not supported by the engine.");
                throw ReconException("Helix is not supported by the engine",__FILE__,__LINE__);
                break;
            default:
                logger(logger.LogError,"Unsupported geometry type.");
                throw ReconException("Unsupported geometry type.",__FILE__,__LINE__);
                break;
        }

    }
    catch (ReconException &e)
    {
        msg.str("");
        msg<<"SetROI failed with a ReconException for "<<m_BackProjector->GetModule()->Name()<<"\n"<<e.what();
        logger(kipl::logging::Logger::LogError,msg.str());
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (ModuleException &e)
    {
            msg.str("");
            msg<<"SetROI failed with a ModuleException for "<<m_BackProjector->GetModule()->Name()<<"\n"<<e.what();
            logger(kipl::logging::Logger::LogError,msg.str());
            throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (kipl::base::KiplException &e)
    {
        msg.str("");
        msg<<"SetROI failed with a KiplException for "<<m_BackProjector->GetModule()->Name()<<"\n"<<e.what();
        logger(kipl::logging::Logger::LogError,msg.str());
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (std::exception &e)
    {
        msg.str("");
        msg<<"SetROI failed with an STL-exception for "<<m_BackProjector->GetModule()->Name()<<"\n"<<e.what();
        logger(kipl::logging::Logger::LogError,msg.str());
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (...)
    {
        msg.str("");
        msg<<"SetROI failed with an unknown exception for "<<m_BackProjector->GetModule()->Name();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }


	std::map<std::string, std::string> parameters;

	// Start processing
	kipl::profile::Timer timer;
    msg.str("");
    msg<<": Allocated preprocessors "<<m_PreprocList.size()<<" using "<<m_BackProjector->GetModule()->Name()<<"\n"
		<<"Arc=["<<m_Config.ProjectionInfo.fScanArc[0]<<", "<<m_Config.ProjectionInfo.fScanArc[1]<<"]"<<"\n"
		<<"Target matrix "<<m_BackProjector->GetModule()->GetVolume();

	logger(kipl::logging::Logger::LogMessage,msg.str());

	timer.Tic();

    kipl::base::TImage<float,3> ext_projections;
    msg.str("");
    msg<<": Reading Projections. ROI=["<<extroi[0]<<", "<<extroi[1]<<", "<<extroi[2]<<", "<<extroi[3]<<"]";
    logger(kipl::logging::Logger::LogMessage,msg.str());

	msg.str("");


    try
    {
        ext_projections=m_ProjectionReader.Read(m_Config,extroi,parameters);
        validateImage(ext_projections.GetDataPtr(),ext_projections.Size(),"post reader");
	}
    catch (ReconException &e)
    {
        msg<<"Reading projections failed with a recon exception: "<<std::endl<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
        msg<<"Reading projections failed with a kipl exception: "<<std::endl<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
        msg<<"Reading projections failed with a STL exception: "<<std::endl<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (...)
    {
        msg<<"Reading projections failed with an unsupported exception: ";
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }

    msg.str("");
    msg<<": Size of read projections using ext roi: "<<ext_projections;
    logger(logger.LogMessage,msg.str());

	logger(kipl::logging::Logger::LogMessage,"Starting preprocessing");

    try
    {
        for (it_Module=m_PreprocList.begin(); it_Module!=m_PreprocList.end(); it_Module++)
        {
			msg.str("");
			msg<<"Processing: "<<(*it_Module)->GetModule()->ModuleName();
			logger(kipl::logging::Logger::LogMessage,msg.str());
			if (!(m_bCancel=UpdateProgress(0.0f, msg.str())))
                (*it_Module)->GetModule()->Process(ext_projections,parameters);
			else
				break;
            validateImage(ext_projections.GetDataPtr(),ext_projections.Size(),(*it_Module)->GetModule()->ModuleName());
		}
	}
    catch (ReconException &e)
    {
		msg<<"Preprocessing failed with a recon exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (kipl::base::KiplException &e)
    {
		msg<<"Preprocessing failed with a kipl exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (std::exception &e)
    {
		msg<<"Preprocessing failed with an STL exception: "<<e.what();
		throw ReconException(msg.str(),__FILE__,__LINE__);
	}
    catch (...)
    {
        msg<<"Preprocessing failed with an unsupported exception: ";
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
	
    kipl::base::TImage<float,3> projections;
    size_t dims[3];

    if (m_ProjectionMargin!=0)
    { // Remove padding
        dims[0]=ext_projections.Size(0);
        dims[1]=ext_projections.Size(1)-(roi[1]!=extroi[1] ? m_ProjectionMargin : 0) - (roi[3]!=extroi[3] ? m_ProjectionMargin : 0);
        dims[2]=ext_projections.Size(2);
        projections.Resize(dims);

        msg.str("");
        msg<<"ext: "<<ext_projections<<", proj: "<<projections;
        logger(logger.LogMessage,msg.str());
        for (size_t i=0; i<projections.Size(2); i++)
            memcpy(projections.GetLinePtr(0,i),ext_projections.GetLinePtr((roi[1]!=extroi[1] ? m_ProjectionMargin : 0),i), projections.Size(0) * projections.Size(1) * sizeof(float));
    }
    else
    {
        projections=ext_projections;
    }

    if (m_Config.MatrixInfo.bAutomaticSerialize==false) // Don't store the projections for the reconstruction to disk case
    {
//        m_ProjectionBlocks.push_back(ProjectionBlock(projections,roi,parameters));
        switch (m_Config.ProjectionInfo.beamgeometry)
        {
            case ReconConfig::cProjections::BeamGeometry_Parallel:
                m_ProjectionBlocks.push_back(ProjectionBlock(projections,roi,parameters));
                break;
            case ReconConfig::cProjections::BeamGeometry_Cone:
                m_ProjectionBlocks.push_back(ProjectionBlock(projections,CBroi,parameters));
                break;
            case ReconConfig::cProjections::BeamGeometry_Helix:
                logger(logger.LogError,"Helix is not supported by the engine.");
                throw ReconException("Helix is not supported by the engine",__FILE__,__LINE__);
                break;
            default:
                logger(logger.LogError,"Unsupported geometry type.");
                throw ReconException("Unsupported geometry type.",__FILE__,__LINE__);
                break;
        }
    }

    int res=0;

    try
    {
        switch (m_Config.ProjectionInfo.beamgeometry)
        {
            case ReconConfig::cProjections::BeamGeometry_Parallel:
                BackProject3D(projections,roi,parameters);
                break;
            case ReconConfig::cProjections::BeamGeometry_Cone:
                BackProject3D(projections,CBroi,parameters);
                break;
            case ReconConfig::cProjections::BeamGeometry_Helix:
                logger(logger.LogError,"Helix is not supported by the engine.");
                throw ReconException("Helix is not supported by the engine",__FILE__,__LINE__);
                break;
            default:
                logger(logger.LogError,"Unsupported geometry type.");
                throw ReconException("Unsupported geometry type.",__FILE__,__LINE__);
                break;
        }


    }
    catch (ReconException &e)
    {
        msg<<"BackProject3D failed with a recon exception: "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (kipl::base::KiplException &e)
    {
        msg<<"BackProject3D failed with a kipl exception: "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (std::exception &e)
    {
        msg<<"BackProject3D failed with an STL exception: "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }

	logger(kipl::logging::Logger::LogVerbose,"Done process 3D.");

    return res;
}

int ReconEngine::ProcessExistingProjections3D(size_t *roi)
{
    std::stringstream msg;
    std::list<ProjectionBlock>::iterator it;
    int i=0;
    int res=0;

    try
    {
        for (it=m_ProjectionBlocks.begin(); it!=m_ProjectionBlocks.end(); ++it, ++i)
        {
            msg.str("");
            msg<<"Back-projecting projection block "<<i+1;
            logger(kipl::logging::Logger::LogMessage,msg.str());
            m_BackProjector->GetModule()->SetROI(it->roi);
            m_Interactor->SetOverallProgress(float(i)/float(m_ProjectionBlocks.size()));

            res=BackProject3D(it->projections,it->roi,it->parameters);
            validateImage(it->projections.GetDataPtr(),it->projections.Size(),"Projections post recon block ProcessExistingProjections3D");
        }
    }
    catch (ReconException &e)
    {
        msg<<"BackProjection of preprocessed block "<<i<<" failed with a recon exception: "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (kipl::base::KiplException &e)
    {
        msg<<"BackProjection of preprocessed block "<<i<<" failed with a kipl exception: "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }
    catch (std::exception &e)
    {
        msg<<"BackProjection of preprocessed block "<<i<<" failed with an STL exception: "<<e.what();
        throw ReconException(msg.str(),__FILE__,__LINE__);
    }


    return res;
}

int ReconEngine::BackProject3D(kipl::base::TImage<float,3> & projections, size_t *roi, std::map<std::string, std::string> parameters)
{
    std::stringstream msg;

    msg<<"Got projections:"<<projections<<" with ROI=["<<roi[0]<<", "<<roi[1]<<", "<<roi[2]<<", "<<roi[3]<<"]";
    logger(kipl::logging::Logger::LogMessage,msg.str());
    msg.str("");
    m_BackProjector->GetModule()->SetROI(roi);
    if (!UpdateProgress(0.2f, "Back projection"))
    {
        try {
            logger(kipl::logging::Logger::LogMessage,"Back projection started.");
            m_BackProjector->GetModule()->Process(projections,parameters);
            logger(kipl::logging::Logger::LogMessage,"Back projection done.");
        }
        catch (ReconException &e) {
            msg<<"Back-projection failed with a recon exception: "<<e.what();
            throw ReconException(msg.str(),__FILE__,__LINE__);
        }
        catch (kipl::base::KiplException &e) {
            msg<<"Back-projection failed with a kipl exception: "<<e.what();
            throw ReconException(msg.str(),__FILE__,__LINE__);
        }
        catch (std::exception &e) {
            msg<<"Back-projection failed with an STL exception: "<<e.what();
            throw ReconException(msg.str(),__FILE__,__LINE__);
        }
    }

    validateImage(projections.GetDataPtr(),projections.Size(),"Projections post recon BackProject3D");

    if (UpdateProgress(1.0f, "Finalizing"))
    {
        logger(kipl::logging::Logger::LogMessage,"Reconstruction was canceled by the user.");
        return 1;
    }
    else
    {
        logger(kipl::logging::Logger::LogMessage,"Reconstruction finished");

        size_t dims[3];

        if (m_Config.MatrixInfo.bAutomaticSerialize==true)
            Serialize(dims);
        else
        {
            TransferMatrix(roi);
        }
    }

    return 0;
}

bool ReconEngine::UpdateProgress(float val, std::string msg)
{
    if (m_Interactor!=nullptr)
		return m_Interactor->SetProgress(val, msg);

    return false;
}

size_t ReconEngine::validateImage(float *data, size_t N, const string &description)
{
    if (m_Config.System.bValidateData)
    {
        std::ostringstream msg;
        size_t cntInf=0;
        size_t cntNan=0;
        size_t cnt=0;

        cnt=kipl::algorithms::dataValidator(data,N,cntNan,cntInf);
        if (cnt!=0)
        {
            msg.str(""); msg<<"Encountered "<<cntInf<<" infs and "<<cntNan<<" NaNs in "<<description;
            logger.warning(msg.str());
        }
    }
}

void ReconEngine::Done()
{
    if (m_Interactor!=nullptr)
		m_Interactor->Done();
}

void ReconEngine::MakeExtendedROI(size_t *roi, size_t margin, size_t *extroi, size_t *margins)
{

}

void ReconEngine::UnpadProjections(kipl::base::TImage<float,3> &projections, size_t *roi, size_t *margins)
{

}

//==========================================
// ProjectionBlock

ProjectionBlock::ProjectionBlock()
{

}

ProjectionBlock::ProjectionBlock(kipl::base::TImage<float,3> & proj, size_t *r,std::map<std::string, std::string> pars) :
    projections(proj),
    parameters(pars)
{
    projections.Clone();
    roi[0]=r[0];
    roi[1]=r[1];
    roi[2]=r[2];
    roi[3]=r[3];
}

ProjectionBlock::ProjectionBlock(const ProjectionBlock &b):
    projections(b.projections),
    parameters(b.parameters)
{
    projections.Clone();
    roi[0]=b.roi[0];
    roi[1]=b.roi[1];
    roi[2]=b.roi[2];
    roi[3]=b.roi[3];
}

ProjectionBlock & ProjectionBlock::operator=(const ProjectionBlock &b)
{
    projections=b.projections;
    projections.Clone();

    parameters=b.parameters;

    roi[0]=b.roi[0];
    roi[1]=b.roi[1];
    roi[2]=b.roi[2];
    roi[3]=b.roi[3];

    return *this;
}


ProjectionBlock::~ProjectionBlock()
{
}

