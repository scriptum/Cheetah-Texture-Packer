#include "imagepacker.h"
#include <QDebug>
#include "maxrects.h"
ImagePacker::ImagePacker()
{
    prevSortOrder = -1;
}

//pack images, return list of positions
void ImagePacker::pack(int heur, int w, int h)
{
    realculateDuplicates();
    summArea = 0;
    QSize size;
    for(int i = 0; i < images.size(); i++)
    {
        images.operator [](i).pos = QPoint(999999, 999999);
        if(trim)
            size = images.at(i).crop.size();
        else
            size = images.at(i).size;
        if(size.width() == w) size.setWidth(size.width() - border.l - border.r);
        if(size.height() == h) size.setHeight(size.height() - border.t - border.b);
        size += QSize(border.l + border.r, border.t + border.b);

        images.operator [](i).rotated = false;
        if((rotate == WIDTH_GREATHER_HEIGHT && size.width() > size.height()) ||
           (rotate == WIDTH_GREATHER_2HEIGHT && size.width() > 2 * size.height()) ||
           (rotate == HEIGHT_GREATHER_WIDTH && size.height() > size.width()) ||
           (rotate == H2_WIDTH_H && size.height() > size.width() && size.width() * 2 > size.height()) ||
           (rotate == W2_HEIGHT_W && size.width() > size.height() && size.height() * 2 > size.width()) ||
           (rotate == HEIGHT_GREATHER_2WIDTH && size.height() > 2 * size.width()))
        {
            size.transpose();
            images.operator [](i).rotated = true;
        }
        images.operator [](i).sizeCurrent = size;
        if(images.at(i).duplicateId == NULL || !merge)
            summArea += size.width() * size.height();
    }
    sort();
    missingImages = 1;
    mergedImages = 0;
    area = 0;
    neededArea = 0;
    bins.clear();
    QList<MaxRects> binsBuffer;
    MaxRects *currentBin;
    int binIndex = 0;
    int added = 1;
    int areaBuf = 0;
    int W, H;
    bool repeat = false;
    bool WH = true;
    while(missingImages && added)
    {
        if(!repeat)
        {
            H = 32;
            W = 16;
            WH = true;
            summArea -= areaBuf;
        }
        repeat = false;
        while(summArea > W * H && W * H < w * h)
        {
            WH = !WH;
            if(WH)
            {
                H *= 2;
                if(H > h)
                    H = h;
            }
            else
            {
                W *= 2;
                if(W > w)
                    W = w;
            }
        }
        missingImages = 0;
        MaxRects rects;
        MaxRectsNode mrn;
        mrn.r = QRect(0, 0, W, H);
        rects.F << mrn;
        rects.heuristic = heur;
        rects.leftToRight = ltr;
        rects.w = W;
        rects.h = H;
        rects.rotation = rotate;
        rects.border = &border;
        binsBuffer << rects;
        bins << QSize(W,H);
        currentBin = &binsBuffer.last();
        added = 0;
        areaBuf = 0;
        for(int i = 0; i < images.size(); i++)
        {
            if(QPoint(999999, 999999) != images.at(i).pos) continue;
            if(images.at(i).duplicateId == NULL || !merge)
            {
                images.operator [](i).pos = currentBin->insertNode(&images.operator [](i));
                images.operator [](i).textureId = binIndex;
                if(QPoint(999999, 999999) == images.at(i).pos)
                    missingImages++;
                else
                {
                    areaBuf += images.at(i).sizeCurrent.width() * images.at(i).sizeCurrent.height();
                    area += images.at(i).sizeCurrent.width() * images.at(i).sizeCurrent.height();
                    added++;
                }
                if(binIndex == 0)
                    neededArea += images.at(i).sizeCurrent.width() * images.at(i).sizeCurrent.height();
            }
        }
        if(W * H < w * h && missingImages)
        {
            for(int i = 0; i < images.size(); i++)
            {
                if(images.at(i).textureId == binIndex)
                    images.operator [](i).pos = QPoint(999999, 999999);
            }
            //summArea += areaBufCopy;
            //areaBuf = areaBufCopy;
            bins.removeLast();
            binsBuffer.removeLast();
            WH = !WH;
            if(WH)
            {
                H *= 2;
                if(H > h)
                    H = h;
            }
            else
            {
                W *= 2;
                if(W > w)
                    W = w;
            }
//            qDebug() <<"!"<< W << H;
            repeat = true;
            added = 1;
        }
        else
        {
            if(added == 0) bins.removeLast();
            binIndex++;
        }
    }
    if(merge)
        for(int i = 0; i < images.size(); i++)
            if(images.at(i).duplicateId != NULL)
            {
                images.operator [](i).pos = find(images.at(i).duplicateId)->pos;
                images.operator [](i).textureId = find(images.at(i).duplicateId)->textureId;
                mergedImages++;
            }
}

quint32 rc_crc32(quint32 crc, const uchar *buf, size_t len)
{
    static quint32 table[256];
    static int have_table = 0;
    quint32 rem, octet;
    int i, j;
    const uchar *p, *q;

    /* This check is not thread safe; there is no mutex. */
    if (have_table == 0) {
        /* Calculate CRC table. */
        for (i = 0; i < 256; i++) {
            rem = i;  /* remainder from polynomial division */
            for (j = 0; j < 8; j++) {
                if (rem & 1) {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                } else
                    rem >>= 1;
            }
            table[i] = rem;
        }
        have_table = 1;
    }

    crc = ~crc;
    q = buf + len;
    for (p = buf; p < q; p++) {
        octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}

void ImagePacker::addItem(QImage img, void * data)
{
    inputImage i;
    if(img.width() == 0 || img.height() == 0) return;
    i.hash = rc_crc32(0, img.bits(), img.byteCount());
    i.crop = crop(&img);
    i.size = img.size();
    i.id = data;
    images << i;
}

void ImagePacker::addItem(QString path, void * data)
{
    addItem(QImage(path), data);
}

void ImagePacker::clear()
{
    images.clear();
}

void ImagePacker::realculateDuplicates()
{
    for(int i = 0; i < images.count(); i++)
        images.operator [](i).duplicateId = NULL;
    for(int i = 0; i < images.count(); i++)
    {
        for(int k = i+1; k < images.count(); k++)
        {
            if(images.at(k).duplicateId == NULL &&
               images.at(i).hash == images.at(k).hash &&
               images.at(i).size == images.at(k).size &&
               images.at(i).crop == images.at(k).crop)
            {
                images.operator [](k).duplicateId = images.at(i).id;
            }
        }
    }
}

void ImagePacker::removeId(void *data)
{
    for(int k = 0; k < images.count(); k++)
    {
        if(images.at(k).id == data)
        {
            images.removeAt(k);
            break;
        }
    }
}
const inputImage *ImagePacker::find(void * data)
{
    for(int i = 0; i < images.count(); i++)
    {
        if(data == images.at(i).id)
            return &images.at(i);
    }
    return NULL;
}
