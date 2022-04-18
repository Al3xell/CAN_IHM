#include "connectdialog.h"
#include "ui_connectdialog.h"



#include <QCanBus>

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Connection to CAN");
    addToComboBoxPlugins();
    addToComboBoxCAN();

    connect(ui->comboBoxPlugin, SIGNAL(currentTextChanged(QString)), this, SLOT(addToComboBoxCAN()));
    connect(ui->comboBoxPort, SIGNAL(currentTextChanged(QString)), this, SLOT(updateSettings()));
    connect(ui->okButton,SIGNAL(released()),this,SLOT(ok()));
    connect(ui->cancelButton,SIGNAL(released()),this,SLOT(cancel()));
}

void ConnectDialog::addToComboBoxPlugins()
{
    QStringList plugins = QCanBus::instance()->plugins();
    ui->comboBoxPlugin->addItems(plugins);
}

void ConnectDialog::addToComboBoxCAN()
{
    m_interfaces = QCanBus::instance()->availableDevices(ui->comboBoxPlugin->currentText());
    ui->comboBoxPort->clear();
    for(int i = 0; i<m_interfaces.count(); i++)
    {
        ui->comboBoxPort->addItem(m_interfaces[i].name());
    }
    updateSettings();
}

ConnectDialog::Settings ConnectDialog::setting()
{
    return param;
}
void ConnectDialog::updateSettings()
{
    param.pluginName = ui->comboBoxPlugin->currentText();
    param.deviceInterfaceName = ui->comboBoxPort->currentText();
    m_interfaces = QCanBus::instance()->availableDevices(ui->comboBoxPlugin->currentText());
}

void ConnectDialog::ok()
{
    updateSettings();
    ui->labelValidation->setText("Connect to : "+param.deviceInterfaceName);
    accept();
}

void ConnectDialog::cancel()
{
    reject();
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}
