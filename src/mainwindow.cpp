#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    exporting = false;
    ui->setupUi(this);
    connect(this, SIGNAL(renderedImage(QList<QImage>)), ui->widget, SLOT(updatePixmap(QList<QImage>)));
    ui->outDir->setText(QDir::homePath());
    exporting = false;
    ui->widget->scaleBox = ui->scale;
    tabifyDockWidget(ui->dockPreferences, ui->dockExport);
    ui->dockPreferences->raise();

    pattern = QPixmap(20,20);
    QPainter painter(&pattern);
#define BRIGHT 190
#define SHADOW 150
    painter.fillRect(0,0,10,10,QColor(SHADOW,SHADOW,SHADOW));
    painter.fillRect(10,0,10,10,QColor(BRIGHT,BRIGHT,BRIGHT));
    painter.fillRect(10,10,10,10,QColor(SHADOW,SHADOW,SHADOW));
    painter.fillRect(0,10,10,10,QColor(BRIGHT,BRIGHT,BRIGHT));
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RecurseDirectory(const QString &dir)
{
    QDir dirEnt(dir);
    QFileInfoList list = dirEnt.entryInfoList();
    for (int i = 0; i < list.count() && !recursiveLoaderDone;i++)
    {
        recursiveLoaderCounter++;
        QFileInfo info = list[i];

        QString filePath = info.filePath();
        QString fileExt = info.suffix().toLower();
        QString name = dir + QDir::separator();
        if (info.isDir())
        {
            // recursive
            if (info.fileName()!=".." && info.fileName()!=".")
                RecurseDirectory(filePath);
        }
        else if(imageExtensions.contains(fileExt))
        {
            if(!QFile::exists(name+info.completeBaseName()+QString(".atlas")))
            {
                ui->tilesList->addItem(filePath.replace(topImageDir, ""));
                packerData * data = new packerData;
                data->listItem = ui->tilesList->item(ui->tilesList->count() - 1);
                data->path = info.absoluteFilePath();
                packer.addItem(data->path, data);
            }
        }
        if(recursiveLoaderCounter == 500)
        {
            if(QMessageBox::No ==
                    QMessageBox::question(
                        this,
                        tr("Directory is too big"),
                        tr("It seems that directory <b>") + topImageDir +
                        tr("</b> is too big. "
                           "Loading may take HUGE amount of time and memory. "
                           "Please, check directory again. <br>"
                           "Do you want to continue?"),
                        QMessageBox::Yes,
                        QMessageBox::No))
            {
                recursiveLoaderDone = true;
                recursiveLoaderCounter++;
                continue;
            }
            ui->previewWithImages->setChecked(false);
        }
    }
}

void MainWindow::addDir(QString dir)
{
    //FIXME
    //this is messy hack due to difference between QFileDialog and QFileInfo dir separator in Windows
    if (QDir::separator() == '\\')
        topImageDir = dir.replace("\\","/") + "/";
    else
        topImageDir = dir + "/";
    ui->outDir->setText(dir);
    recursiveLoaderCounter = 0;
    recursiveLoaderDone = false;
    //packer.clear();
    RecurseDirectory(dir);
    QFileInfo info(dir);
    ui->outFile->setText(info.baseName());

}

void MainWindow::addTiles()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select tile directory"), topImageDir);
    if(dir.length() > 0)
    {
        addDir(dir);
        packerUpdate();
    }
}

void MainWindow::deleteSelectedTiles()
{
    QList<QListWidgetItem *> itemList = ui->tilesList->selectedItems();
    for (int i=0; i<itemList.size(); i++) {
        for (int j = 0; j < packer.images.size(); ++j) {
            if((static_cast<packerData*>(packer.images.at(j).id))->listItem == itemList[i])
            {
                delete (static_cast<packerData*>(packer.images.at(j).id));
                packer.images.removeAt(j);
            }
        }
    }
    qDeleteAll(ui->tilesList->selectedItems());
}

