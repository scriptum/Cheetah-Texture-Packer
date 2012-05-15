#ifndef IMAGEPACKER_H
#define IMAGEPACKER_H

#include <QObject>
#include <QImage>

struct packedImage
{
    QImage img;
    QRect rc;
    QRect crop;
    bool border, rotate;
    int textureId;
    int id;
};

struct inputImage
{
    u_int32_t hash;
    int textureId;
    void *id;
    void *duplicateId;
    QPoint pos;
    QSize size, sizeCurrent;
    QRect crop;

    bool cropped, rotated;
};
class ImagePacker : public QObject
{
private:
    int prevSortOrder;
public:
    QList<inputImage> images;
    QList<QSize> bins;
    ImagePacker();
    bool compareImages(QImage* img1, QImage* img2, int* i, int *j);
    void pack(int packMethod, int heuristic, uint w = 512, uint h = 512);
    QRect crop(QImage*);
    void sort();
    void addItem(QImage img, void * data);
    void addItem(QString path, void * data);
    const inputImage *find(void * data);
    void removeId(void*);
    void realculateDuplicates();
    void clear();
    int compare;
    quint64 area, summArea, neededArea;
    int missingImages;
    int mergedImages;
    bool ltr, trim, merge, mergeBF;
    unsigned int borderTop, borderBottom, borderLeft, borderRight, rotate;
    int sortOrder;
    enum {GUILLOTINE, MAXRECTS}; //method
    enum {NONE, TL, BAF, BSSF, BLSF, MINW, MINH, HEURISTIC_NUM}; //heuristic
    enum {SORT_NONE, WIDTH, HEIGHT, SORT_NUM}; //sort
    enum {NEVER, H2_WIDTH_H, WIDTH_GREATHER_HEIGHT, WIDTH_GREATHER_2HEIGHT, W2_HEIGHT_W, HEIGHT_GREATHER_WIDTH, HEIGHT_GREATHER_2WIDTH, ROTATION_NUM}; //rotation
};




#endif // IMAGEPACKER_H
