#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QGraphicsView>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QString;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget* parent = 0);

protected:
    void wheelEvent(QWheelEvent *event);

public slots:
    void paintImage(const QString& filePath);


private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem* imageItem;
};

#endif // VIEW_H
