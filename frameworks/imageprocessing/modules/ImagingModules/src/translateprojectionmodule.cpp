//
// This file is part of the i KIPL image processing tool by Anders Kaestner
// (c) 2008,2009,2010,2011,2012,2013 Anders Kaestner
// Distribution is only allowed with the permission of the author.
//
// Revision information
// $Author: kaestner $
// $Date: 2013-08-15 21:58:23 +0200 (Thu, 15 Aug 2013) $
// $Rev: 1481 $
//

#include "translateprojectionmodule.h"
#include <ParameterHandling.h>
#include <strings/miscstring.h>

TranslateProjectionModule::TranslateProjectionModule() : KiplProcessModuleBase("TranslateProjectionModule",false),
    m_fSlope(1.0f),
    m_fIntercept(0.0f),
    m_eSampler(ImagingAlgorithms::TranslateLinear),
    m_fParameter(1.0f)
{
}

TranslateProjectionModule::~TranslateProjectionModule()
{}

int TranslateProjectionModule::Configure(KiplProcessConfig config, std::map<std::string, std::string> parameters)
{
    m_fSlope          = GetFloatParameter(parameters,"slope");
    m_fIntercept      = GetFloatParameter(parameters,"intercept");
    m_fParameter      = GetFloatParameter(parameters,"parameter");
    string2enum(GetStringParameter(parameters,"sampler"), m_eSampler);

    return 0;
}

std::map<std::string, std::string> TranslateProjectionModule::GetParameters()
{
    std::map<std::string, std::string> parameters;

    parameters["slope"]     = kipl::strings::value2string(m_fSlope);
    parameters["intercept"] = kipl::strings::value2string(m_fIntercept);
    parameters["parameter"] = kipl::strings::value2string(m_fParameter);
    parameters["sampler"]=enum2string(m_eSampler);

    return parameters;
}

int TranslateProjectionModule::ProcessCore(kipl::base::TImage<float,3> & img, std::map<std::string, std::string> & coeff)
{
    ImagingAlgorithms::TranslateProjections translater;

    translater.translate(img,m_fSlope,m_fIntercept,m_eSampler,m_fParameter);

    return 0;
}
