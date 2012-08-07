#include "imagepacker.h"

//auto-cropping algorithm
QRect ImagePacker::crop(QImage *img)
{
    int i, j, w, h, x, y;
    QRgb pix;
    //QImage im;
    bool t;
    pix = img->pixel(0,0);
    t = true;
    //top trimming
    for(y = 0; y < img->height(); y++)
    {
            for(j = 0; j < img->width(); j++)
                    if(qAlpha(img->pixel(j,y)) > 0) {t = false; break;}
            if(!t) break;
    }
    t = true;
    //left
    for(x = 0; x < img->width(); x++){
            for(j = y; j < img->height(); j++)
                    if(qAlpha(img->pixel(x,j)) > 0) {t = false; break;}
            if(!t) break;
    }
    t = true;
    //right
    for(w = img->width(); w > 0; w--){
            for(j = y; j < img->height(); j++)
                    if(qAlpha(img->pixel(w-1,j)) > 0) {t = false; break;}
            if(!t) break;
    }
    t = true;
    //if(w == 0) h = 0;
    //else
    {
        //bottom
        for(h = img->height(); h > 0; h--){
            for(j = x; j < w; j++)
                if(qAlpha(img->pixel(j,h-1)) > 0) {t = false; break;}
            if(!t) break;
        }
    }
    w = w - x;
    h = h - y;
    if(w < 0) w = 0;
    if(h < 0) h = 0;
    return QRect(x, y, w, h);
}
