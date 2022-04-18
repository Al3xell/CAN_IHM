#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>
#include <QtDebug>
#include <QMessageBox>
#include <cmath>


#define  RETURN  0x01
#define  COMMAND 0x02

#define  MPU     0x10

#define  VL6180X 0x20
#define  LUM     0x21
#define  DIST    0x22

#define  ANEMO   0x30

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("IHM CAN");

    m_connectDialog = new ConnectDialog;

    ui->actionDisconnect->setEnabled(false);
    ui->actionConnect->setEnabled(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(sendFrame()));
    connect(ui->actionConnect, SIGNAL(triggered()), m_connectDialog, SLOT(show()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onExit()));
    connect(ui->actionDisconnect,SIGNAL(triggered()),this,SLOT(disconnectCan()));
    connect(m_connectDialog, SIGNAL(accepted()), this, SLOT(connectCan()));
    connect(ui->inputSpeed, SIGNAL(returnPressed()), this, SLOT(updateSlider()));
    connect(ui->sliderSpeed, SIGNAL(sliderReleased()), this, SLOT(updateInput()));

    timer->start(80);

}

void MainWindow::onExit()
{
    QMessageBox warning;
    warning.setText("Are you sure ?");
    warning.setIcon(QMessageBox::Warning);
    warning.addButton(QMessageBox::Ok);
    warning.addButton(QMessageBox::Cancel);
    int answer = warning.exec();
    if(answer == 0x00000400)
    {
        this->close();
    }
}

void MainWindow::connectCan()
{
    const ConnectDialog::Settings p = m_connectDialog->setting();

    if((p.deviceInterfaceName==""||p.pluginName==""))
    {
        QMessageBox warning;
        warning.setText("Error ! Invalid Info !");
        warning.setIcon(QMessageBox::Warning);
        warning.addButton(QMessageBox::Ok);
        warning.exec();
        return;
    }

    m_canDevice = QCanBus::instance()->createDevice(p.pluginName, p.deviceInterfaceName);


    m_canDevice->connectDevice();
    this->setWindowTitle("IHM CAN : "+p.deviceInterfaceName);
    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setEnabled(false);

    QMessageBox info;
    info.setText("Connected to : "+p.deviceInterfaceName);
    info.setIcon(QMessageBox::Information);
    info.addButton(QMessageBox::Ok);
    info.exec();

}

void MainWindow::disconnectCan()
{
    if(!m_canDevice)
    {
        QMessageBox warning;
        warning.setText("Not connected to any CAN !");
        warning.setIcon(QMessageBox::Warning);
        warning.addButton(QMessageBox::Ok);
        warning.exec();
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setEnabled(false);
        return;
    }
    else
    {
        m_canDevice->disconnectDevice();

        this->setWindowTitle("IHM CAN");

        ui->actionDisconnect->setEnabled(false);
        ui->actionConnect->setEnabled(true);

        QMessageBox info;
        info.setText("Disconnected");
        info.setIcon(QMessageBox::Information);
        info.addButton(QMessageBox::Ok);
        info.exec();

    }
}
void MainWindow::sendFrame()
{
    QByteArray transmitMPU;
    QByteArray transmitVL;
    QByteArray transmitANE;


    if(m_canDevice)
    {
        transmitMPU[0] = MPU;
        transmitVL[0] = VL6180X;
        if(ui->radioButtonDis->isChecked())
        {
            transmitVL[1] = DIST;
        }
        else
        {
            transmitVL[1] = LUM;
        }
        transmitANE[0] = ANEMO;

        QCanBusFrame transmitMPUFrame = QCanBusFrame(RETURN, transmitMPU);
        m_canDevice->writeFrame(transmitMPUFrame);
        QEventLoop loop;
        QTimer::singleShot(20, &loop, SLOT(quit()));
        loop.exec();
        readFrame();
        QCanBusFrame transmitVLFrame = QCanBusFrame(RETURN, transmitVL);
        m_canDevice->writeFrame(transmitVLFrame);
        QTimer::singleShot(20, &loop, SLOT(quit()));
        loop.exec();
        readFrame();
        QCanBusFrame transmitANEFrame = QCanBusFrame(RETURN, transmitANE);
        m_canDevice->writeFrame(transmitANEFrame);
        QTimer::singleShot(20, &loop, SLOT(quit()));
        loop.exec();
        readFrame();
        if(ui->radioButtonMoteurOn->isChecked())
        {
            transmitANE[0] = ANEMO;
            transmitANE[1] = 0xAA;
            transmitANE[2] = ui->sliderSpeed->value()*2.55;


            QCanBusFrame commandMoteurFrame = QCanBusFrame(COMMAND, transmitANE);
            m_canDevice->writeFrame(commandMoteurFrame);
            QTimer::singleShot(20, &loop, SLOT(quit()));
            loop.exec();
            readFrame();
        }
        else
        {
            transmitANE[0] = ANEMO;
            transmitANE[1] = 0xBB;
            transmitANE[2] = 0x00;
            QCanBusFrame commandMoteurFrame = QCanBusFrame(COMMAND, transmitANE);
            m_canDevice->writeFrame(commandMoteurFrame);
            QTimer::singleShot(20, &loop, SLOT(quit()));
            loop.exec();
            readFrame();
        }

    }
}

