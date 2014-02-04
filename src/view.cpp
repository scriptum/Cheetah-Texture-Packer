#include "view.h"
#include <QDebug>
View::View(QWidget * /* parent */)
{
    scale = 1;
}

void View::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    int minHeight = 0;
    for(int i = 0; i < textures.count(); i++)
    {
        minHeight += (textures[i].size().height() + 10) * scale;
    }
    this->setMinimumSize(size.width()*scale, minHeight);
    int pos = 0;
    for(int i = 0; i < textures.count(); i++)
    {
        painter.fillRect(0, pos, textures.at(i).width()*scale,
                         textures.at(i).height()*scale, Qt::magenta);
        painter.drawPixmap(0, pos, textures.at(i).width()*scale,
                           textures.at(i).height()*scale, textures.at(i));
        pos += (textures.at(i).height() + 10) * scale;
    }
}


void View::updatePixmap(const QList<QImage> &images)
{
    textures.clear();
    QPixmap texture;
    for(int i = 0; i < images.count(); i++)
    {
        texture = QPixmap::fromImage(images.at(i));
        if(i == 0)
        {
            size = texture.size();
        }
        textures << texture;
    }
    this->setMinimumSize(size.width()*scale,
                         (size.height() + 10)*images.count()*scale);
    update();
}

void View::rescale(QString s)
{
    scale = (float)s.remove('%').toInt() / 100.f;
    if(scale == 0.f)
    {
        scale = 1.f;
    }
    this->update();
}
