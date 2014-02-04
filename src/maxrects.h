#ifndef MAXRECTS_H
#define MAXRECTS_H
#include <QImage>
#include "imagepacker.h"

struct trbl
{
    QPoint t, r, b, l;
};

struct MaxRectsNode
{
    QRect r; //rect
    //    QSize *i; //image
    trbl b; //border
};
class MaxRects
{
    public:
        MaxRects();
        QList<MaxRectsNode> F;
        QList<QRect> R;
        QList<MaxRectsNode *> FR;
        QPoint insertNode(inputImage *);
        int heuristic, w, h, rotation;
        bool leftToRight;
        border_t *border;
};

#endif // MAXRECTS_H
