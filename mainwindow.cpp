#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    currentImage=0;
    currentImageData=0;

    pixmapItem=new QGraphicsPixmapItem();
    ui->graphicsView->scene()->addItem(pixmapItem);

    connect(ui->addImagesBtn,&QPushButton::clicked,this,&MainWindow::addImagesBtnClicked);
    connect(ui->removeSelectedBtn,&QPushButton::clicked,this,&MainWindow::removeSelectedBtnClicked);
    connect(ui->generateImageBtn,&QPushButton::clicked,this,&MainWindow::generateImageBtnClicked);
    connect(ui->saveAsBtn,&QPushButton::clicked,this,&MainWindow::saveAsBtnClicked);
}

uint32_t *MainWindow::qImageToBitmapData(QImage *image)
{
    int32_t width=image->width();
    int32_t height=image->height();
    uint32_t *out=(uint32_t*)malloc(width*height*sizeof(uint32_t));
    for(int32_t y=0;y<height;y++)
    {
        int32_t offset=y*width;
        QRgb *scanLine=(QRgb*)image->scanLine(y); // Do not free!
        for(int32_t x=0;x<width;x++)
        {
            QRgb color=scanLine[x];
            uint32_t alpha=qAlpha(color);
            uint32_t red=qRed(color);
            uint32_t green=qGreen(color);
            uint32_t blue=qBlue(color);
            out[offset+x]=(alpha<<24)|(red<<16)|(green<<8)|blue;
        }
        // Do not free "scanLine"!
    }
    return out;
}

void MainWindow::addImagesBtnClicked()
{
    QStringList imgs=QFileDialog::getOpenFileNames(this,"Select images to add",QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),tr("All images (*.jpg *.jpeg *.png *.gif *.bmp);;JPEG images (*.jpg *.jpeg);;PNG images (*.png);;GIF images (*.gif);;Bitmaps (*.bmp)") );
    int c=ui->imageList->count();
    for(QString img:imgs)
    {
        for(int i=0;i<c;i++)
        {
            if(ui->imageList->item(i)->text().compare(img,Qt::CaseInsensitive)==0)
                goto DoNotAdd;
        }
        ui->imageList->addItem(img);
        DoNotAdd:
        continue;
    }
}

void MainWindow::removeSelectedBtnClicked()
{
    QList<QListWidgetItem*> s=ui->imageList->selectedItems();
    for(QListWidgetItem *i:s)
        ui->imageList->takeItem(ui->imageList->row(i));
}

