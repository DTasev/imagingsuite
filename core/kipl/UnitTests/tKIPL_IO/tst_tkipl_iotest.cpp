#include <sstream>

#include <QString>
#include <QtTest>

#include <base/timage.h>
#include <io/io_tiff.h>

class tKIPL_IOTest : public QObject
{
    Q_OBJECT

public:
    tKIPL_IOTest();

private Q_SLOTS:
    void testBasicReadWriteTIFF();
};

tKIPL_IOTest::tKIPL_IOTest()
{
}

void tKIPL_IOTest::testBasicReadWriteTIFF()
{
    size_t dims[2]={100,50};

    kipl::base::TImage<float,2> fimg(dims);
    fimg.info.sArtist="UnitTest";
    fimg.info.sCopyright="none";
    fimg.info.sDescription="slope = 1.0E0 \noffset = 0.0E0";
    fimg.info.SetDPCMX(123.0f);
    fimg.info.SetDPCMY(321.0f);

    for (size_t i=0; i<fimg.Size(); i++)
        fimg[i]=static_cast<float>(i);

    kipl::io::WriteTIFF(fimg,"basicRW.tif");

    kipl::base::TImage<float,2> resimg;

    kipl::io::ReadTIFF(resimg,"basicRW.tif");
    std::ostringstream msg;
    QVERIFY2(fimg.Size(0)==resimg.Size(0),"X size missmatch");
    QVERIFY2(fimg.Size(1)==resimg.Size(1),"Y size missmatch");

    QVERIFY2(fimg.info.sArtist==resimg.info.sArtist,"Artist information error");
    QVERIFY2(fimg.info.sDescription==resimg.info.sDescription,"Description error");
    QVERIFY2(fimg.info.sCopyright==resimg.info.sCopyright,"Copyright error");
    QVERIFY2(fimg.info.GetDPCMX()==resimg.info.GetDPCMX(),"X Resolution error");
    msg<<"Y resolution error: fimg="<<fimg.info.GetDPCMY()<<", resimg="<<resimg.info.GetDPCMY();
    QVERIFY2(fimg.info.GetDPCMY()==resimg.info.GetDPCMY(),msg.str().c_str());

    for (size_t i=0; i<fimg.Size(); i++) {
        QVERIFY2(fimg[i]==resimg[i],"Pixels not similar");
    }

}

QTEST_APPLESS_MAIN(tKIPL_IOTest)

#include "tst_tkipl_iotest.moc"
