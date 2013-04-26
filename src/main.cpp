#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#   include <QtWidgets/QApplication>
#else
#   include <QtGui/QApplication>
#endif

#include "mainwindow.h"
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include "stdio.h"
#include "stdlib.h"

QStringList imageExtensions;

void printHelp(const char * error = NULL)
{
    if(error)
        puts(error);
    printf("Usage: packer [-s size] [-o outfile] [--disable-merge] [--disable-crop] [--disable-border] [--enable-rotate] [file|directory ...]\n");
    if(error)
        exit(1);
}
struct packerData
{
    QString path, file;
};

ImagePacker *mainPacker;
QString topImageDir;

void RecurseDirectory(const QString dir)
{
    QDir dirEnt(dir);
    QFileInfoList list = dirEnt.entryInfoList();
    for (int i = 0; i < list.count();i++)
    {
        QFileInfo info = list[i];

        QString filePath = info.filePath();
        QString fileExt = info.suffix().toLower();
        QString name = dir + QDir::separator();
        if (info.isDir())
        {
            // recursive
            if (info.fileName() != ".." && info.fileName() != ".")
                RecurseDirectory(filePath);
        }
        else if(imageExtensions.contains(fileExt))
        {
            if(!QFile::exists(name+info.completeBaseName() + QString(".atlas")))
            {
                packerData * data = new packerData;
                data->path = info.absoluteFilePath();
                data->file = filePath.replace(topImageDir, "");
//                qDebug() << "Packing " << data->path << "...";
                mainPacker->addItem(data->path, data);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    imageExtensions << "bmp" << "png" << "jpg" << "jpeg";
    //command-line version
    if(argc > 1)
    {
        int textureWidth = 512;
        int textureHeight = 512;
        bool merge = true;
        bool crop = true;
        bool border = true;
        bool rotate = false;
        ImagePacker packer;
        mainPacker = &packer;
        QString outDir = QDir::currentPath();
        QString outFile = "atlas";
        for (int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i], "--help") == 0)
                printHelp();
            else if(strcmp(argv[i], "-s") == 0)
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option -s");
                if(sscanf(argv[i], "%dx%d", &textureWidth, &textureHeight) != 2)
                {
                    if(sscanf(argv[i], "%d", &textureWidth) != 1)
                        printHelp("Wrong texture size format");
                    else
                        textureHeight = textureWidth;
                }
                printf("Setting texture size: %dx%d\n", textureWidth, textureHeight);
            }
            else if(strcmp(argv[i], "-o") == 0)
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option -o");
                QFileInfo info(argv[i]);
                outFile = info.baseName();
                outDir = info.absolutePath();
            }
            else if(strcmp(argv[i], "--disable-merge") == 0)
            {
                merge = false;
            }
            else if(strcmp(argv[i], "--disable-crop") == 0)
            {
                crop = false;
            }
            else if(strcmp(argv[i], "--disable-border") == 0)
            {
                border = false;
            }
            else if(strcmp(argv[i], "--enable-rotate") == 0)
            {
                rotate = true;
            }
            //dir or file
            else
            {
                QFileInfo file(argv[i]);
                if(file.isFile())
                {
                    packerData * data = new packerData;
                    data->path = file.absoluteFilePath();
                    data->file = file.fileName();
                    packer.addItem(data->path, data);
                }
                else if(file.isDir())
                {
                    topImageDir = file.absoluteFilePath();
                    RecurseDirectory(file.absoluteFilePath());
                }
            }
        }

        qDebug() << "Saving to dir" << outDir << "and file" << outFile;
        packer.sortOrder = 2;
        packer.border.t = 0;
        packer.border.l = 0;
        if(border)
        {
            packer.border.r = 1;
            packer.border.b = 1;
        }
        else
        {
            packer.border.r = 0;
            packer.border.b = 0;
        }
        packer.trim = crop;
        packer.merge = merge;
        packer.mergeBF = false;
        packer.rotate = rotate;
        int heuristic = 1;

        QString outFormat("PNG");

        if(packer.images.size() == 0)
        {
            fprintf(stderr, "No images found, exitting\n");
                    exit(1);
        }

        packer.pack(heuristic, textureWidth, textureHeight);
        QList<QImage> textures;
        for (int i = 0; i < packer.bins.size(); i++)
        {
            QImage texture(packer.bins.at(i).width(), packer.bins.at(i).height(), QImage::Format_ARGB32);
            texture.fill(Qt::transparent);
            textures << texture;
        }
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
                fprintf(stderr, "Cannot create file %s", qPrintable(outputFile));
            else
            {
                QTextStream out(&positionsFile);
                out << "textures: " << imgFile << "\n";
                for (int i = 0; i < packer.images.size(); i++)
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
                        crop = QRect(packer.images.at(i).size.height() - crop.y() - crop.height(),
                                     crop.x(), crop.height(), crop.width());
                    }
                    out << ((packerData*)(packer.images.at(i).id))->file <<
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

        for (int i = 0; i < packer.images.size(); i++)
        {
            qDebug() << "Processing" << ((packerData*)(packer.images.at(i).id))->file;
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
            img = QImage(((packerData*)(packer.images.at(i).id))->path);
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
                p.drawImage(pos.x(), pos.y(), img, crop.x(), crop.y(), crop.width(), crop.height());
            }
        }
        qint64 area = 0;
        for(int i = 0; i < textures.count(); i++)
            area += textures.at(i).width() * textures.at(i).height();
        float percent = (((float)packer.area / (float)area) * 100.0f);
//        float percent2 = (float)(((float)packer.neededArea / (float)area) * 100.0f );
        printf("Atlas generated. %f%% filled, %d images missed, %d merged.\n",
               percent, packer.missingImages, packer.mergedImages);

//        const char * format = qPrintable(outFormat);
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
            textures.at(i).save(imgdirFile);
        }

        return 0;
    }

    QTranslator myTranslator;
    myTranslator.load("tile_" + QLocale::system().name(), "qm");
    a.installTranslator(&myTranslator);
    MainWindow w;
    w.show();

    
    return a.exec();
}
