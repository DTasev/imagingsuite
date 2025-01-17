#include "basicthresholddlg.h"
#include "ui_basicthresholddlg.h"

#include <QDebug>
#include <QLineSeries>
#include <QMessageBox>

#include <ParameterHandling.h>

#include <strings/miscstring.h>
#include <math/image_statistics.h>
#include <base/thistogram.h>

BasicThresholdDlg::BasicThresholdDlg(QWidget *parent) :
    ConfiguratorDialogBase("BasicThreshold", true, false, true,parent),
    ui(new Ui::BasicThresholdDlg),
    pOriginal(nullptr),
    m_fThreshold(0.0),
    m_nHistMax(0UL)
{
    ui->setupUi(this);
    ui->viewer->showDifference(false);
    ui->plot_histogram->hideLegend();
    ui->plot_histogram->setPointsVisible(0);
}

BasicThresholdDlg::~BasicThresholdDlg()
{
    delete ui;
}

void BasicThresholdDlg::UpdateDialog()
{
    ui->doubleSpinBox_threshold->setValue(m_fThreshold);
}

void BasicThresholdDlg::UpdateParameters()
{
    m_fThreshold = ui->doubleSpinBox_threshold->value();
}

void BasicThresholdDlg::ApplyParameters()
{
    float *pImg = pOriginal->GetDataPtr();
    size_t N=pOriginal->Size();
    float th=static_cast<float>(m_fThreshold);

    for (size_t i=0; i<N; i++) {
        bilevelImg[i]=static_cast<float>(th<pImg[i]);
    }

    ui->viewer->setImages(pOriginal,&bilevelImg);
}

void BasicThresholdDlg::UpdateParameterList(std::map<std::string, std::string> &parameters)
{
    parameters.clear();

    parameters["threshold"]=kipl::strings::value2string(m_fThreshold);
}

int BasicThresholdDlg::exec(ConfigBase *config, std::map<string, string> &parameters, kipl::base::TImage<float, 3> &img)
{
    pOriginal = &img;
    bilevelImg.Resize(img.Dims());

    const size_t N=512;
    size_t bins[N];
    float axis[N];

    kipl::base::Histogram(img.GetDataPtr(),img.Size(),bins,N,0.0f,0.0f,axis);
    m_nHistMax = *std::max_element(bins,bins+N);
    ui->doubleSpinBox_threshold->setMinimum(static_cast<double>(axis[0]));
    ui->doubleSpinBox_threshold->setMaximum(static_cast<double>(axis[N-1]));
    try {
        m_fThreshold = static_cast<double>(GetFloatParameter(parameters,"threshold"));

        ui->plot_histogram->setCurveData(0,axis,bins,N,"Histogram");
    }
    catch (std::bad_alloc & e) {
        QString msg="Failed to allocate series: ";
        msg=msg+QString::fromStdString(e.what());
        QMessageBox::warning(this,"Exception",msg);
        reject();
    }
    UpdateDialog();

    on_doubleSpinBox_threshold_valueChanged(m_fThreshold);

    int res=QDialog::exec();

    if (res==QDialog::Accepted) {
        logger(kipl::logging::Logger::LogMessage,"Use settings");
        UpdateParameters();
        UpdateParameterList(parameters);
        return true;
    }
    else
    {
        logger(kipl::logging::Logger::LogMessage,"Discard settings");
    }

    return 0;
}

void BasicThresholdDlg::on_doubleSpinBox_threshold_valueChanged(double arg1)
{

    QtCharts::QLineSeries *series=nullptr;
    try {
        series=new QtCharts::QLineSeries();
    }
    catch (std::bad_alloc & e) {
        QString msg="Failed to allocate series: ";
        msg=msg+QString::fromStdString(e.what());
        QMessageBox::warning(this,"Exception",msg);
        return;
    }

    series->append(static_cast<qreal>(arg1),static_cast<qreal>(0));
    series->append(static_cast<qreal>(arg1),static_cast<qreal>(m_nHistMax));
    series->setName("Threshold");

    ui->plot_histogram->setCurveData(1,series);
    m_fThreshold=arg1;
    ApplyParameters();
}