void MainWindow::packerUpdate()
{
    int i;
    quint64 area = 0;
    packer.sortOrder = ui->sortOrder->currentIndex();
    packer.border.t = ui->borderTop->value();
    packer.border.l = ui->borderLeft->value();
    packer.border.r = ui->borderRight->value();
    packer.border.b = ui->borderBottom->value();
    packer.extrude = ui->extrude->value();
    packer.merge = ui->merge->isChecked();
    packer.square = ui->square->isChecked();
    packer.autosize = ui->autosize->isChecked();
    packer.minFillRate = ui->minFillRate->value();
    packer.mergeBF = false;
    packer.rotate = ui->rotationStrategy->currentIndex();
    int textureWidth = ui->textureW->value(), textureHeight = ui->textureH->value();
    int heuristic = ui->comboHeuristic->currentIndex();
    QString outDir = ui->outDir->text();
    QString outFile = ui->outFile->text();
    QString outFormat = ui->outFormat->currentText();
    bool previewWithImages =ui->previewWithImages->isChecked();


    packer.pack(heuristic, textureWidth, textureHeight);

    QList<QImage> textures;
    for (i = 0; i < packer.bins.size(); i++)
    {
        QImage texture(packer.bins.at(i).width(), packer.bins.at(i).height(), QImage::Format_ARGB32);
        texture.fill(Qt::transparent);
        textures << texture;
    }
    if(exporting)
    {
        for(int j = 0; j < textures.count(); j++)
        {
            QString outputFile = outDir;
            outputFile += QDir::separator();
            outputFile += outFile;
            if(textures.count() > 1)
                outputFile += QString("_") + QString::number(j + 1);
            outputFile += ".atlas";
            QString imgFile = outFile;
            if(textures.count() > 1)
                imgFile += QString("_") + QString::number(j + 1);
            imgFile += ".";
            imgFile += outFormat.toLower();

            QFile positionsFile(outputFile);
            if (!positionsFile.open(QIODevice::WriteOnly | QIODevice::Text))
                QMessageBox::critical(0, tr("Error"), tr("Cannot create file ") + outputFile);
            else
            {
                QTextStream out(&positionsFile);
                out << "textures: " << imgFile << "\n";
                for (i = 0; i < packer.images.size(); i++)
                {
                    if(packer.images.at(i).textureId != j) continue;
                    QPoint pos(packer.images.at(i).pos.x() + packer.border.l + packer.extrude,
                               packer.images.at(i).pos.y() + packer.border.t + packer.extrude);
                    QSize size, sizeOrig;
                    QRect crop;
                    sizeOrig = packer.images.at(i).size;
                    if(!packer.cropThreshold)
                    {
                        size = packer.images.at(i).size;
                        crop = QRect(0,0,size.width(),size.height());
                    }
                    else
                    {
                        size = packer.images.at(i).crop.size();
                        crop = packer.images.at(i).crop;
                    }
                    if(packer.images.at(i).rotated)
                    {
                        size.transpose();
                        crop = QRect(crop.y(), crop.x(), crop.height(), crop.width());
                    }
                    out << ((static_cast<packerData*>(packer.images.at(i).id))->listItem)->text() <<
                           "\t" <<
                           pos.x() << "\t" <<
                           pos.y() << "\t" <<
                           crop.width() << "\t" <<
                           crop.height() << "\t" <<
                           crop.x() << "\t" <<
                           crop.y() << "\t" <<
                           sizeOrig.width() << "\t" <<
                           sizeOrig.height() << "\t" <<
                           (packer.images.at(i).rotated ? "r" : "") << "\n";
                }
            }
        }
    }
    for (i = 0; i < packer.images.size(); i++)
    {
        if(packer.images.at(i).pos == QPoint(999999, 999999))
        {
            ((static_cast<packerData*>(packer.images.at(i).id))->listItem)->setForeground(Qt::red);
            continue;
        }
        ((static_cast<packerData*>(packer.images.at(i).id))->listItem)->setForeground(Qt::black);
        if(packer.images.at(i).duplicateId != NULL && packer.merge)
        {
            continue;
        }
        QPoint pos(packer.images.at(i).pos.x() + packer.border.l,
                   packer.images.at(i).pos.y() + packer.border.t);
        QSize size;
        QRect crop;
        if(!packer.cropThreshold)
        {
            size = packer.images.at(i).size;
            crop = QRect(0,0,size.width(),size.height());
        }
        else
        {
            size = packer.images.at(i).crop.size();
            crop = packer.images.at(i).crop;
        }
        QImage img;
        if((exporting || previewWithImages))
            img = QImage((static_cast<packerData*>(packer.images.at(i).id))->path);
        if(packer.images.at(i).rotated)
        {
            QTransform myTransform;
            myTransform.rotate(90);
            img = img.transformed(myTransform);
            size.transpose();
            crop = QRect(packer.images.at(i).size.height() - crop.y() - crop.height(), crop.x(), crop.height(), crop.width());
        }
        if(packer.images.at(i).textureId < packer.bins.size())
        {
            QPainter p(&textures.operator [](packer.images.at(i).textureId));
            if(!exporting)
                p.fillRect(pos.x(), pos.y(), size.width() + 2*packer.extrude, size.height() + 2*packer.extrude, pattern);
            if(previewWithImages || exporting)
            {
                if(packer.extrude)
                {
                    QColor color1=QColor::fromRgba(img.pixel(crop.x(), crop.y()));
                    p.setPen(color1);
                    p.setBrush(color1);
                    if(packer.extrude==1)
                        p.drawPoint(QPoint(pos.x(), pos.y()));
                    else
                        p.drawRect(QRect(pos.x(), pos.y(),packer.extrude-1,packer.extrude-1));
                    
                    QColor color2=QColor::fromRgba(img.pixel(crop.x(), crop.y() + crop.height()-1));
                    p.setPen(color2);
                    p.setBrush(color2);
                    if(packer.extrude==1)
                        p.drawPoint(QPoint(pos.x(), pos.y() + crop.height() + packer.extrude));
                    else
                        p.drawRect(QRect(pos.x(), pos.y() + crop.height() + packer.extrude, packer.extrude-1, packer.extrude-1));
                    
                    QColor color3=QColor::fromRgba(img.pixel(crop.x() + crop.width()-1, crop.y()));
                    p.setPen(color3);
                    p.setBrush(color3);
                    if(packer.extrude==1)
                        p.drawPoint(QPoint(pos.x() + crop.width() + packer.extrude, pos.y()));
                    else
                        p.drawRect(QRect(pos.x() + crop.width() + packer.extrude, pos.y(), packer.extrude-1, packer.extrude-1));
                    
                    QColor color4=QColor::fromRgba(img.pixel(crop.x() + crop.width()-1, crop.y() + crop.height()-1));
                    p.setPen(color4);
                    p.setBrush(color4);
                    if(packer.extrude==1)
                        p.drawPoint(QPoint(pos.x() + crop.width() + packer.extrude, pos.y() + crop.height() + packer.extrude));
                    else
                        p.drawRect(QRect(pos.x() + crop.width() + packer.extrude, pos.y() + crop.height() + packer.extrude, packer.extrude-1, packer.extrude-1));
                
                    p.drawImage(QRect(pos.x(), pos.y() + packer.extrude, packer.extrude, crop.height()), img, QRect(crop.x(), crop.y(), 1, crop.height()));
                    p.drawImage(QRect(pos.x() + crop.width() + packer.extrude, pos.y() + packer.extrude, packer.extrude, crop.height()), img, QRect(crop.x() + crop.width() - 1, crop.y(), 1, crop.height()));
                
                    p.drawImage(QRect(pos.x() + packer.extrude, pos.y(), crop.width(), packer.extrude), img, QRect(crop.x(), crop.y(), crop.width(), 1));
                    p.drawImage(QRect(pos.x() + packer.extrude, pos.y() + crop.height() + packer.extrude, crop.width(), packer.extrude), img, QRect(crop.x(), crop.y() + crop.height() - 1, crop.width(), 1));

                    p.drawImage(pos.x() + packer.extrude, pos.y() + packer.extrude, img, crop.x(), crop.y(), crop.width(), crop.height());
                }
                else
                    p.drawImage(pos.x(), pos.y(), img, crop.x(), crop.y(), crop.width(), crop.height());
            }
            else if(!exporting)
                p.drawRect(pos.x(), pos.y(), size.width() - 1, size.height() - 1);
        }
    }
    for(int i = 0; i < textures.count(); i++)
        area += textures.at(i).width() * textures.at(i).height();
    float percent = (((float)packer.area / (float)area) * 100.0f);
    float percent2 = (float)(((float)packer.neededArea / (float)area) * 100.0f );
    ui->preview->setText(tr("Preview: ") +
                         QString::number(percent) + QString("% filled, ") +
                         (packer.missingImages == 0 ? QString::number(packer.missingImages) + tr(" images missed,") :
                                                      QString("<font color=red><b>") + QString::number(packer.missingImages) + tr(" images missed,") + "</b></font>") +
                         " " + QString::number(packer.mergedImages) + tr(" images merged, needed area: ") +
                         QString::number(percent2) + "%." + tr(" KBytes: ") + QString::number(area*4/1024));
    if(exporting)
    {
        const char * format = qPrintable(outFormat);
        for(int i = 0; i < textures.count(); i++)
        {
            QString imgdirFile;
            imgdirFile = outDir;
            imgdirFile += QDir::separator();
            imgdirFile += outFile;
            if(textures.count() > 1)
                imgdirFile += QString("_") + QString::number(i + 1);
            imgdirFile += ".";
            imgdirFile += outFormat.toLower();
            if(outFormat == "JPG")
            {
                textures.at(i).save(imgdirFile, format, 100);
            }
            else
            {
                textures.at(i).save(imgdirFile);
            }
        }
        QMessageBox::information(0, tr("Done"), tr("Your atlas successfully saved in ") + outDir);
        exporting = false;
    }
    else
        emit renderedImage(textures);
}

