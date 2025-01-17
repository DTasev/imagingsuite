#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include "QtAddons_global.h"

#include <QRect>
#include <QColor>
#include <QWidget>
#include <QRubberBand>
#include <QResizeEvent>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QList>
#include <QSet>
#include <QMutex>

#include <logging/logger.h>

#include "imagepainter.h"
#include "plotter.h"
#include "imageviewerinfodialog.h"
#include "qglyphs.h"
#include "qmarker.h"

namespace QtAddons {

class QTADDONSSHARED_EXPORT ImageViewerWidget : public QWidget
{
    Q_OBJECT

    kipl::logging::Logger logger;
    static QList<ImageViewerWidget *> s_ViewerList;
    static int m_nViewerCounter;
    QString m_sViewerName;
public:
    enum eViewerMouseModes {
        ViewerROI=0,
        ViewerZoom=1,
        ViewerPan=2,
        ViewerProfile=3
    };

    enum eRubberBandStatus
    {
        RubberBandHide=0,
        RubberBandFreeze=1,
        RubberBandDrag=2,
        RubberBandMove=3
    };

   // explicit ImageViewerWidget(QWidget *parent = 0);
    ImageViewerWidget(QWidget *parent = 0);
    ~ImageViewerWidget();

    void set_image(float const * const data, size_t const * const dims);
    void set_image(float const * const data, size_t const * const dims, const float low, const float high);
    void getImageDims(int &x, int &y);
    void set_plot(QVector<QPointF> data, QColor color, int idx);
    void clear_plot(int idx=-1);
    void set_rectangle(QRect rect, QColor color, int idx);
    void clear_rectangle(int idx=-1);
    void set_marker(QtAddons::QMarker marker, int idx);
    void clear_marker(int idx=-1);
    void hold_annotations(bool hold);

    void clear_viewer();
    void set_levels(const float level_low, const float level_high, bool updatelinked=true);
    void get_levels(float *level_low, float *level_high);
    void get_minmax(float *level_low, float *level_high);
    void show_clamped(bool show);
    QString viewerName();
    void set_viewerName(QString &name);
    QRect get_marked_roi();
    const QVector<QPointF> & get_histogram();

 //   void set_interpolation(Gdk::InterpType interp) {m_Interpolation=interp; queue_draw(); }

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void LinkImageViewer(QtAddons::ImageViewerWidget *w, bool connect=true);
    void ClearLinkedImageViewers(QtAddons::ImageViewerWidget *w=nullptr);
public slots:
//    void zoomIn();
//    void zoomOut();
    void ShowContextMenu(const QPoint& pos);
    void on_levelsChanged(float lo, float hi);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *);
    virtual void keyPressEvent(QKeyEvent *event);
//    void wheelEvent(QWheelEvent *event);
        
protected:
    void UpdateFromLinkedViewer(QtAddons::ImageViewerWidget *w);
    void UpdateLinkedViewers();

    ImagePainter m_ImagePainter;
    void updateRubberBandRegion();
    void showToolTip(QPoint position, QString message);
    void refreshPixmap();
    QRect rubberBandRect;
    QRubberBand m_rubberBandLine;
    QPoint m_rubberBandOrigin;
    eRubberBandStatus m_RubberBandStatus;
    QRect roiRect;
    eViewerMouseModes m_MouseMode;
    Qt::MouseButton m_PressedButton;
    QPoint m_LastMotionPosition;
    QSize widgetSize;
    QSet<ImageViewerWidget *> m_LinkedViewers;
    enum { Margin = 5 };
    ImageViewerInfoDialog m_infoDialog;
    QMutex m_MouseMoveMutex;
    QMutex m_ImageMutex;

signals:
    void newImageDims(const QRect &rect);
    void levelsChanged(float lo, float hi);
};


}

#endif // IMAGEVIEWERWIDGET_H
