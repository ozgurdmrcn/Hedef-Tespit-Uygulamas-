#include "drawarea.h"

DrawArea::DrawArea(QWidget *parent) : QWidget(parent) {}

void DrawArea::setPositions(QPoint s1, QPoint s2, QPoint target) {
    sensor1Pos = s1;
    sensor2Pos = s2;
    targetPos = target;
    update();
}

void DrawArea::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    auto scale = [](const QPoint& p) {
        return QPoint(p.x() / 2 + 50, p.y() / 2 + 50);
    };

    painter.setBrush(Qt::blue);
    painter.drawEllipse(scale(sensor1Pos), 5, 5);
    painter.drawText(scale(sensor1Pos) + QPoint(5, -5), "S1");

    painter.setBrush(Qt::green);
    painter.drawEllipse(scale(sensor2Pos), 5, 5);
    painter.drawText(scale(sensor2Pos) + QPoint(5, -5), "S2");

    painter.setBrush(Qt::red);
    painter.drawEllipse(scale(targetPos), 6, 6);
    painter.drawText(scale(targetPos) + QPoint(5, -5), "Hedef");
}