void MainWindow::setTextureSize2048()
{
    ui->textureW->setValue(2048);
    ui->textureH->setValue(2048);
}

void MainWindow::setTextureSize256()
{
    ui->textureW->setValue(256);
    ui->textureH->setValue(256);
}

void MainWindow::setTextureSize512()
{
    ui->textureW->setValue(512);
    ui->textureH->setValue(512);
}

void MainWindow::setTextureSize1024()
{
    ui->textureW->setValue(1024);
    ui->textureH->setValue(1024);
}
void MainWindow::updateAplhaThreshold()
{
    packer.cropThreshold = ui->alphaThreshold->value();
    packer.UpdateCrop();
    updateAuto();
}

void MainWindow::getFolder()
{
    ui->outDir->setText(QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                          ui->outDir->text(),
                                                          QFileDialog::ShowDirsOnly));
}

void MainWindow::exportImage()
{
    exporting = true;
    packerUpdate();
}

void MainWindow::swapSizes()
{
    int buf = ui->textureW->value();
    ui->textureW->setValue(ui->textureH->value());
    ui->textureH->setValue(buf);
}

void MainWindow::clearTiles()
{
    packer.images.clear();
    ui->tilesList->clear();
}

void MainWindow::updateAuto()
{
    if(ui->autoUpdate->isChecked())
        packerUpdate();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> droppedUrls = event->mimeData()->urls();
    int droppedUrlCnt = droppedUrls.size();
    for(int i = 0; i < droppedUrlCnt; i++)
    {
        QString localPath = droppedUrls[i].toLocalFile();
        QFileInfo fileInfo(localPath);
        if(fileInfo.isFile()) {
            ui->tilesList->addItem(fileInfo.fileName());
            packerData *data = new packerData;
            data->listItem = ui->tilesList->item(ui->tilesList->count() - 1);
            data->path = fileInfo.absoluteFilePath();
            packer.addItem(data->path, data);
            //QMessageBox::information(this, tr("Dropped file"), "Dropping files is not supported yet. Drad and drop directory here.");
        }
        else if(fileInfo.isDir()) {
            addDir(fileInfo.absoluteFilePath());
        }
    }
    packerUpdate();

    event->acceptProposedAction();
}
