#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QCanBus>

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();

    struct Settings {
        QString pluginName = "";
        QString deviceInterfaceName = "";
    };

    QList<QCanBusDeviceInfo> m_interfaces;

public slots:
    void addToComboBoxPlugins();
    void addToComboBoxCAN();
    Settings setting();
    void updateSettings();
    void ok();
    void cancel();

private:
    Ui::ConnectDialog *ui;
    Settings param;
};

#endif // CONNECTDIALOG_H
