#include "menu.h"
#include "ui_menu.h"
#include <QDateTime>
#include<QDebug>
Menu::Menu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);
    //最小化窗口-信号
    connect(ui->widget_top_btn_Min,&QPushButton::clicked,this,&Menu::showMin);
    //最大化-还原窗口-信号
    connect(ui->widget_top_btn_Max,&QPushButton::clicked,this,[=](bool ok){
        if(ok)
        {
            emit showMax();
            ui->widget_top_btn_Max->setIcon(QPixmap(":/ptr/Max1-1.png"));
        }else {
            emit showNormal();
            ui->widget_top_btn_Max->setIcon(QPixmap(":/ptr/Max1.png"));
        }
    });
    //关闭窗口-信号
    connect(ui->widget_top_btn_Close,&QPushButton::clicked,this,&Menu::showClose);
    QString defaultStyleSheet = "QToolButton{color: rgb(122, 175, 227);font: 10pt '楷体';border-radius:6px;}\
    QToolButton:hover{background-color: rgb(19, 48, 80);background-color: rgb(3, 57, 103);font: 10pt '楷体';border-radius:6px;border:2px solid rgb(13,39,67);}";
    QString pressedStyleSheet = "QToolButton{background-color: rgb(19, 48, 80);background-color: rgb(3, 57, 103);font: 10pt '楷体';border-radius:6px;border:2px solid rgb(13,39,67);}";
    //去实时数据页面-信号
    connect(ui->btn_Real_time,&QPushButton::clicked,this,[=]{
        emit toRealTimePage();
        ui->btn_Real_time->setStyleSheet(pressedStyleSheet);
        ui->btn_Record_query->setStyleSheet(defaultStyleSheet);
        ui->btn_Hot_video->setStyleSheet(defaultStyleSheet);
        ui->btn_Control->setStyleSheet(defaultStyleSheet);
        ui->btn_Fault_detect->setStyleSheet(defaultStyleSheet);

    });
    //去记录查询页面-信号
    connect(ui->btn_Record_query,&QPushButton::clicked,this,[=]{
        emit toRecordQueryPage();
        ui->btn_Real_time->setStyleSheet(defaultStyleSheet);
        ui->btn_Record_query->setStyleSheet(pressedStyleSheet);
        ui->btn_Hot_video->setStyleSheet(defaultStyleSheet);
        ui->btn_Control->setStyleSheet(defaultStyleSheet);
        ui->btn_Fault_detect->setStyleSheet(defaultStyleSheet);
    });
    //去记录查询页面-信号
    connect(ui->btn_Hot_video,&QPushButton::clicked,this,[=]{
        emit toHotVideoPage();
        ui->btn_Real_time->setStyleSheet(defaultStyleSheet);
        ui->btn_Record_query->setStyleSheet(defaultStyleSheet);
        ui->btn_Hot_video->setStyleSheet(pressedStyleSheet);
        ui->btn_Control->setStyleSheet(defaultStyleSheet);
        ui->btn_Fault_detect->setStyleSheet(defaultStyleSheet);
    });
    //去控制管理页面-信号
    connect(ui->btn_Control,&QPushButton::clicked,this,[=]{
        emit toControlPage();
        ui->btn_Real_time->setStyleSheet(defaultStyleSheet);
        ui->btn_Record_query->setStyleSheet(defaultStyleSheet);
        ui->btn_Hot_video->setStyleSheet(defaultStyleSheet);
        ui->btn_Control->setStyleSheet(pressedStyleSheet);
        ui->btn_Fault_detect->setStyleSheet(defaultStyleSheet);
    });
    //去故障检测页面-信号
    connect(ui->btn_Fault_detect,&QPushButton::clicked,this,[=]{
        emit toFaultDetectPage();
        ui->btn_Real_time->setStyleSheet(defaultStyleSheet);
        ui->btn_Record_query->setStyleSheet(defaultStyleSheet);
        ui->btn_Hot_video->setStyleSheet(defaultStyleSheet);
        ui->btn_Control->setStyleSheet(defaultStyleSheet);
        ui->btn_Fault_detect->setStyleSheet(pressedStyleSheet);
    });
    //实时时间显示
    UpdataTime=new QTimer;
    connect(UpdataTime,&QTimer::timeout,this,[=]{
        // 获取当前本地时间
        QDateTime currentDateTime = QDateTime::currentDateTime();
        ui->widget_top_year->setText(currentDateTime.toString("yyyy年MM月dd日"));
        ui->widget_top_time->setText(currentDateTime.toString("HH:mm:ss"));
    });
    UpdataTime->start(1000);
}

Menu::~Menu()
{
    delete ui;
}
