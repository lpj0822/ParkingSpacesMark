#pragma execution_character_set("utf-8")
#include "editablelabel.h"
#include <QMenu>
#include <QDebug>

#define COLOR_COUNT 10
#define POLYGON_EDGE 4
#define NEAR_POINT_LENGTH 10
#define RECT_WIDTH 40
#define RECT_HEIGHT 40
#define SCALE 2.5

const static QColor brushColor[COLOR_COUNT] = {QColor(163, 0, 0, 20), QColor(163, 250, 0, 20),QColor(163, 250, 148, 20),
                                              QColor(0, 250, 0, 20), QColor(0, 250, 148, 20), QColor(0, 0, 148, 20),
                                              QColor(163, 0, 148, 20), QColor(150, 100, 140, 20), QColor(100, 250, 140, 20),
                                              QColor(200, 100, 100, 20)};

EditableLabel::EditableLabel(QWidget *parent):
    QLabel(parent)
{
    initData();
    initConnect();
}

void EditableLabel::enterEvent(QEvent *e)
{
    setCursor(myDrawCursor);
}

void EditableLabel::leaveEvent(QEvent *e)
{
    drawPixmap();
}

void EditableLabel::mousePressEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(e->button() == Qt::LeftButton)
    {
        nearIndex = nearPiont(e->localPos());
        if(nearIndex >= 0)
        {
            if(this->isModify)
            {
                pointsList[nearIndex] = e->localPos();
                movePoint = true;
                setCursor(Qt::SizeAllCursor);
                return;
            }
        }
        else
        {
            pointsList.append(e->localPos());
            pointIndex++;
        }
        updatePolygons();
        drawPixmap();
    }
}

void EditableLabel::mouseMoveEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(movePoint)
    {
        pointsList[nearIndex] = e->pos();
        drawPixmap();
    }
    else
    {
        nearIndex = nearPiont(e->localPos());
        if(nearIndex >= 0 && this->isModify)
        {
            setCursor(Qt::SizeAllCursor);
        }
        else
        {
            setCursor(myDrawCursor);
        }
    }
    drawPixmap();
    scaleRectImage(e->pos());
}

void EditableLabel::mouseReleaseEvent(QMouseEvent *e)
{
    movePoint = false;
    nearIndex = -1;
}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
//    nearIndex = nearPiont(e->localPos());
//    if(nearIndex > 0)
//    {
//        pointsList.removeOne(pointsList[nearIndex]);
//        updatePolygon();
//        drawPixmap();
//    }
}

void EditableLabel::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    QPainter painter(this);
    painter.drawPixmap(QPoint(0,0), temppixmap);
    painter.end();
}

void EditableLabel::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu* popMenu = new QMenu(this);
    QList<QPolygonF> polygons = getPolygons();
    removePolygonIndex = polygons.size();
    for(int loop = 0; loop < polygons.size(); loop++)
    {
        if(isInsidePolygon(this->mapFromGlobal(QCursor::pos()), polygons[loop]))
        {
            removePolygonIndex = loop;
            break;
        }
    }
    if(removePolygonIndex >= 0 && removePolygonIndex < polygons.size())
    {
        popMenu->addAction(removePolygonAction);
    }
    else
    {
        drawPixmap();
    }
    // 菜单出现的位置为当前鼠标的位置
    popMenu->exec(QCursor::pos());
}

void EditableLabel::slotRemovePolygon()
{
    if(removePolygonIndex >= 0 && removePolygonIndex <= polygonList.size())
    {
        this->polygonList.removeAt(removePolygonIndex);
        modifyPolygonsIndex();
        drawPixmap();
        removePolygonIndex = -1;
    }
    setCursor(Qt::CrossCursor);
}

void EditableLabel::setIsModify(bool isModify)
{
    this->isModify = isModify;
}

bool EditableLabel::getIsModify()
{
    return this->isModify;
}

void EditableLabel::setTopPoint(QPoint point)
{
    topPoint = point;
}

