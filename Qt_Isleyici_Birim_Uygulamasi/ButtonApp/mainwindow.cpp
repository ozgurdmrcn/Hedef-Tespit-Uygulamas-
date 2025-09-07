#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRegularExpressionMatch>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    ui->pushButton->adjustSize();


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    socket->connectToHost("127.0.0.1", 8090);

       if (socket->waitForConnected(3000))
       {
           socket->write("GET_TARGET");

           if (socket->waitForReadyRead(3000))
           {
               QString response = socket->readAll();
               SensorData s1, s2;

               if (parseResponse(response, s1, s2))
               {
                   ui->labelSensor1->setText(QString("Sensor-1 → X: %1, Y: %2, Açı: %3°")
                                             .arg(s1.x).arg(s1.y).arg(s1.angle_deg));

                   ui->labelSensor2->setText(QString("Sensor-2 → X: %1, Y: %2, Açı: %3°")
                                             .arg(s2.x).arg(s2.y).arg(s2.angle_deg));

                   QPoint target = estimateTarget(s1, s2);
                   ui->labelTarget->setText(QString("Hedef → X: %1, Y: %2")
                                            .arg(target.x()).arg(target.y()));
               }
               else
               {
                   ui->labelTarget->setText("Veri ayrıştırılamadı.");
               }
           }
           else
           {
               ui->labelTarget->setText("Sunucudan veri alınamadı.");
           }
       }
       else
       {
           ui->labelTarget->setText("Bağlantı başarısız.");
       }

    ui->labelSensor1->adjustSize();
    ui->labelSensor2->adjustSize();
    ui->labelTarget->adjustSize();
}


bool MainWindow::parseResponse(const QString& response, SensorData& s1, SensorData& s2)
{
    QStringList lines = response.split("\n", QString::SkipEmptyParts);
    QRegularExpression regex(R"(Sensor-(\d): \((\d+),\s*(\d+)\), Angle: ([\d\.]+))");

    int found = 0;
    for (const QString& line : lines)
    {
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch())
        {
            int id = match.captured(1).toInt();
            int x = match.captured(2).toInt();
            int y = match.captured(3).toInt();
            double angle = match.captured(4).toDouble();

            if (id == 1)
            {
                s1 = {x, y, angle};
                found++;
            }
            else if (id == 2)
            {
                s2 = {x, y, angle};
                found++;
            }
        }
    }
    return found == 2;
}

QPoint MainWindow::estimateTarget(const SensorData& s1, const SensorData& s2)
{
    double theta1 = qDegreesToRadians(s1.angle_deg);
    double theta2 = qDegreesToRadians(s2.angle_deg);

    double dx1 = qCos(theta1);
    double dy1 = qSin(theta1);
    double dx2 = qCos(theta2);
    double dy2 = qSin(theta2);

    double det = dx1 * dy2 - dy1 * dx2;
    if (qFabs(det) < 1e-8)
    {
        return QPoint(-1, -1);
    }

    double x_diff = s2.x - s1.x;
    double y_diff = s2.y - s1.y;
    double t1 = (x_diff * dy2 - y_diff * dx2) / det;

    double x_target = s1.x + t1 * dx1;
    double y_target = s1.y + t1 * dy1;

    return QPoint(qRound(x_target), qRound(y_target));
}
