#ifndef DRAWAREA_H
#define DRAWAREA_H

#include <QWidget>
#include <QPainter>

class DrawArea : public QWidget
{
    Q_OBJECT
public:
    explicit DrawArea(QWidget *parent = nullptr);
    void setPositions(QPoint s1, QPoint s2, QPoint target);
signals:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPoint sensor1Pos;
    QPoint sensor2Pos;
    QPoint targetPos;

};

#endif // DRAWAREA_H
