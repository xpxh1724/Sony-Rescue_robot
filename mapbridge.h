#ifndef MAPBRIDGE_H
#define MAPBRIDGE_H

#include<QObject>
class MapBridge: public QObject
{
    Q_OBJECT
public:
    MapBridge();
signals:
    void send_click_signal(QString lon,QString lat);   //发送点击的信号
    void send_mousemove_signal(QString str);           //发送移动的信号
public slots:
    void getCoordinate_click(QString lon,QString lat);//从js获取到鼠标点击的经纬度
    void getCoordinate_mousemove(QString lon,QString lat,QString zoom);//从js获取到鼠标移动的经纬度
};

#endif // MAPBRIDGE_H
