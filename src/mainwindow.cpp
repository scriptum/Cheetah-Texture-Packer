#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    exporting = false;
    imageExtensions << "bmp" << "png" << "jpg" << "jpeg" /*<< "dds"*/;
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
//    QString dir(QString::fromUtf8("/home/rpg/Игры/violetland-v0.3.0/images/anima/player"));
//    ui->tilesList->clear();
//    packedImageList.clear();
//    topImageDir = dir + QString("/");
//    ui->outDir->setText(dir);
//    recursiveLoaderCounter = 0;
//    recursiveLoaderDone = false;
//    packer.clear();
//    RecurseDirectory(dir);
//    ui->retranslateUi(this);
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
        //~ qDebug() << filePath << fileExt;
        QString name = dir + QDir::separator();
        if (info.isDir())
        {
            // recursive
            if (info.fileName()!=".." && info.fileName()!=".")
                RecurseDirectory(filePath);
        }
        else if(imageExtensions.contains(fileExt))
        {
//            QImage img(filePath);
//            if(img)
            if(!QFile::exists(name+info.completeBaseName()+QString(".atlas")))
            {
                ui->tilesList->addItem(filePath.replace(topImageDir, ""));
                packer.addItem(name+info.fileName(), ui->tilesList->item(ui->tilesList->count() - 1));
//                packedImage pi;
//                pi.img = img;
//                pi.crop = QRect(0, 0, img.width(), img.height());
//                packedImageList << pi;
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

void MainWindow::addTiles()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select tile directory"), topImageDir);
    if(dir.length() > 0)
    {
        ui->tilesList->clear();
        packedImageList.clear();
        //FIXME
        //this is messy hack due to difference between QFileDialog and QFileInfo dir separator in Windows
        if (QDir::separator() == '\\')
            topImageDir = dir.replace("\\","/") + "/";
        else
            topImageDir = dir + "/";
        ui->outDir->setText(dir);
        recursiveLoaderCounter = 0;
        recursiveLoaderDone = false;
        packer.clear();
        RecurseDirectory(dir);
        QFileInfo info(dir);
        ui->outFile->setText(info.baseName());
        packerUpdate();
    }
}

void MainWindow::deleteSelectedTiles()
{
    QList<QListWidgetItem *> itemList = ui->tilesList->selectedItems();
    for (int i=0; i<itemList.size(); i++) {
        packer.removeId(itemList[i]);
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
    packer.trim = ui->trim->currentIndex();
    packer.merge = ui->merge->isChecked();
    packer.mergeBF = false;
    packer.rotate = ui->rotationStrategy->currentIndex();
//    packedImageList.clear();
//    for (i = 0; i < ui->tilesList->count(); i++)
//    {
//        QString filename = topImageDir + ui->tilesList->item(i)->text();
//        QImage img(filename);
//        packedImage pi;
//        pi.img = img;
//        pi.crop = QRect(0, 0, img.width(), img.height());
//        //~ pi.border = true;
//        pi.id = i;
//        pi.rotate = false;
//        packedImageList << pi;
//    }
//    QList<QPoint> points;
    uint width = ui->textureW->value(), height = ui->textureH->value();
    packer.pack(0, ui->comboHeuristic->currentIndex(), width, height);
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
            QString outputFile = ui->outDir->text();
            outputFile += QDir::separator();
            outputFile += ui->outFile->text();
            if(textures.count() > 1)
                outputFile += QString("_") + QString::number(j + 1);
            outputFile += ".atlas";
            QString imgFile = ui->outFile->text();
            if(textures.count() > 1)
                imgFile += QString("_") + QString::number(j + 1);
            imgFile += ".";
            imgFile += ui->outFormat->currentText().toLower();

            QFile positionsFile(outputFile);
            if (!positionsFile.open(QIODevice::WriteOnly | QIODevice::Text))
                QMessageBox::critical(0, tr("Error"), tr("Cannot create file ") + outputFile);
            else
            {
                QTextStream out(&positionsFile);
                out << tr("textures: ") << imgFile << "\n";
                for (i = 0; i < packer.images.size(); i++)
                {
                    if(packer.images.at(i).textureId != j) continue;
                    QPoint pos(packer.images.at(i).pos.x() + packer.border.l,
                             packer.images.at(i).pos.y() + packer.border.t);
                    QSize size, sizeOrig;
                    QRect crop;
                    sizeOrig = packer.images.at(i).size;
                    if(!packer.trim)
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
//                        sizeOrig.transpose();
                        crop = QRect(packer.images.at(i).size.height() - crop.y() - crop.height(), crop.x(), crop.height(), crop.width());
                    }
                    out << ((QListWidgetItem *)packer.images.at(i).id)->text() <<
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
//    else
//    {
        for (i = 0; i < packer.images.size(); i++)
        {
            if(packer.images.at(i).pos == QPoint(999999, 999999))
            {
                ((QListWidgetItem *)packer.images.at(i).id)->setForeground(Qt::red);
                continue;
            }
            ((QListWidgetItem *)packer.images.at(i).id)->setForeground(Qt::black);
            if(packer.images.at(i).duplicateId != NULL && packer.merge)
            {
                continue;
            }
            QPoint pos(packer.images.at(i).pos.x() + packer.border.l,
                     packer.images.at(i).pos.y() + packer.border.t);
            QSize size;
            QRect crop;
            if(!packer.trim)
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
            if((exporting || ui->previewWithImages->isChecked()))
                img = QImage(topImageDir+QDir::separator()+((QListWidgetItem *)packer.images.at(i).id)->text());
            if(packer.images.at(i).rotated)
            {
                QTransform myTransform;
                myTransform.rotate(90);
                img = img.transformed(myTransform);
                size.transpose();
                crop = QRect(packer.images.at(i).size.height() - crop.y() - crop.height(), crop.x(), crop.height(), crop.width());
            }
            QPainter p(&textures.operator [](packer.images.at(i).textureId));
            if(!exporting)
                p.fillRect(pos.x(), pos.y(), size.width(), size.height(), pattern);
            if(ui->previewWithImages->isChecked() || exporting)
            {
                p.drawImage(pos.x(), pos.y(), img, crop.x(), crop.y(), crop.width(), crop.height());
            }
            else if(!exporting)
                p.drawRect(pos.x(), pos.y(), size.width() - 1, size.height() - 1);
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
             QString::number(percent2) + "%.");
        if(exporting)
        {
            const char * format = qPrintable(ui->outFormat->currentText());
            for(int i = 0; i < textures.count(); i++)
            {
                QString imgdirFile;
                imgdirFile = ui->outDir->text();
                imgdirFile += QDir::separator();
                imgdirFile += ui->outFile->text();
                if(textures.count() > 1)
                    imgdirFile += QString("_") + QString::number(i + 1);
                imgdirFile += ".";
                imgdirFile += ui->outFormat->currentText().toLower();
                //~ qDebug() << imgdirFile;
                if(ui->outFormat->currentText() == "JPG")
                {
                    int res = textures.at(i).save(imgdirFile, format, 100);
                    //~ qDebug("%d", res);
                    //~ qDebug("%d", textures.at(i).width());
                    //~ qDebug("%x", &textures.at(i));
                }
                else
                {
                    int res = textures.at(i).save(imgdirFile);
                    //~ qDebug("%d", res);
                    //~ qDebug("%d", textures.at(i).width());
                    //~ qDebug("%x", &textures.at(i));
                }
            }

            QMessageBox::information(0, tr("Done"), tr("Your atlas successfully saved in ") + ui->outDir->text());
            exporting = false;
        }
        else
            emit renderedImage(textures);
//    }
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

    QMessageBox::information(this, tr("Dropped file"), "123");
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> droppedUrls = event->mimeData()->urls();
    int droppedUrlCnt = droppedUrls.size();
    QMessageBox::information(this, tr("Dropped file"), "123");
    for(int i = 0; i < droppedUrlCnt; i++) {
        QString localPath = droppedUrls[i].toLocalFile();
        QFileInfo fileInfo(localPath);
        if(fileInfo.isFile()) {
// file
            QMessageBox::information(this, tr("Dropped file"), fileInfo.absoluteFilePath());
        }
        else if(fileInfo.isDir()) {
// directory
            QMessageBox::information(this, tr("Dropped directory"), fileInfo.absoluteFilePath());
        }
        else {
// none
            QMessageBox::information(this, tr("Dropped, but unknown"), tr("Unknown: %1").arg(fileInfo.absoluteFilePath()));
        }
    }

    event->acceptProposedAction();
}
