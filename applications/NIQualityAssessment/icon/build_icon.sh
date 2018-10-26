#!/bin/sh

if [$# -e 2]; then
    ICONDIR=$2.iconset
    ICONSRC=$1
else
    ICONDIR=$1.iconset
    ICONSRC=$1
fi


mkdir $ICONDIR

sips -z 16 16     $ICONSRC --out $ICONDIR/icon_16x16.png
sips -z 32 32     $ICONSRC --out $ICONDIR/icon_16x16@2x.png
sips -z 32 32     $ICONSRC --out $ICONDIR/icon_32x32.png
sips -z 64 64     $ICONSRC --out $ICONDIR/icon_32x32@2x.png
sips -z 128 128   $ICONSRC --out $ICONDIR/icon_128x128.png
sips -z 256 256   $ICONSRC --out $ICONDIR/icon_128x128@2x.png
sips -z 256 256   $ICONSRC --out $ICONDIR/icon_256x256.png
sips -z 512 512   $ICONSRC --out $ICONDIR/icon_256x256@2x.png
sips -z 512 512   $ICONSRC --out $ICONDIR/icon_512x512.png
cp $ICONSRC $ICONDIR/icon_512x512@2x.png
iconutil -c icns $ICONDIR
rm -R $ICONDIR