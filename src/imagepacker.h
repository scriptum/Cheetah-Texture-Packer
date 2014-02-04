#ifndef IMAGEPACKER_H
#define IMAGEPACKER_H

#include <QObject>
#include <QImage>

class MaxRects;

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
    quint32 hash;
    int textureId;
    void *id;
    void *duplicateId;
    QPoint pos;
    QSize size, sizeCurrent;
    QRect crop;
    QString path;

    bool cropped, rotated;
};

struct border_t
{
    int t, b, l, r;
};

class ImagePacker : public QObject
{
    private:
        int prevSortOrder;
        void internalPack(int heur, int w, int h);

        void SortImages(int w, int h);

    public:
        QList<inputImage> images;
        QList<QSize> bins;
        ImagePacker();
        bool compareImages(QImage *img1, QImage *img2, int *i, int *j);
        void pack(int heur, int w, int h);

        unsigned AddImgesToBins(int heur, int w, int h);

        void CropLastImage(int heur, int w, int h, bool wh);
        void DivideLastImage(int heur, int w, int h, bool wh);

        void UpdateCrop();

        float GetFillRate();

        void ClearBin(int binIndex);

        int FillBin(int heur, int w, int h, int binIndex);

        QRect crop(const QImage &img);
        void sort();
        void addItem(const QImage &img, void *data, QString path);
        void addItem(QString path, void *data);
        const inputImage *find(void *data);
        void removeId(void *);
        void realculateDuplicates();
        void clear();
        int compare;
        quint64 area, neededArea;
        int missingImages;
        int mergedImages;
        bool ltr, merge, square, autosize, mergeBF;
        int cropThreshold;
        border_t border;
        int extrude;
        int rotate;
        int sortOrder;
        int minFillRate;
        int minTextureSizeX;
        int minTextureSizeY;
        enum {GUILLOTINE, MAXRECTS}; //method
        enum {NONE, TL, BAF, BSSF, BLSF, MINW, MINH, HEURISTIC_NUM}; //heuristic
        enum {SORT_NONE, WIDTH, HEIGHT, SORT_AREA, SORT_MAX, SORT_NUM}; //sort
        enum {NEVER, ONLY_WHEN_NEEDED, H2_WIDTH_H, WIDTH_GREATHER_HEIGHT, WIDTH_GREATHER_2HEIGHT, W2_HEIGHT_W, HEIGHT_GREATHER_WIDTH, HEIGHT_GREATHER_2WIDTH, ROTATION_NUM}; //rotation
};




#endif // IMAGEPACKER_H
