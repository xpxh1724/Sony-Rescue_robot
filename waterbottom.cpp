#include "waterbottom.h"
#include "ui_waterbottom.h"

WaterBottom::WaterBottom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaterBottom)
{
    ui->setupUi(this);
}

WaterBottom::~WaterBottom()
{
    delete ui;
}
//通过值来判断对应范围数值的背景颜色：正常、提示、警告
void WaterBottom::ValueColorIsChanged(double nowValue, double zcValue1, double zcValue2, double infoValue1, double infoValue2,double infoValue3,double infoValue4, QWidget *widget)
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
void WaterBottom::setValue(double *Data)
{
    ui->O2Value->setText(QString("%1").arg(Data[0]));
    ui->CombustibleGasValue->setText(QString("%1").arg(Data[1]));
    ui->NO2Value->setText(QString("%1").arg(Data[2]));
    ui->COValue->setText(QString("%1").arg(Data[3]));
    ui->NH3Value->setText(QString("%1").arg(Data[4]));
    ui->LngValue->setText(QString("%1").arg(Data[5]));
    ui->LatValue->setText(QString("%1").arg(Data[6]));

//    ValueColorIsChanged(Data[0],6.5,8.5,6.0,6.5,8.5,9.0,ui->O2Widget);
//    ValueColorIsChanged(Data[1],200,500,150,200,500,800,ui->CombustibleGasWidget);
//    ValueColorIsChanged(Data[2],1.5,3.0,1.0,1.5,3.0,4.0,ui->NO2Widget);
//    ValueColorIsChanged(Data[3],1.5,3.0,1.0,1.5,3.0,4.0,ui->COWidget);
//    ValueColorIsChanged(Data[4],0,10,10,20,10,20,ui->NH3Widget);
//    ValueColorIsChanged(Data[5],20,30,15,20,30,35,ui->LngWidget);
//    ValueColorIsChanged(Data[6],20,30,15,20,30,35,ui->LatWidget);

}

