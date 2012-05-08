#include "imagepacker.h"

bool ImageCompareByHeight(const inputImage i1, const inputImage i2)
{
    return (i1.sizeCurrent.height() << 10) + i1.sizeCurrent.width() > (i2.sizeCurrent.height() << 10) +i2.sizeCurrent.width();
}
bool ImageCompareByWidth(const inputImage i1, const inputImage i2)
{
    return (i1.sizeCurrent.width() << 10) + i1.sizeCurrent.height() > (i2.sizeCurrent.width() << 10) + i2.sizeCurrent.height();
}
bool ImageCompareByArea(const inputImage i1, const inputImage i2)
{
    return i1.sizeCurrent.height() * i1.sizeCurrent.width() > i2.sizeCurrent.height() * i2.sizeCurrent.width();
}

void ImagePacker::sort()
{
    switch(sortOrder)
    {
    case 1:
        qSort(images.begin(), images.end(), ImageCompareByWidth);
        break;
    case 2:
        qSort(images.begin(), images.end(), ImageCompareByHeight);
        break;
    case 3:
        qSort(images.begin(), images.end(), ImageCompareByArea);
        break;
    }
}

