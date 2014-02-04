#include "imagepacker.h"
#include <QHash>
#define CMPF1(x,y,a) qAlpha(img.pixel(x,y)) > a
#define CMP(x,y,a) if(CMPF1(x,y,a)) {t = false; break;} if(!t) break;
#define CMPF2(x,y) img.pixel(x,y) != pix
#define CMP2(x,y) if(CMPF2(x,y)) {t = false; break;} if(!t) break;

//auto-cropping algorithm
QRect ImagePacker::crop(const QImage &img)
{
    int j, w, h, x, y;
    QRgb pix;
    //QImage im;
    bool t;
    //crop all
    if(false)
    {
        //    qDebug("%d", img->depth());
        QHash<QRgb, int> hash;
        hash[img.pixel(0, 0)]++;
        hash[img.pixel(img.width() - 1, 0)]++;
        hash[img.pixel(img.width() - 1, img.height() - 1)]++;
        hash[img.pixel(0, img.height() - 1)]++;
        QHashIterator<QRgb, int> i(hash);
        while(i.hasNext())
        {
            i.next();
            //        qDebug("%d %d %d %d - %d", qRed(i.key()), qGreen(i.key()), qBlue(i.key()), qAlpha(i.key()), i.value());
            if(i.value() > 2)
            {
                pix = i.key();
                //            qDebug("%d %d %d %d - %d", qRed(i.key()), qGreen(i.key()), qBlue(i.key()), qAlpha(i.key()), i.value());
                //~ if(qAlpha(pix) == 0) break;
                t = true;
                for(y = 0; y < img.height(); y++)
                {
                    for(j = 0; j < img.width(); j++)
                        CMP2(j, y)
                    }
                t = true;
                for(x = 0; x < img.width(); x++)
                {
                    for(j = y; j < img.height(); j++)
                        CMP2(x, j)
                    }
                t = true;
                for(w = img.width(); w > 0; w--)
                {
                    for(j = y; j < img.height(); j++)
                        CMP2(w - 1, j)
                    }
                t = true;
                for(h = img.height(); h > 0; h--)
                {
                    for(j = x; j < w; j++)
                        CMP2(j, h - 1)
                    }
                goto found_by_color;
            }
        }
    }
    //crop only alpha
    t = true;
    for(y = 0; y < img.height(); y++)
    {
        for(j = 0; j < img.width(); j++)
            CMP(j, y, cropThreshold)
        }
    t = true;
    for(x = 0; x < img.width(); x++)
    {
        for(j = y; j < img.height(); j++)
            CMP(x, j, cropThreshold)
        }
    t = true;
    for(w = img.width(); w > 0; w--)
    {
        for(j = y; j < img.height(); j++)
            CMP(w - 1, j, cropThreshold)
        }
    t = true;
    for(h = img.height(); h > 0; h--)
    {
        for(j = x; j < w; j++)
            CMP(j, h - 1, cropThreshold)
        }
found_by_color:
    w = w - x;
    h = h - y;
    if(w < 0)
    {
        w = 0;
    }
    if(h < 0)
    {
        h = 0;
    }
    return QRect(x, y, w, h);
}
