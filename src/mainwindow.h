#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QUrl>
#include <QDrag>
#include <QListWidget>
#include "imagepacker.h"

extern QStringList imageExtensions;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    void RecurseDirectory(const QString &dir);
    QString topImageDir;
    ImagePacker packer;
    QList<packedImage> packedImageList;
    bool exporting;
    int recursiveLoaderCounter;
    bool recursiveLoaderDone;
    QPixmap pattern;
    void addDir(QString dir);
    struct packerData {
        QListWidgetItem * listItem;
        QString path;
    };

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
signals:
    void renderedImage(const QList<QImage> &image);
public slots:
    void addTiles();
    void deleteSelectedTiles();
    void packerUpdate();
    void updateAuto();
    void setTextureSize2048();
    void setTextureSize256();
    void setTextureSize512();
    void setTextureSize1024();
    void getFolder();
    void exportImage();
    void swapSizes();
    void clearTiles();
};

#endif // MAINWINDOW_H
