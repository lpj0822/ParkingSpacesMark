#pragma execution_character_set("utf-8")
#include "controlwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "apd_detection_and_track.h"

ControlWindow::ControlWindow(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
    initData();
}

ControlWindow::~ControlWindow()
{
    videoProcess->closeVideo();
}

void ControlWindow::closeEvent(QCloseEvent *event)
{
    saveResult();
}

void ControlWindow::keyPressEvent(QKeyEvent *e)
{
    if(videoProcess->isOpen())
    {
        if(e->key() == Qt::Key_A)
        {
            slotPrevious();
        }
        else if(e->key() == Qt::Key_D)
        {
            slotNext();
        }
    }
}

void ControlWindow::slotOpenVideo()
{
    QString roiPath = "./roi.txt";
    int errorCode = 0;
    QString videoPath = QFileDialog::getOpenFileName(this, tr("打开视频"), ".", "video files(*.avi *.mp4 *.mpg)");
    if(videoPath.trimmed().isEmpty())
    {
        qDebug() << "打开的视频文件路径有误:" << videoPath << endl;
        return;
    }
    videoPathText->setText(videoPath.trimmed());
    errorCode = videoProcess->openVideo(videoPath);
    if(errorCode == 0 && videoProcess->isOpen())
    {
        QFileInfo fileInfo(videoPath);
        resultSavePath = fileInfo.path() + "/" + fileInfo.completeBaseName();

        stopButton->setEnabled(true);

        canInfoPath = videoPath + "_CANlog.txt";
        QFileInfo canFile(canInfoPath);
        if(canFile.exists())
        {
            hasCanInfo = true;

            roiRect = initRoi(roiPath);
            drawImageLabel->setTopPoint(QPoint(roiRect.x(), roiRect.y()));

            allCountFrame = videoProcess->getFrameCount();
            drawImageLabel->setEnabled(false);
            configGroundBox->setEnabled(false);
            processGroundBox->setEnabled(true);
            modifyButton->setEnabled(false);
            reviewNext();
            updateButton();
        }
        else
        {
            QMessageBox::StandardButton result = QMessageBox::question(this, tr("CAN信息文件"), tr("%1CAN信息文件不存在，是否继续?").arg(canInfoPath),
                                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            hasCanInfo = false;
            if(result == QMessageBox::Yes)
            {
                roiRect = initRoi(videoPath);
                drawImageLabel->setTopPoint(QPoint(roiRect.x(), roiRect.y()));

                allCountFrame = videoProcess->getFrameCount();
                drawImageLabel->setEnabled(false);
                configGroundBox->setEnabled(false);
                processGroundBox->setEnabled(true);
                modifyButton->setEnabled(false);
                reviewNext();
                updateButton();
            }
        }
    }
    else
    {
        QMessageBox::information(this, tr("打开视频"), tr("%1视频打开失败").arg(videoPath));
    }
}

void ControlWindow::slotPrevious()
{
    reviewPrevious();
}

void ControlWindow::slotNext()
{
    cv::Mat tempFrame;
    if(currentFrameNum < allCountFrame)
    {
        if(isStartMark && hasCanInfo)
        {
            if(videoProcess->readFrame(frame) == 0)
            {
                QList< QList<int> > polygonsIndex = drawImageLabel->getPolygonsIndex();
                QList<QPointF> pointList = drawImageLabel->getPoints();
                QList<QPointF> trackingResultPoints;
                saveResult();
                trackingResultPoints = trackingWidthCan(pointList, canInfoPath, polygonsIndex);

                cv::cvtColor(frame, tempFrame, cv::COLOR_BGR2RGB);//Qt中支持的是RGB图像, OpenCV中支持的是BGR
                currentImage = QImage((uchar*)(tempFrame.data), tempFrame.cols, tempFrame.rows, QImage::Format_RGB888);
                currentFrameNum = videoProcess->getFramePosition();
                updateVideoProcess();
                updateImage();
                drawImageLabel->setDrawData(trackingResultPoints, polygonsIndex);
            }
            else
            {
                QMessageBox::information(this, tr("读取视频"), tr("读取视频第%1帧失败！").arg(currentFrameNum + 1));
            }
        }
        else
        {
            reviewNext();
        }
    }
    updateButton();
}

void ControlWindow::slotStartMark()
{
    if(isStartMark)
    {
        saveResult();
        isStartMark = false;
        drawImageLabel->setEnabled(false);
        startMarkButton->setText(tr("启用标注"));
        modifyButton->setEnabled(false);
    }
    else
    {
        isStartMark = true;
        drawImageLabel->setEnabled(true);
        startMarkButton->setText(tr("禁止标注"));
        modifyButton->setEnabled(true);
    }
    modifyButton->setText(tr("调整库位"));
    drawImageLabel->setIsModify(false);
}

void ControlWindow::slotModify()
{
    if(isStartMark)
    {
        if(drawImageLabel->getIsModify())
        {
            modifyButton->setText(tr("调整库位"));
            drawImageLabel->setIsModify(false);
        }
        else
        {
            modifyButton->setText(tr("禁止调整库位"));
            drawImageLabel->setIsModify(true);
        }
    }
}

void ControlWindow::slotStop()
{
    saveResult();
    currentFrameNum = 0;
    allCountFrame = 0;
    isStartMark = false;
    startMarkButton->setText(tr("启用标注"));
    drawImageLabel->setIsModify(false);
    modifyButton->setText(tr("调整库位"));
    currentImage = QImage(tr(":/images/images/play.png"));
    videoProcess->closeVideo();
    processGroundBox->setEnabled(false);
    configGroundBox->setEnabled(true);
    drawImageLabel->setEnabled(false);
    stopButton->setEnabled(false);
    updateImage();
    updateVideoProcess();
}

void ControlWindow::reviewPrevious()
{
    cv::Mat tempFrame;
    if(currentFrameNum > 1)
    {
        QList< QList<int> > polygonsIndex;
        QList<QPointF> pointList;
        saveResult();
        currentFrameNum--;
        readResult(pointList, polygonsIndex);
        videoProcess->setFramePosition(currentFrameNum - 1);
        if(videoProcess->readFrame(frame) == 0)
        {
            cv::cvtColor(frame, tempFrame, cv::COLOR_BGR2RGB);//Qt中支持的是RGB图像, OpenCV中支持的是BGR
            currentImage = QImage((uchar*)(tempFrame.data), tempFrame.cols, tempFrame.rows, QImage::Format_RGB888);
            updateVideoProcess();
            updateImage();
            drawImageLabel->setDrawData(pointList, polygonsIndex);
        }
        else
        {
            QMessageBox::information(this, tr("读取视频"), tr("读取视频第%1帧失败！").arg(currentFrameNum));
            currentFrameNum++;
        }
    }
    updateButton();
}

void ControlWindow::reviewNext()
{
    cv::Mat tempFrame;
    if(currentFrameNum < allCountFrame)
    {
        if(videoProcess->readFrame(frame) == 0)
        {
            QList< QList<int> > polygonsIndex;
            QList<QPointF> pointList;
            cv::cvtColor(frame, tempFrame, cv::COLOR_BGR2RGB);//Qt中支持的是RGB图像, OpenCV中支持的是BGR
            currentImage = QImage((uchar*)(tempFrame.data), tempFrame.cols, tempFrame.rows, QImage::Format_RGB888);
            currentFrameNum = videoProcess->getFramePosition();
            readResult(pointList, polygonsIndex);
            updateVideoProcess();
            updateImage();
            drawImageLabel->setDrawData(pointList, polygonsIndex);
        }
        else
        {
            QMessageBox::information(this, tr("读取视频"), tr("读取视频第%1帧失败！").arg(currentFrameNum + 1));
        }

    }
    updateButton();
}

void ControlWindow::updateButton()
{
    if(currentFrameNum >= allCountFrame)
    {
        previousButton->setEnabled(true);
        nextButton->setEnabled(false);
    }
    else if(currentFrameNum <= 1)
    {
        previousButton->setEnabled(false);
        nextButton->setEnabled(true);
    }
    else
    {
        previousButton->setEnabled(true);
        nextButton->setEnabled(true);
    }
}

void ControlWindow::updateImage()
{
    drawImageLabel->clearPoints();
    drawImageLabel->setNewQImage(currentImage.copy(roiRect));
    referenceLabel->setPixmap(QPixmap::fromImage(currentImage.copy(roiRect)));
}

void ControlWindow::updateVideoProcess()
{
    videoProcessShow->setText(QString("%1/%2(帧)").arg(currentFrameNum).arg(allCountFrame));
}

QRect ControlWindow::initRoi(const QString &roiPath)
{
    int index = 0;
    QRect tempRoi;
    QFileInfo roiFile(roiPath);
    if(roiFile.exists())
    {
        QFile file(roiPath);
        if(file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            while(!in.atEnd())
            {
                QString strContent = in.readLine().trimmed();
                if(strContent.isEmpty())
                    continue;
                QStringList pointsStr = strContent.split(" ");
                if(index == 0)
                {
                    tempRoi.setX(pointsStr[0].toInt());
                }
                else if (index == 1)
                {
                    tempRoi.setY(pointsStr[0].toInt());
                }
                else if (index == 2)
                {
                    tempRoi.setWidth(pointsStr[0].toInt());
                }
                else if (index == 3)
                {
                    tempRoi.setHeight(pointsStr[0].toInt());
                    break;
                }
                index++;
            }
            file.close();
        }

    }
    else
    {
        cv::Size size = videoProcess->getSize();
        tempRoi.setX(0);
        tempRoi.setY(0);
        tempRoi.setWidth(size.width);
        tempRoi.setHeight(size.height);
    }
    return tempRoi;
}

QList<QPointF> ControlWindow::trackingWidthCan(const QList<QPointF> &pointList, const QString canFilePath, const QList< QList<int> > &polygonsIndex)
{
    QList<QPointF> trackingResult;
    const QByteArray canPath = canFilePath.toLocal8Bit();
    const char *canInfoFile = canPath.data();
    trackingResult.clear();
    if(pointList.size() > 0 && polygonsIndex.size() > 0)
    {
        int pointCount = pointList.size();
        int indexCount = polygonsIndex.size() * polygonsIndex[0].size();
        Point2f* tempPoints = new Point2f[pointCount];
        Point2f* resultPoints = new Point2f[pointCount];
        int* polygonIndex = new int[indexCount];
        for(int index = 0; index < pointCount; index++)
        {
            tempPoints[index].x = pointList[index].x();
            tempPoints[index].y = pointList[index].y();
        }
        int index = 0;
        for(int index1 = 0; index1 < polygonsIndex.size(); index1++)
        {
            for(int index2 = 0; index2 < polygonsIndex[0].size(); index2++)
            {
                polygonIndex[index] = polygonsIndex[index1][index2];
                index++;
            }
        }
        slot_pt_tracking(frame, QString("./ParkingAppConfigFile.txt").toStdString().c_str(),
                         tempPoints, pointCount, polygonIndex, indexCount, canInfoFile, currentFrameNum, resultPoints);
        //slot_tracking(tempPoints, count, canInfoFile, currentFrameNum, resultPoints);
        for(int index = 0; index < pointCount; index++)
        {
            QPointF point;
            point.setX(resultPoints[index].x);
            point.setY(resultPoints[index].y);
            trackingResult.append(point);
        }
        if(tempPoints != NULL)
        {
            delete tempPoints;
            tempPoints = NULL;
        }
        if(resultPoints != NULL)
        {
            delete resultPoints;
            resultPoints = NULL;
        }
        if(polygonIndex != NULL)
        {
            delete polygonIndex;
            polygonIndex = NULL;
        }
    }
    //qDebug() << "result point:" << trackingResult << endl;
    return trackingResult;
}

void ControlWindow::saveResult()
{
    if(isStartMark)
    {
        if(videoProcess->isOpen())
        {
            QDir saveDir(resultSavePath);
            if(!saveDir.exists())
            {
                if(!saveDir.mkdir(resultSavePath))
                {
                    qDebug() << "make dir fail:" << resultSavePath << endl;
                    return;
                }
            }
            QList<QPolygonF> polygonList = drawImageLabel->getPolygons();
            int polygonCount = polygonList.size();

            //if(polygonCount > 0)
            //{
                QString resultFileName = resultSavePath + QString("/%1.txt").arg(currentFrameNum);
                QFile file(resultFileName);
                if(file.open(QIODevice::WriteOnly))
                {
                    QTextStream out(&file);
                    for(int loop = 0; loop < polygonCount; loop++)
                    {
                        QPolygonF &polygon = polygonList[loop];
                        for(int loop1 = 0; loop1 < polygon.size(); loop1++)
                        {
                            QString pointText;
                            pointText.sprintf("%.2f %.2f|", polygon[loop1].x(), polygon[loop1].y());
                            out << pointText;
                        }
                        out << endl;
                    }
                    file.close();
                }
                else
                {
                    QMessageBox::information(this, tr("保存结果"), tr("保存结果文件%1失败!").arg(resultFileName));
                }
            //}
        }
    }
}

void ControlWindow::readResult(QList<QPointF> &pointList, QList< QList<int> > &polygonsIndex)
{
    QString resultFileName = resultSavePath + QString("/%1.txt").arg(currentFrameNum);
    QFile file(resultFileName);
    QList<QPolygonF> tempPolygons;
    pointList.clear();
    polygonsIndex.clear();
    tempPolygons.clear();
    if(file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString strContent = in.readLine().trimmed();
            if(strContent.isEmpty())
                continue;
            QStringList pointsStr = strContent.split("|");
            QPolygonF polygon;
            for(int loop = 0; loop < pointsStr.size(); loop++)
            {
                QString point = pointsStr.at(loop).trimmed();
                if(!point.isEmpty())
                {
                    QStringList tempPoint = point.split(" ");
                    polygon.append(QPointF(tempPoint[0].toFloat(), tempPoint[1].toFloat()));
                }
            }
            if(!polygon.isEmpty())
            {
                tempPolygons.append(polygon);
            }
        }
        file.close();
    }
    else
    {    
        //QMessageBox::information(this, tr("打开库位信息文件"), tr("第%1帧未标注库位信息！").arg(currentFrameNum));
        return;
    }

    for(int loop = 0; loop < tempPolygons.size(); loop++)
    {
        QPolygonF &polygon = tempPolygons[loop];
        for(int loop1 = 0; loop1 < polygon.size(); loop1++)
        {
            if(!pointList.contains(polygon[loop1]))
            {
                pointList.append(polygon[loop1]);
            }
        }
    }

    for(int loop = 0; loop < tempPolygons.size(); loop++)
    {
        QPolygonF &polygon = tempPolygons[loop];
        QList<int> tempIndex;
        for(int loop1 = 0; loop1 < polygon.size(); loop1++)
        {
            tempIndex.append(pointList.indexOf(tempPolygons[loop][loop1]));
        }
        polygonsIndex.append(tempIndex);
    }
}