void MainWindow::generateImageBtnClicked()
{
    int c=ui->imageList->count();
    if(c==0)
    {
        QMessageBox::critical(this,"Error","No images added.");
        return;
    }
    if(c==1)
    {
        QMessageBox::critical(this,"Error","Please add more than one image.");
        return;
    }
    // Load images
    QList<QImage> images;
    bool error=false;
    for(int i=0;i<c;i++)
    {
        QListWidgetItem *item=ui->imageList->item(i);
        QImage img=QImage(item->text());
        if(img.isNull())
        {
            if(!error)
                error=true;
            continue;
        }
        images.append(img);
    }
    if(error)
    {
        if(images.count()<2)
        {
            QMessageBox::critical(this,"Error","Images could not be loaded.");
            return;
        }
        else
            QMessageBox::critical(this,"Error","Some images could not be loaded.");
    }
    // Check if widths/heights uniform
    int firstWidth=images.at(0).width();
    int firstHeight=images.at(0).height();
    int nC=images.count();
    for(int i=1;i<nC;i++)
    {
        QImage img=images.at(i);
        if(img.width()!=firstWidth||img.height()!=firstHeight)
        {
            QMessageBox::critical(this,"Error","All images must be of the same width and height.");
            return;
        }
    }
    // Success
    delete currentImage;
    free(currentImageData);
    // Load pixel data of images
    QList<uint32_t*> pixelDataList;
    for(int i=0;i<nC;i++)
    {
        QImage img=images.at(i);
        pixelDataList.append(qImageToBitmapData(&img));
    }
    // Generate image
    currentImageData=(uint32_t*)malloc(firstWidth*firstHeight*sizeof(uint32_t));

    /*
    bool useMedianMethod=ui->medianRadioBtn->isChecked();
    if(useMedianMethod)
    {
    */
    int *pixelSums=(int*)calloc(firstWidth*firstHeight*4*sizeof(int),1);
    for(int i=0;i<nC;i++)
    {
        uint32_t *pd=pixelDataList.at(i);
        for(int y=0;y<firstHeight;y++)
        {
            int offset=y*firstWidth;
            for(int x=0;x<firstWidth;x++)
            {
                int pos=(offset+x)*4;
                uint32_t color=pd[offset+x];
                pixelSums[pos]+=getAlpha(color);
                pixelSums[pos+1]+=getRed(color);
                pixelSums[pos+2]+=getGreen(color);
                pixelSums[pos+3]+=getBlue(color);
            }
        }
    }
    for(int y=0;y<firstHeight;y++)
    {
        int offset=y*firstWidth;
        for(int x=0;x<firstWidth;x++)
        {
            int pos=(offset+x)*4;
            int alpha=(int)round(floatDiv(pixelSums[pos],nC));
            int red=(int)round(floatDiv(pixelSums[pos+1],nC));
            int green=(int)round(floatDiv(pixelSums[pos+2],nC));
            int blue=(int)round(floatDiv(pixelSums[pos+3],nC));
            currentImageData[offset+x]=getColor(alpha,red,green,blue);
        }
    }
    free(pixelSums);
    /*
    }
    else
    {
        // Use color distance-adjustment method
        float *totalDistanceSums=(float*)calloc(firstWidth*firstHeight,sizeof(float));
        float *individualDistanceSums=(float*)calloc(firstWidth*firstHeight*nC,sizeof(float));
        // Calculate distance sums
        for(int i=0;i<nC;i++)
        {
            uint32_t *pd=pixelDataList.at(i);
            for(int y=0;y<firstHeight;y++)
            {
                int offset=y*firstWidth;
                for(int x=0;x<firstWidth;x++)
                {
                    uint32_t color=pd[offset+x];
                    float distanceSum=0.0;
                    for(int j=0;j<nC;j++)
                    {
                        if(j==i)
                            continue;
                        uint32_t *jpd=pixelDataList.at(j);
                        uint32_t jColor=jpd[offset+x];
                        distanceSum+=getColorError(color,jColor);
                    }
                    distanceSum=1.0f/distanceSum;
                    individualDistanceSums[(offset+x)*i]=distanceSum;
                    totalDistanceSums[offset+x]+=distanceSum;
                }
            }
        }
        // Get 4-float color map (A,R,G,B for each pixel)
        float *colorMap=(float*)calloc(firstWidth*firstHeight*4,sizeof(float));
        for(int i=0;i<nC;i++)
        {
            uint32_t *pd=pixelDataList.at(i);
            for(int y=0;y<firstHeight;y++)
            {
                int offset=y*firstWidth;
                for(int x=0;x<firstWidth;x++)
                {
                    int pos=(offset+x)*4;
                    uint32_t color=pd[offset+x];
                    float totalDistanceSum=totalDistanceSums[offset+x];
                    float individualDistanceSum=individualDistanceSums[(offset+x)*i];
                    // This needs to be changed (definitely):
                    float colorWeight=individualDistanceSum/totalDistanceSum;
                    // Do not divide by 255, it is unnecessary
                    colorMap[pos]+=colorWeight*((float)getAlpha(color));
                    colorMap[pos+1]+=colorWeight*((float)getRed(color));
                    colorMap[pos+2]+=colorWeight*((float)getGreen(color));
                    colorMap[pos+3]+=colorWeight*((float)getBlue(color));
                }
            }
        }
        // Translate color map into pixel map
        for(int y=0;y<firstHeight;y++)
        {
            int offset=y*firstWidth;
            for(int x=0;x<firstWidth;x++)
            {
                int pos=(offset+x)*4;
                int alpha=(int)round(colorMap[pos]);
                int red=(int)round(colorMap[pos+1]);
                int green=(int)round(colorMap[pos+2]);
                int blue=(int)round(colorMap[pos+3]);
                currentImageData[offset+x]=getColor(alpha,red,green,blue);
            }
        }
        free(colorMap);
        free(totalDistanceSums);
        free(individualDistanceSums);
    }*/
    for(int i=0;i<nC;i++)
        free(pixelDataList.at(i));
    currentImage=new QImage((uchar*)currentImageData,firstWidth,firstHeight,QImage::Format_ARGB32);
    pixmapItem->setPixmap(QPixmap::fromImage(*currentImage));
}

void MainWindow::saveAsBtnClicked()
{
    if(currentImageData==0)
        return;
    QString path=QFileDialog::getSaveFileName(this,"Save as...",QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),"JPG image (*.jpg);;PNG image (*.png);;GIF image (*.gif);;Bitmap (*.bmp)");
    if(path=="")
        return;
    currentImage->save(path,0,100);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete currentImage;
    free(currentImageData);
}