void EditableLabel::setDrawData(const QList<QPointF>& points, const QList< QList<int> >& polygonsIndex)
{
    QList<int> tempXList;
    QList<int> tempYList;  

    int pointCount = points.size();
    int polygonCount = polygonsIndex.size();

    this->pointsList.clear();

    for(int index = 0; index < pointCount; index++)
    {
        this->pointsList.append(QPointF(points[index].x() - topPoint.x(),
                                points[index].y() - topPoint.y()));
    }

    tempXList.clear();
    tempYList.clear();
    for(int index = 0; index < polygonCount; index++)
    {
        tempXList.append(0);
        tempYList.append(0);
    }

    for(int index = 0; index < pointCount; index++)
    {
        if(!this->rect().contains(pointsList[index].x(), pointsList[index].y()))
        {
            if(pointsList[index].x() > this->width() || pointsList[index].x() < 0)
            {
                if(pointsList[index].x() > this->width())
                    pointsList[index].setX(this->width());
                else
                    pointsList[index].setX(0);

                for(int loop = 0; loop < polygonCount; loop++)
                {
                    if(polygonsIndex[loop].contains(index))
                    {
                        tempXList[loop]++;
                    }
                }
            }

            if(pointsList[index].y() > this->height() || pointsList[index].y() < 0)
            {
                if(pointsList[index].y() > this->height())
                    pointsList[index].setY(this->height());
                else
                    pointsList[index].setY(0);

                for(int loop = 0; loop < polygonCount; loop++)
                {
                    if(polygonsIndex[loop].contains(index))
                    {
                        tempYList[loop]++;
                    }
                }
            }

        }
    }

    this->polygonList.clear();
    for(int loop = 0; loop < polygonCount; loop++)
    {
        if(tempXList[loop] <= 1 && tempYList[loop] <= 1)
        {
            this->polygonList.append(polygonsIndex[loop]);
        }
        else if(tempXList[loop] <= 2 && tempYList[loop] < 1)
        {
            this->polygonList.append(polygonsIndex[loop]);
        }
    }

    modifyPolygonsIndex();
    pointIndex = this->pointsList.size() - 1;
    drawPixmap();
}

void EditableLabel::setNewQImage(QImage &image)
{
    mp = QPixmap::fromImage(image);
    this->resize(mp.width(), mp.height());
    drawPixmap();
}

QList<QPolygonF> EditableLabel::getPolygons()
{
    QList<QPolygonF> resultPolygons;
    resultPolygons.clear();
    QList<QPointF> absolutePointList = getPoints();
    for(int index1 = 0; index1 < polygonList.size(); index1++)
    {
        QList<int> polygonIndex = polygonList[index1];
        QPolygonF polygon;
        for(int index2 = 0; index2 < polygonIndex.size(); index2++)
        {
            polygon.append(absolutePointList[polygonIndex[index2]]);
        }
        resultPolygons.append(polygon);
    }

    return resultPolygons;
}

QList< QList<int> > EditableLabel::getPolygonsIndex()
{
    return polygonList;
}

QList<QPointF> EditableLabel::getPoints()
{
    QList<QPointF> absolutePointList;
    absolutePointList.clear();
    for(int index = 0; index < pointsList.size(); index++)
    {
        absolutePointList.append(QPointF(pointsList[index].x() + topPoint.x(),
                                        pointsList[index].y() + topPoint.y()));
    }
    return absolutePointList;
}

void EditableLabel::clearPoints()
{
    polygonList.clear();
    pointsList.clear();
    movePoint = false;
    nearIndex = -1;
    pointIndex = -1;
    removePolygonIndex = -1;
    drawPixmap();
}

void EditableLabel::drawPixmap()
{
    QPainter painter;
    temppixmap = mp.copy();
    painter.begin(&temppixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QColor("#3CFF55"));
    QList<QPolygonF> resultPolygons = getRelativePolygons();
    for(int i = 0; i < pointsList.size(); i++)
    {
        if(pointsList[i].x() >= 0 && pointsList[i].y() >= 0)
        {
            painter.drawEllipse(pointsList[i], 3, 3);
        }
    }

    QPen pen(QColor("#3CFF55"), 2 ,Qt::DashLine);
    painter.setPen(pen);

    for(int i = 0; i < resultPolygons.size(); i++)
    {
        QPolygonF drawpoints = resultPolygons[i];
        QPainterPath path;
        path.addPolygon(drawpoints);
        if(drawpoints.size() >= POLYGON_EDGE)
        {
            drawpoints.append(drawpoints.at(0));
            painter.fillPath(path, QBrush(brushColor[i%COLOR_COUNT]));
        }
        else
        {
            painter.fillPath(path, QBrush());
        }
        painter.drawPolyline(QPolygonF(drawpoints));
    }

    painter.end();
    this->update();
}