void ControlWindow::initUI()
{
    infoLabel = new QLabel(tr("泊车库位标注软件使用:\n按a键显示上一帧\n按d键显示上一帧\n单击鼠标左键为画点"
                              "\n单击鼠标右键库可以删除库位\n\n注意：\n一个泊车库位为4个点,\n如果某一点画错，\n需要补全库位的其他点，"
                              "\n然后删除库位。"));
    videoProcessLabel = new QLabel(tr("视频播放进度："));
    videoProcessShow = new QLabel(tr("0/0"));
    QHBoxLayout *hLayout0 = new QHBoxLayout();
    hLayout0->setSpacing(20);
    hLayout0->addWidget(videoProcessLabel);
    hLayout0->addWidget(videoProcessShow);
    stopButton = new QPushButton(tr("停止播放"));
    stopButton->setEnabled(false);
    QVBoxLayout *rightLayout0 = new QVBoxLayout();
    rightLayout0->setSpacing(20);
    rightLayout0->addWidget(infoLabel);
    rightLayout0->addLayout(hLayout0);
    rightLayout0->addWidget(stopButton);
    videoInfoGroundBox = new QGroupBox(tr("视频播放进度"));
    videoInfoGroundBox->setLayout(rightLayout0);

    previousButton = new QPushButton(tr("上一帧"));
    nextButton = new QPushButton(tr("下一帧"));
    QHBoxLayout *hLayout1 = new QHBoxLayout();
    hLayout1->setSpacing(50);
    hLayout1->addWidget(previousButton);
    hLayout1->addWidget(nextButton);
    startMarkButton = new QPushButton(tr("启用标注"));
    modifyButton = new QPushButton(tr("调整库位"));
    QVBoxLayout *rightLayout1 = new QVBoxLayout();
    rightLayout1->setSpacing(30);
    rightLayout1->addLayout(hLayout1);
    rightLayout1->addWidget(startMarkButton);
    rightLayout1->addWidget(modifyButton);
    processGroundBox = new QGroupBox(tr("库位标注"));
    processGroundBox->setLayout(rightLayout1);
    processGroundBox->setEnabled(false);

    videoPathLabel = new QLabel(tr("视频路径："));
    videoPathText = new QLineEdit();
    videoPathText->setReadOnly(true);
    videoOpenButton = new QPushButton(tr("打开视频"));
    videoPathText->setReadOnly(true);
    QVBoxLayout *rightLayout3 = new QVBoxLayout();
    rightLayout3->setSpacing(20);
    rightLayout3->addWidget(videoPathLabel, Qt::AlignLeft);
    rightLayout3->addWidget(videoPathText);
    rightLayout3->addWidget(videoOpenButton);
    configGroundBox = new QGroupBox(tr("参数信息"));
    configGroundBox->setLayout(rightLayout3);

    QVBoxLayout *rightMainLayout = new QVBoxLayout();
    rightMainLayout->setSpacing(10);
    rightMainLayout->addWidget(videoInfoGroundBox);
    rightMainLayout->addWidget(processGroundBox);
    rightMainLayout->addWidget(configGroundBox);


    referenceText = new QLabel(tr("参考图"));
    referenceText->setAlignment(Qt::AlignCenter);
    drawImageText = new QLabel(tr("标注图"));
    drawImageText->setAlignment(Qt::AlignCenter);
    referenceLabel = new QLabel();
    referenceLabel->setAlignment(Qt::AlignCenter);
    drawImageLabel = new EditableLabel();
    referenceScrollArea = new QScrollArea(this);
    referenceScrollArea->setAlignment(Qt::AlignCenter);
    referenceScrollArea->setWidgetResizable(true);
    referenceScrollArea->viewport()->setBackgroundRole(QPalette::Dark);
    referenceScrollArea->viewport()->setAutoFillBackground(true);
    //referenceScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  //控件大小 小于 视窗大小时，默认不会显示滚动条
    //referenceScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    //强制显示滚动条。
    referenceScrollArea->setWidget(referenceLabel);
    drawImageScrollArea = new QScrollArea(this);
    drawImageScrollArea->setAlignment(Qt::AlignCenter);
    drawImageScrollArea->viewport()->setBackgroundRole(QPalette::Dark);
    drawImageScrollArea->viewport()->setAutoFillBackground(true);
    //drawImageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  //控件大小 小于 视窗大小时，默认不会显示滚动条
    //drawImageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    //强制显示滚动条。
    drawImageScrollArea->setWidget(drawImageLabel);
    drawImageLabel->setEnabled(false);

    QVBoxLayout *referenceLayout = new QVBoxLayout();
    referenceLayout->setSpacing(10);
    referenceLayout->addWidget(referenceScrollArea);
    referenceLayout->addWidget(referenceText);
    QVBoxLayout *drawImageLayout = new QVBoxLayout();
    drawImageLayout->setSpacing(10);
    drawImageLayout->addWidget(drawImageScrollArea);
    drawImageLayout->addWidget(drawImageText);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(referenceLayout);
    mainLayout->addLayout(drawImageLayout);
    mainLayout->addLayout(rightMainLayout);
    mainLayout->setStretchFactor(referenceLayout, 3);
    mainLayout->setStretchFactor(drawImageLayout, 3);
    mainLayout->setStretchFactor(rightMainLayout, 1);

    this->setLayout(mainLayout);
    this->setMinimumSize(1200, 600);
    this->setWindowTitle(tr("泊车库位标注"));
}

void ControlWindow::initConnect()
{
    connect(videoOpenButton, &QPushButton::clicked, this, &ControlWindow::slotOpenVideo);
    connect(previousButton, &QPushButton::clicked, this, &ControlWindow::slotPrevious);
    connect(nextButton, &QPushButton::clicked, this, &ControlWindow::slotNext);
    connect(startMarkButton, &QPushButton::clicked, this, &ControlWindow::slotStartMark);
    connect(modifyButton, &QPushButton::clicked, this, &ControlWindow::slotModify);

    connect(stopButton, &QPushButton::clicked, this, &ControlWindow::slotStop);
}

void ControlWindow::initData()
{
    currentImage = QImage(tr(":/images/images/play.png"));
    videoProcess = std::shared_ptr<VideoProcess>(new VideoProcess());
    currentFrameNum = 0;
    allCountFrame = 0;
    isStartMark = false;
    hasCanInfo = false;
    roiRect.setX(0);
    roiRect.setY(0);
    roiRect.setWidth(currentImage.width());
    roiRect.setHeight(currentImage.height());
    updateVideoProcess();
    updateImage();
}
