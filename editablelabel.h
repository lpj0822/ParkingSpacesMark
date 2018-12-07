#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDateTime>
#include <QToolButton>
#include <QAction>
#include <QCursor>

class EditableLabel : public QLabel
{
public:
    EditableLabel(QWidget *parent = 0);

    void setDrawData(const QList<QPointF>& points, const QList< QList<int> >& polygonsIndex);

    void clearPoints();
    void setNewQImage(QImage &image);

    QList<QPolygonF> getPolygons();
    QList< QList<int> > getPolygonsIndex();
    QList<QPointF> getPoints();

    void setIsModify(bool isModify);
    bool getIsModify();

    void setTopPoint(QPoint point);

public slots:

    void slotRemovePolygon();

protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent*e);
    void paintEvent(QPaintEvent *e);

    void contextMenuEvent(QContextMenuEvent * event);

private:

    int nearPiont(QPointF point);
    void updatePolygons();
    void drawPixmap();
    void modifyPolygonsIndex();

    QList<QPolygonF> getRelativePolygons();

    void scaleRectImage(QPoint crossPoint);

    bool isInsidePolygon(const QPointF &point,const QPolygonF &polygon);

    void initData();
    void initConnect();

private:

    QAction *removePolygonAction;

    bool movePoint;
    int pointIndex;
    int nearIndex;
    int removePolygonIndex;

    QList< QList<int> > polygonList;
    QList<QPointF> pointsList;

    QPixmap mp;
    QPixmap temppixmap;

    QPixmap scalePixmap;

    QCursor myDrawCursor;

    bool isModify;
    QPoint topPoint;
};

#endif // EDITABLELABEL_H
