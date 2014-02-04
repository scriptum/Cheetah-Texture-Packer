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
        fputs(error, stderr);
    printf(
"Usage: packer [-s size] [-o OUTFILE] [options] [file|directory ...]\n"
"Avaiable options:\n"
"  -s, --size W[xH]           atlas maximum size (if it is not enough - create\n"
"                             more than 1 atlas)\n"
"  -o, --out-file OUTFILE     output atlas name\n"
"      --disable-merge        do not merge similar images\n"
"      --disable-crop         do not crop images\n"
"      --crop-threshold N     crop threshold (0-255)\n"
"      --disable-border       do not make 1px border\n"
"      --border-size          set border size in pixels\n"
"      --extrude-size         set extrude size in pixels\n"
"      --enable-rotate        enable sprites rotation\n"
"      --disable-recursion    disable recursive scan (pack only given directory)\n"
"      --square               force to make square textures\n"
"      --autosize-threshold N auto-optimize atlas size (0-100, 0 - disabled)\n"
"      --min-texture-size WxH auto-optimize minimum size\n"
"      --sort-order N         select sorting order algorithm (0-4)\n"
"  -h, -?, --help             show this help and exit\n");
    if(error)
        exit(1);
    else
        exit(0);
}
struct packerData
{
    QString path, file;
};

ImagePacker *mainPacker;
QString topImageDir;

void RecurseDirectory(const QString dir, bool recursion)
{
    QDir dirEnt(dir);
    QFileInfoList list = dirEnt.entryInfoList();
    for (int i = 0; i < list.count();i++)
    {
        QFileInfo info = list[i];

        QString filePath = info.filePath();
        QString fileExt = info.suffix().toLower();
        QString name = dir + QDir::separator();
        if (recursion && info.isDir())
        {
            // recursive
            if (info.fileName() != ".." && info.fileName() != ".")
                RecurseDirectory(filePath, recursion);
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

#define check_opt(opt) (strncmp(argv[i], opt, sizeof(opt) - 1) == 0)

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
        int border = 1;
        int extrude = 0;
        bool rotate = false;
        bool recursion = true;
        bool square = false;
        bool autosize = false;
        int cropThreshold = 1;
        int autosizeThreshold = 80;
        int minTextureSizeX = 32;
        int minTextureSizeY = 32;
        int sortorder = 4;
        ImagePacker packer;
        mainPacker = &packer;
        QString outDir = QDir::currentPath();
        QString outFile = "atlas";
        for (int i = 1; i < argc; ++i)
        {
            if(check_opt("--help") || check_opt("-h") || check_opt("-?"))
                printHelp();
            else if(check_opt("-s") || check_opt("--size"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option -s");
                if(sscanf(argv[i], "%10dx%10d", &textureWidth, &textureHeight) != 2)
                {
                    if(sscanf(argv[i], "%10d", &textureWidth) != 1)
                        printHelp("Wrong texture size format");
                    else
                        textureHeight = textureWidth;
                }
                printf("Setting texture size: %dx%d\n", textureWidth, textureHeight);
            }
            else if(check_opt("-o") || check_opt("--out-file"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option -o");
                QFileInfo info(argv[i]);
                outFile = info.baseName();
                outDir = info.absolutePath();
            }
            else if(check_opt("--disable-merge"))
            {
                merge = false;
            }
            else if(check_opt("--disable-crop"))
            {
                crop = false;
            }
            else if(check_opt("--disable-recursion"))
            {
                recursion = false;
            }
            else if(check_opt("--square"))
            {
                square = true;
            }
            else if(check_opt("--autosize-threshold"))
            {
                autosize = true;
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option --autosize-threshold");
                if ((sscanf(argv[i], "%10d", &autosizeThreshold) != 1) ||
                    (autosizeThreshold < 0) ||
                    (autosizeThreshold > 100))
                {
                    printHelp("Wrong autosize threshold");
                }
            }
            else if(check_opt("--extrude-size"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option --extrude-size");
                if ((sscanf(argv[i], "%10d", &extrude) != 1) || (extrude < 0) )
                {
                    printHelp("Wrong extrude size");
                }
            }
            else if(check_opt("--border-size"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option --border-size");
                if ((sscanf(argv[i], "%10d", &border) != 1) || (border < 0) )
                {
                    printHelp("Wrong border size");
                }
            }
            else if(check_opt("--min-texture-size"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option -min-texture-size");
                if(sscanf(argv[i], "%10dx%10d", &minTextureSizeX, &minTextureSizeY) != 2)
                {
                    if(sscanf(argv[i], "%10d", &minTextureSizeX) != 1)
                        printHelp("Wrong texture size format");
                    else
                        minTextureSizeY = minTextureSizeX;
                }
            }
            else if(check_opt("--crop-threshold"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option --crop-threshold");
                if ((sscanf(argv[i], "%10d", &cropThreshold) != 1) ||
                    (cropThreshold < 0) ||
                    (cropThreshold > 255))
                {
                    printHelp("Wrong crop threshold");
                }
            }
            else if(check_opt("--sort-order"))
            {
                ++i;
                if(i >= argc)
                    printHelp("Argument needed for option --sort-order");
                if ((sscanf(argv[i], "%10d", &sortorder) != 1) ||
                    (sortorder < 0) ||
                    (sortorder > 4))
                {
                    printHelp("Wrong sortorder must be from 0 to 4");
                }
            }
            else if(check_opt("--disable-border"))
            {
                border = 0;
            }
            else if(check_opt("--enable-rotate"))
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
                    RecurseDirectory(file.absoluteFilePath(),recursion);
                }
            }
        }

        qDebug() << "Saving to dir" << outDir << "and file" << outFile;
        packer.sortOrder = sortorder;
        packer.border.t = 0;
        packer.border.l = 0;
        packer.border.r = border;
        packer.border.b = border;
        packer.extrude = extrude;
        packer.cropThreshold = crop?cropThreshold:0;
        packer.minFillRate = autosize?autosizeThreshold:0;
        packer.minTextureSizeX = minTextureSizeX;
        packer.minTextureSizeY = minTextureSizeY;
        packer.merge = merge;
        packer.mergeBF = false;
        packer.rotate = rotate;
        packer.square = square;
        packer.autosize = autosize;
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
        }
        qint64 area = 0;
        for(int i = 0; i < textures.count(); i++)
            area += textures.at(i).width() * textures.at(i).height();
        float percent = (((float)packer.area / (float)area) * 100.0f);
//        float percent2 = (float)(((float)packer.neededArea / (float)area) * 100.0f );
        printf("Atlas generated. %f%% filled, %d images missed, %d merged, %d KB\n",
               percent, packer.missingImages, packer.mergedImages, (int)((area*4)/1024));

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