void MainWindow::readFrame()
{
    if(m_canDevice == nullptr)
    {
        return;
    }
    else if(!(m_canDevice->framesAvailable()))
    {
        return;
    }
    else if(m_canDevice->framesAvailable() == 1)
    {
        QCanBusFrame frame = m_canDevice->readFrame();
        QByteArray received = frame.payload();

        if(frame.frameId()==0x21)
        {
            uint16_t lum = (received[0] << 8) + received[1];

            ui->lcdLumDist->display(lum);
        }
        if(frame.frameId()==0x22)
        {
            uint16_t dist = (received[0] << 8) + received[1];
            if(dist >= 100)
            {
                ui->lcdLumDist->display("ERR");
            }
            else
            {
                ui->lcdLumDist->display(dist);
            }

        }
        if(frame.frameId()==0x31)
        {
            uint16_t pression = (received[0] << 8) + received[1];
            uint16_t humidite = (received[2] << 8) + received[3];
            uint16_t vitesse =  (received[4] << 8) + received[5];
            double temp = (double)((uint16_t)(received[6] << 8) + received[7])/100;
            //qInfo() << pression;

            qInfo() << frame.toString();


            ui->lcdPression->display(pression);
            ui->lcdHumidite->display(humidite);
            ui->lcdVent->display(vitesse);
            ui->lcdTemperature->display(temp);
        }
    }
    else
    {
        QVector<QCanBusFrame> frames = m_canDevice->readAllFrames();
        for(int i = 0; i<frames.count(); i++)
        {
            QByteArray received = frames[i].payload();

            if(frames[i].frameId()==0x11)
            {
                int phi = (uint8_t)((received[0] << 8) + received[1])-200;
                int theta = (uint8_t)((received[2] << 8) + received[3])-200;
                int psi = (uint8_t)((received[4] << 8) + received[5])-200;

                ui->phi->setText(QString::number(phi));
                ui->theta->setText(QString::number(theta));
                ui->psi->setText(QString::number(psi));
            }
            if(frames[i].frameId()==0x12)
            {
                float ax = (float)((received[0] << 8) + received[1])/100-100;
                float ay = (float)((received[2] << 8) + received[3])/100-100;
                float az = (float)((received[4] << 8) + received[5])/100-100;

                ui->accelX->setText(QString::number(ax));
                ui->accelY->setText(QString::number(ay));
                ui->accelZ->setText(QString::number(az));
            }
            if(frames[i].frameId()==0x13)
            {
                float gx = (float)((received[0] << 8) + received[1])/100-100;
                float gy = (float)((received[2] << 8)+ received[3])/100-100;
                float gz = (float)((received[4] << 8) + received[5])/100-100;

                ui->gyroX->setText(QString::number(gx));
                ui->gyroY->setText(QString::number(gy));
                ui->gyroZ->setText(QString::number(gz));
            }

        }
    }
}

void MainWindow::updateInput()
{
    ui->inputSpeed->setText(QString::number(ui->sliderSpeed->value()));
}

void MainWindow::updateSlider()
{
    int value = (ui->inputSpeed->text()).toInt();
    if(value > 100)
    {
        value = 100;
    }
    ui->inputSpeed->setText(QString::number(value));
    ui->sliderSpeed->setValue(value);
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_canDevice;
    delete m_connectDialog;
}

