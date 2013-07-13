#include "view.h"
#include <QDebug>
View::View(QWidget *parent)
{
    scale = 1;
}

void View::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    this->setMinimumSize(size.width()*scale, (size.height() + 10)*textures.count()*scale);
    for(int i = 0; i < textures.count(); i++)
    {
        painter.fillRect(0, i * (size.height() + 10)*scale, textures.at(i).width()*scale, textures.at(i).height()*scale, Qt::magenta);
        painter.drawPixmap(0, i * (size.height() + 10)*scale, textures.at(i).width()*scale, textures.at(i).height()*scale, textures.at(i));
    }
}


void View::updatePixmap(const QList<QImage> &images)
{
    textures.clear();
    QPixmap texture;
    for(int i = 0; i < images.count(); i++)
    {
        texture = QPixmap::fromImage(images.at(i));
        if(i == 0) size = texture.size();
        textures << texture;
    }
    this->setMinimumSize(size.width()*scale, (size.height() + 10)*images.count()*scale);
    update();
}

void View::rescale(QString s)
{
    bool ok;
    int sscale = atoi(s.toLatin1());
//    if (ok)
    {
//        scaleBox->setEditText(QString::number(sscale) + QString("%"));
        scale = (float) sscale/100;
    }
    this->update();
}
