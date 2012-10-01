#include "guillotine.h"

Guillotine::Guillotine(Guillotine* _head)
{
    if(_head)
        head = _head;
    else
        head = this;
    image = NULL;
    leaf = true;
    child[0] = NULL;
    child[1] = NULL;
}

void Guillotine::delGuillotine()
{
    if(!leaf)
    {
        child[0]->delGuillotine();
        child[1]->delGuillotine();
    }
    delete this;
    //qDebug("%d", this);
}

void Guillotine::heuristic(QImage * img, int lvl)
{
    if(leaf)
    {
        int m = 0;
        switch (head->heuristicMethod)
        {
            case ImagePacker::NONE:
                head->heurBestFit = NULL;
                return;
            case ImagePacker::TL:
                m = rc.x() + rc.y();
                break;
            case ImagePacker::BAF:
                m = rc.width() * rc.height();
                break;
            case ImagePacker::BSSF:
                m = qMin(rc.width() - img->width(), rc.height() - img->height());
                break;
            case ImagePacker::BLSF:
                m = qMax(rc.width() - img->width(), rc.height() - img->height());
                break;
            case ImagePacker::MINW:
                m = rc.width();
                break;
            case ImagePacker::MINH:
                m = rc.height();
        }

        //int m = qMin(rc.width() - img->width(), rc.height() - img->height());
        //int m = (rc.height() * rc.width() - img->width() * img->height());
        if (img->width() <= rc.width() && img->height() <= rc.height() &&
            m < head->heurMin && !image)
        {
            head->heurMin = m;
            head->heurBestFit = this;
        }
    }
    else
    {
       child[0]->heuristic(img, lvl + 1);
       child[1]->heuristic(img, lvl + 1);
    }
}

Guillotine* Guillotine::insertNode(QImage * img)
{
    int x, y;
    //head->duplicate = false;
    if (!leaf)//we're not a leaf then
    {
        //qDebug("123");
        //(try inserting into first child)
        Guillotine* node = child[0]->insertNode(img);
            if (node) return node;
        //(no room, insert into second)
        return child[1]->insertNode(img);
    }
    else
    {
        //qDebug("123");
        //(if there's already a lightmap here, return)
        //qDebug("%d", image);
        //if (image)
        //{
            //qDebug("123");
            //if(head->packer->compareImages(image, img, &x, &y))
            //{
            //    head->duplicate = true;
            //    return this;
            //}
            //return NULL;
        //}
        //(if we're too small, return)
        if (img->width() > rc.width() || img->height() > rc.height())
            return NULL;
        //(if we're just right, accept)
        if (img->width() == rc.width() && img->height() == rc.height())
        {
            image = img;
            return this;
        }
        //heuristics
        head->heurBestFit = NULL;
        head->heurMin = 99999999;
        head->heurMax = 0;
        head->heuristic(img);
        Guillotine * p = head->heurBestFit;
        if(!p)
            p = this;
        //(otherwise, gotta split this Guillotine and create some kids)
        p->child[0] = new Guillotine(head);
        p->child[1] = new Guillotine(head);
        p->leaf = false;
        //(decide which way to split)
        int dw = p->rc.width() - img->width();
        int dh = p->rc.height() - img->height();

        if (dw > dh)
        {
            p->child[0]->rc = QRect(p->rc.x(), p->rc.y(), img->width(), p->rc.height());
            p->child[1]->rc = QRect(p->rc.x() + img->width(), p->rc.y(), dw, p->rc.height());
        }
        else
        {
            p->child[0]->rc = QRect(p->rc.x(), p->rc.y(), p->rc.width(), img->height());
            p->child[1]->rc = QRect(p->rc.x(), p->rc.y() + img->height(), p->rc.width(), dh);
        }
        //(insert into first child we created)
        return p->child[0]->insertNode(img);
    }
}
