#include "systemstate.h"
#include "ui_systemstate.h"

SystemState::SystemState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemState)
{
    ui->setupUi(this);
    ui->FeedingType->setText("正常");
    ui->TempType->setText("正常");
    ui->WaterLevelType->setText("正常");
    ui->DianYuanType->setText("84%");
}

SystemState::~SystemState()
{
    delete ui;
}

void SystemState::updateMqttType(int mqttType)
{
    if(mqttType==0)
        ui->PCType->setPixmap(QPixmap(":/ptr/NOtype.png"));
    else
        ui->PCType->setPixmap(QPixmap(":/ptr/OKtype.png"));

}

void SystemState::updateTcpType(int tcpType)
{
    if(tcpType==0)
        ui->YJType->setPixmap(QPixmap(":/ptr/NOtype.png"));
    else
        ui->YJType->setPixmap(QPixmap(":/ptr/OKtype.png"));
}
