#ifndef GUILLOTINE_H
#define GUILLOTINE_H

#include <QImage>
#include "imagepacker.h"
class Guillotine
{
public:
    Guillotine(Guillotine* _head = 0);
    //~Guillotine();
    void delGuillotine();
    Guillotine* head;
    bool duplicate;
    Guillotine* insertNode(QImage * img);
    void heuristic(QImage * img, int lvl = 0);
    Guillotine* child[2];
    QRect rc;
    bool leaf;
    int heuristicMethod;
    QImage * image;
    ImagePacker* packer;
private:
    int heurMin, heurMax;
    Guillotine* heurBestFit;
};

#endif // GUILLOTINE_H
