#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QLineEdit>
#include <QKeyEvent>
#include <QCheckBox>
#include <QSplitter>
#include "editablelabel.h"

#include "utility/videoprocess.h"

#include <memory>

class ControlWindow : public QWidget
{
    Q_OBJECT

public:
    ControlWindow(QWidget *parent = 0);
    ~ControlWindow();

public slots:

    void slotOpenVideo();
    void slotPrevious();
    void slotNext();
    void slotStartMark();
    void slotModify();
    void slotStop();

protected:

    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *e);

private:

    void reviewPrevious();
    void reviewNext();

    void updateButton();
    void updateVideoProcess();
    void updateImage();

    QRect initRoi(const QString &videoPath);

    QList<QPointF> trackingWidthCan(const QList<QPointF> &pointList, const QString canFilePath,
                                    const QList< QList<int> > &polygonsIndex);

    void saveResult();
    void readResult(QList<QPointF> &pointList, QList< QList<int> > &polygonsIndex);

    void initData();
    void initUI();
    void initConnect();

private:

    QLabel *infoLabel;
    QLabel *videoProcessLabel;
    QLabel *videoProcessShow;
    QPushButton *stopButton;
    QGroupBox *videoInfoGroundBox;


    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *startMarkButton;
    QPushButton *modifyButton;
    QGroupBox *processGroundBox;

    QLabel *videoPathLabel;
    QLineEdit *videoPathText;
    QPushButton *videoOpenButton;
    QGroupBox *configGroundBox;

    QLabel *referenceLabel;
    EditableLabel *drawImageLabel;
    QLabel *referenceText;
    QLabel *drawImageText;
    QScrollArea *referenceScrollArea;
    QScrollArea *drawImageScrollArea;

    std::shared_ptr<VideoProcess> videoProcess;
    int currentFrameNum;
    int allCountFrame;
    QString resultSavePath;
    QString canInfoPath;

    QRect roiRect;
    QImage currentImage;
    cv::Mat frame;

    bool isStartMark;

    bool hasCanInfo;
};

#endif // CONTROLWINDOW_H
