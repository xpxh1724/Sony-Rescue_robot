#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>//关闭窗口事件头文件
#include "menu.h"//菜单类
#include "watertop.h"//四肢数据类
#include "waterbottom.h"//机械臂数据类
#include "systemstate.h"//系统状态类
#include "log.h"//日志类
#include "videothread.h"  //视频线程头文件
#include<QtMqtt/qmqttclient.h>
#include<QJsonDocument>
#include<QJsonObject>
#include<QTcpSocket>
//#include "gifthread.h"//GIF线程头文件
//要包含下面的两个文件，必须在.pro文件中添加  QT += multimedia  multimediawidgets
#include <QMediaPlayer>
#include <QVideoWidget>

#include <QWebEngineView> //用于加载和显示Web页面
#include <QStackedLayout> //堆叠布局管理器头文件
#include <QWebChannel>    //用于web与qt双向通信
#include "mapbridge.h"    //自定义与JS通信类
#include "biaozhuwidget.h"//自定义标注控件
#include <QVector>        //容器头文件
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void init();
    //---------- 窗口移动事件(重写) ----------
    //鼠标按下事件
    void mousePressEvent(QMouseEvent *event);
    //鼠标释放时间
    void mouseReleaseEvent(QMouseEvent *event);
    //鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event);
    //关闭窗口事件
    void closeEvent(QCloseEvent *event);
    //DockWidget窗口初始化
    void DockWidgetInit();
    //RealTimePage页面初始化
    void RealTimePageInit();
    //操作控制页面初始化
    void OperationControlPageInit();
    //处理来自小程序中间件的消息
    void handleAppletMsg(QString Msg);
private slots:
    //-------------------Voide-------------------
    //将视频线程对象传递的每一帧图像设置到视频显示窗口
    void onFrameReady(QImage frame);
    //btn-播放
    void on_btn_video_start_clicked();
    //重复播放视频
    void onVideoStateChanged(QMediaPlayer::State state);

    //--------------------Map--------------------
    //接收点击信号的槽函数
    void recv_click_slot(QString lon,QString lat);
    //接收移动信号的槽函数
    void recv_mousemove_slot(QString str);
    //btn-Map添加标注
    void on_btn_Map_addBZ_clicked();
    //btn-Map-移除标注
    void on_btn_Map_moveBZ_clicked();
    //为标注分配名称
    QString BiaoZhuName();
    //btn-Map-路径规划
    void on_btn_Map_LJGH_clicked();
    //btn-Map-清除地图上所有覆盖物
    void on_btn_Map_ClearAllBz_clicked();
    //显示运动轨迹
    void mapDrawTrajectory();
    //btn-Map-开启/关闭轨迹
    void on_btn_Map_YDGJ_clicked();

signals:

private:
    Ui::MainWindow *ui;
    bool m_bIsPressed=false;	//鼠标按下标志位
    QPoint m_lastPt;	//记录第一次鼠标按下的局部坐标
    Menu *menu;//菜单-窗口
    WaterTop *waterTop;//四肢数据-窗口
    WaterBottom *waterBottom;//机械臂数据-窗口
    SystemState *systemState;//系统状态-窗口
    Log *log;//日志-窗口
    //------------ 视频流 ------------------------
    VideoThread *m_videoThread;       //创建视频线程对象
    //------------ Map --------------------------
    QStackedLayout *layout;           //创建堆叠布局管理器
    QWebEngineView *mainMapView;      //用于加载网页
    MapBridge *mybridge;               //js与qt的桥梁对象
    QVector<BiaoZhuWidget*>BzVector;  //tableview标注列表控制容器
    QMap<int,QString>BZName;          //用于控制tableview上增添删除自定义控件checkbox控件的命名
    QString carLng,carLat;            //小车经纬度
    bool isDrawTrajectoryFlag=false;  //是否添加小车运动轨迹点
    //-------------热感----------------------------
    QWebEngineView *m_webView;
    //------------ MQTT  ------------------------
    QMqttClient *m_client;
    QString topicName=QString("Data");
    //指令
    QJsonObject msg_json;
    QTcpSocket *m_tcp;//用于连接小程序对应的服务端，接收小程序消息
    QTimer *TankTimer;
    int TankZL=5;

    //GIF视频
    //GifThread *t1;
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
};

#endif // MAINWINDOW_H
