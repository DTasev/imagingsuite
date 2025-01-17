#ifndef MERGEVOLUMESDIALOG_H
#define MERGEVOLUMESDIALOG_H

#include <QDialog>

#include <base/timage.h>
#include <logging/logger.h>

#include "mergevolume.h"


namespace Ui {
class MergeVolumesDialog;
}

class MergeVolumesDialog : public QDialog
{
    Q_OBJECT
    kipl::logging::Logger logger;
public:
    explicit MergeVolumesDialog(QWidget *parent = nullptr);
    ~MergeVolumesDialog();

private slots:
    void on_pushButton_loaddata_clicked();

    void on_pushButton_loadA_clicked();

    void on_pushButton_loadB_clicked();

    void on_comboBox_mixorder_currentIndexChanged(int index);

    void on_pushButton_browseout_clicked();

    void on_pushButton_startmerge_clicked();

    void on_comboBox_result_currentIndexChanged(int index);

    void on_pushButton_TestMix_clicked();

protected:
    void updateDialog();
    void updateConfig();
    void saveConfig();
    void loadConfig();
    bool checkImageSizes();

    void loadVerticalSlice(std::string filemask,
                                        int first,
                                        int last,
                                        kipl::base::TImage<float,2> *img);

    MergeVolume m_merger;
    kipl::base::TImage<float,2> m_VerticalImgA;
    kipl::base::TImage<float,2> m_VerticalImgB;
    kipl::base::TImage<float,2> m_VerticalImgResult;
    kipl::base::TImage<float,2> m_VerticalImgLocalResult;
    kipl::base::TImage<float,2> m_HorizontalSliceResult;


private:
    Ui::MergeVolumesDialog *ui;
};

#endif // MERGEVOLUMESDIALOG_H