int EditableLabel::nearPiont(QPointF point)
{
    int resultIndex = -1;
    for(int index = 0; index < pointsList.size(); index++)
    {
        QPointF tempPoint = point - pointsList[index];
        if(tempPoint.manhattanLength() <= NEAR_POINT_LENGTH)
        {
            resultIndex = index;
            break;
        }
    }
    return resultIndex;
}

void EditableLabel::updatePolygons()
{
    int count = polygonList.size();
    if(count <= 0)
    {
        QList<int> polygon;
        polygon.clear();
        polygon.append(pointIndex);
        polygonList.append(polygon);
    }
    else
    {
        if(polygonList[count-1].size() >= POLYGON_EDGE)
        {
            QList<int> polygon;
            polygon.clear();
            if(nearIndex >= 0)
            {
                polygon.append(nearIndex);
            }
            else
            {
                polygon.append(pointIndex);
            }
            polygonList.append(polygon);
        }
        else
        {
            if(nearIndex >= 0)
            {
                polygonList[count-1].append(nearIndex);
            }
            else
            {
                polygonList[count-1].append(pointIndex);
            }
        }
    }
}

void EditableLabel::modifyPolygonsIndex()
{
    QSet<int> indexPoint;
    for(int loop = 0; loop < this->polygonList.size() ; loop++)
    {
        indexPoint = indexPoint.unite(this->polygonList[loop].toSet());
    }

    for(int index = 0; index < pointsList.size(); index++)
    {
        if(!indexPoint.contains(index))
        {
            this->pointsList[index] = QPointF(-1, -1);
        }
    }
}

QList<QPolygonF> EditableLabel::getRelativePolygons()
{
    QList<QPolygonF> resultPolygons;
    resultPolygons.clear();
    for(int index1 = 0; index1 < polygonList.size(); index1++)
    {
        QList<int> polygonIndex = polygonList[index1];
        QPolygonF polygon;
        for(int index2 = 0; index2 < polygonIndex.size(); index2++)
        {
            polygon.append(pointsList[polygonIndex[index2]]);
        }
        resultPolygons.append(polygon);
    }

    return resultPolygons;
}

bool EditableLabel::isInsidePolygon(const QPointF &point,const QPolygonF &polygon)
{

    qreal x = point.x();
    qreal y = point.y();
    int left = 0;
    int right = 0;
    int j = polygon.size() - 1;

    for(int i = 0; i < polygon.size(); i++)
    {
        if((polygon[i].y() < y && polygon[j].y() >= y)||(polygon[j].y() < y && polygon[i].y() >= y))
        {

            if((y-polygon[i].y())*(polygon[i].x()-polygon[j].x()) / (polygon[i].y()-polygon[j].y())+polygon[i].x() < x)
            {

                left++;
            }
            else
            {
                right++;
            }

        }
        j = i;
    }

    return left & right;
}

void EditableLabel::scaleRectImage(QPoint crossPoint)
{
    QPainter painter;
    int x = crossPoint.x() - RECT_WIDTH / 2;
    int y = crossPoint.y() - RECT_HEIGHT / 2;
    if(x < 0)
    {
        x = 0;
    }
    if(y < 0)
    {
        y = 0;
    }
    scalePixmap = (temppixmap.copy(x, y, RECT_WIDTH, RECT_HEIGHT)).scaled(RECT_WIDTH * SCALE, RECT_HEIGHT * SCALE);
    painter.begin(&temppixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    x = crossPoint.x() - RECT_WIDTH * SCALE / 2;
    y = crossPoint.y() - RECT_HEIGHT * SCALE / 2;
    if(x < 0)
    {
        x = 0;
    }
    if(y < 0)
    {
        y = 0;
    }
    painter.drawPixmap(x, y, scalePixmap);
    this->update();
}

void EditableLabel::initData()
{
    movePoint = false;
    nearIndex = -1;
    pointIndex = -1;
    removePolygonIndex = -1;
    this->setMouseTracking(true);
    this->isModify = false;

    removePolygonAction = new QAction(tr("删除库位"), this);

    myDrawCursor = QCursor(QPixmap(tr(":/images/images/cross.png")));
}

void EditableLabel::initConnect()
{
    connect(removePolygonAction, &QAction::triggered, this, &EditableLabel::slotRemovePolygon);
}
