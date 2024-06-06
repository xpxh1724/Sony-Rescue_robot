#include "watertop.h"
#include "ui_watertop.h"
#include<QDebug>
WaterTop::WaterTop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaterTop)
{
    ui->setupUi(this);
}

WaterTop::~WaterTop()
{
    delete ui;
}
//通过值来判断对应范围数值的背景颜色：正常、提示、警告
void WaterTop::ValueColorIsChanged(double nowValue, double zcValue1, double zcValue2, double infoValue1, double infoValue2,double infoValue3,double infoValue4, QWidget *widget)
{
    QString InfromtionQss=QString("#%1{background: qradialgradient(cx: 0.5, cy: 0.5, radius: 1, fx: 0.5, fy: 0.5, stop: 0 rgba(7, 46, 125, 0), stop: 1 rgba(183, 172, 31, 0.6));}").arg(widget->objectName());
    QString WarningQss=QString("#%1{background: qradialgradient(cx: 0.5, cy: 0.5, radius: 1, fx: 0.5, fy: 0.5, stop: 0 rgba(7, 46, 125, 0), stop: 1 #d5442b);}").arg(widget->objectName());
      if(nowValue>=zcValue1&&nowValue<=zcValue2)
    {
        widget->setStyleSheet("");
    }
    else if((nowValue>=infoValue1&&nowValue<infoValue2) ||(nowValue>infoValue3&&nowValue<=infoValue4))
    {
        widget->setStyleSheet(InfromtionQss);
    }
    else {
        widget->setStyleSheet(WarningQss);
    }
}
//设置值
void WaterTop::setValue(double *Data)
{
    ui->TempValue->setText(QString("%1").arg(Data[0]));
    ui->HumidityValue->setText(QString("%1").arg(Data[1]));
    ui->PressureValue->setText(QString("%1").arg(Data[2]));
    ui->UltravioletValue->setText(QString("%1").arg(Data[3]));
    ui->LightValue->setText(QString("%1").arg(Data[4]));
//    ValueColorIsChanged(Data[0],20,30,15,20,30,35,ui->TempWidget);
//    ValueColorIsChanged(Data[1],60,80,50,60,80,90,ui->HumidityWidget);
//    ValueColorIsChanged(Data[2],1010,1020,1000,1010,1020,1030,ui->PressureWidget);
//    ValueColorIsChanged(Data[3],0,20,20,40,20,40,ui->UltravioletWidget);
//    ValueColorIsChanged(Data[4],500,1000,300,500,1000,1500,ui->LightWidget);
}

