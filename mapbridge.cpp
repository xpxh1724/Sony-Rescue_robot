#include "mapbridge.h"
#include<QDebug>
MapBridge::MapBridge()
{

}

void MapBridge::getCoordinate_click(QString lon, QString lat)
{
    QString str=QString("%1,%2").arg(lon).arg(lat);
    qDebug()<<str;  //获取鼠标点击位置的地图坐标
    emit send_click_signal(lon,lat); //发送信号
}

void MapBridge::getCoordinate_mousemove(QString lon, QString lat,QString zoom)
{
    QString str=QString("%1,%2,%3").arg(lon).arg(lat).arg(zoom);
    qDebug()<<str;  //获取鼠标点击位置的地图坐标
    emit send_mousemove_signal(str);//发送信号
}
