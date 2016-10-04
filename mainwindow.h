#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QImage>

#include <stdint.h>

#include "graphicsviewex.h"
#include "graphicssceneex.h"
#include "extcolordefs.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QGraphicsPixmapItem *pixmapItem;
    QImage *currentImage;
    uint32_t *currentImageData;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static uint32_t *qImageToBitmapData(QImage *image);

public:
    void addImagesBtnClicked();
    void removeSelectedBtnClicked();
    void generateImageBtnClicked();
    void saveAsBtnClicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
