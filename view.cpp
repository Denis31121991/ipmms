#include "view.h"

#include <QGraphicsPixmapItem>
#include <QString>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QBrush>
#include <QDebug>

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setBackgroundBrush(QBrush(Qt::gray));
    scene = new QGraphicsScene(this);
    setScene(scene);
    imageItem = new QGraphicsPixmapItem(0, scene);
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    double scaleFactor = 1.15;

    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->delta() > 0)
        {
            scale(scaleFactor, scaleFactor);
        }
        else
        {
            scale(1 / scaleFactor, 1 / scaleFactor);
        }

        event->accept();
    }
    else
    {
        QGraphicsView::wheelEvent(event);
    }
}

void GraphicsView::paintImage(const QString &filePath)
{
    imageItem->setPixmap(QPixmap(filePath));
    scene->setSceneRect(imageItem->boundingRect());
}
