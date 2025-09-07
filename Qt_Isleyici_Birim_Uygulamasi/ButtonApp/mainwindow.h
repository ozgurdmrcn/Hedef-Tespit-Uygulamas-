#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <cmath>

struct SensorData {
    int x;
    int y;
    double angle_deg;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;

    bool parseResponse(const QString& response, SensorData& s1, SensorData& s2);
    QPoint estimateTarget(const SensorData& s1, const SensorData& s2);
private:
    int counter = 0;
};
#endif // MAINWINDOW_H
