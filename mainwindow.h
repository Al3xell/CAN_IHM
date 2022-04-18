#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "connectdialog.h"

#include <QCanBusDevice>
#include <QCanBus>
#include <QTimer>
#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onExit();
    void connectCan();
    void disconnectCan();
    void readFrame();
    void sendFrame();
    void updateInput();
    void updateSlider();

private:
    ConnectDialog *m_connectDialog = nullptr;
    QCanBusDevice *m_canDevice = nullptr;
    Ui::MainWindow *ui;
    QTimer *timer = new QTimer(this);
};
#endif // MAINWINDOW_H
