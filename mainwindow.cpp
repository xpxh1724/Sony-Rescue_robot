#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QComboBox>
#include <QDateTime>
#include<QDebug>
#include<QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkProxyFactory>
#include <QScrollBar>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    TankTimer=new QTimer;
    connect(TankTimer,&QTimer::timeout,this,[=]{
        QJsonObject obj;
        obj.insert("0",TankZL);
        QJsonDocument doc(obj);
        QByteArray msg= doc.toJson();
        m_client->publish(QString("Tank"),msg);
        qDebug()<<msg<<endl;
        TankTimer->stop();
    });
    TankTimer->start(800);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//初始化
void MainWindow::init()
{
    setWindowTitle("SONY-DOG");
    setWindowFlags(Qt::FramelessWindowHint);//设置无系统边框  //Qt::WindowStaysOnTopHint窗口最高权限-窗口始终在最上层
    //this->setAttribute(Qt::WA_TranslucentBackground, true);//设置透明-窗体标题栏不透明,背景透明
    setGeometry(110,30,geometry().width(),geometry().height());//窗口初始显示位置
    DockWidgetInit();//DockWidget窗口初始化
    ui->stackedWidget->setCurrentIndex(0);
    //------------------------------------------- MQTT --------------------------------------------------------
    {
        m_client = new QMqttClient(this);
        m_client->setHostname("49.232.149.245");
        m_client->setPort(quint16(1883));
        // 监听 QMqttClient 的状态变化
        connect(m_client, &QMqttClient::stateChanged, this, [=](QMqttClient::ClientState state) {
            if (state == QMqttClient::Connected) {
                log->appendLog("MQTT连接成功");
                systemState->updateMqttType(1);
                auto subscription= m_client->subscribe(topicName);
                if (!subscription) {
                    log->appendLog("订阅主题失败");
                }else{log->appendLog("订阅主题成功");}
            }
            else if (state == QMqttClient::Disconnected) {log->appendLog("MQTT断开连接");systemState->updateMqttType(0);}
        });
        //连接mqtt
        m_client->connectToHost();
        connect(m_client,&QMqttClient::messageReceived,this,[=](const QByteArray&message){
            //            qDebug()<<message<<endl;
            QJsonDocument doc = QJsonDocument::fromJson(message);
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                double data1[5],data2[7]={};
                data1[0]=obj["Temp"].toString().toDouble();
                data1[1]=obj["Humi"].toString().toDouble();
                data1[2]=obj["Pressure"].toString().toDouble();
                data1[3]=obj["UltravioletIntensity"].toString().toDouble();
                data1[4]=obj["LuminousIntensity"].toString().toDouble();
                data2[0]=obj["Oxygen"].toString().toDouble();
                data2[1]=obj["GAS"].toString().toDouble();
                data2[2]=obj["No2"].toString().toDouble();
                data2[3]=obj["Co"].toString().toDouble();
                data2[4]=obj["Nh3"].toString().toDouble();
                data2[5]=obj["Lon"].toString().toDouble();
                data2[6]=obj["Lat"].toString().toDouble();
                waterTop->setValue(data1);
                waterBottom->setValue(data2);
                carLng=obj["Lon"].toString();
                carLat=obj["Lat"].toString();
                if(isDrawTrajectoryFlag)
                {
                    ui->line_Map_lon->setText(carLng);
                    ui->line_Map_lat->setText(carLat);
                    ui->line_Map_title->setText("运动轨迹");
                    ui->btn_Map_addBZ->clicked();//添加运动轨迹标注点
                    mapDrawTrajectory();//更新轨迹
                }
            }
            else {
                qDebug()<<"数据解析失败"<<endl;
            }
        });
    }
    //------------------------------------------- TCP --------------------------------------------------------
    {
        m_tcp=new QTcpSocket;
        m_tcp->connectToHost("127.0.0.1",quint16(11224));
        // 检测是否和服务器是否连接成功了
        connect(m_tcp, &QTcpSocket::connected, this, [=]()
        {
            log->appendLog("TCP连接成功");
            systemState->updateTcpType(1);
        });
        // 检测服务器是否回复了数据
        connect(m_tcp, &QTcpSocket::readyRead, [=]()
        {
            // 接收服务器发送的数据
            QByteArray recvMsg = m_tcp->readAll();
            QString msg=QString::fromUtf8(recvMsg).remove("\"");
            handleAppletMsg(msg);
        });
        // 检测服务器是否和客户端断开了连接
        connect(m_tcp, &QTcpSocket::disconnected, this, [=]()
        {
            log->appendLog("TCP断开了连接");
            systemState->updateTcpType(0);
        });
    }
    //-------------------------------------------Video---------------------------------------------------------
    {
        m_videoThread = new VideoThread(this);//初始化视频线程对象
        connect(m_videoThread, &VideoThread::frameReady, this, &MainWindow::onFrameReady);//连接图像传输信号槽
        ui->Video_show->setScaledContents(true);//图像自适应大小
    }
    //------------------------------------------地图显示---------------------------------------------------------
    {
        mainMapView = new QWebEngineView(this);//初始化
        mainMapView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);// 设置 QWidget 控件的大小策略为自适应大小
        mybridge = new MapBridge();        //初始化通道对象用于与JS交互
        layout = new QStackedLayout(ui->Map);//#1、引入布局，用于存放QWebengineView； 2、指定的父项ui->frame是在ui界面引入了一个QFrame,用于加载QWebengineView。
        ui->Map->setLayout(layout);    //设置frame的布局为layout。
        layout->addWidget(mainMapView);
        QWebChannel *channel = new QWebChannel(mainMapView->page()); // 将通道对象与页面关联
        channel->registerObject(QString("MainWindow"), mybridge); //  QWebChannel对Widget类，注册一个MainWindow的通信介质
        mainMapView->page()->setWebChannel(channel);
        // 加载Web页面
        mainMapView->page()->load(QUrl("qrc:/baidu.html"));
        //关联信号和槽函数
        connect(mybridge, &MapBridge::send_click_signal, this, &MainWindow::recv_click_slot);
        connect(mybridge, &MapBridge::send_mousemove_signal, this, &MainWindow::recv_mousemove_slot);
        //-----------------------设置标注列表----------------------------------------------
        {
            QString horQss="QTableView QHeaderView{background-color:rgb(240,207,214);}"
                           "QTableView QHeaderView::section{color: rgb(122, 175, 227);"
                           "border-right: 1px solid rgb(240,207,214);border-left: 1px solid rgb(240,207,214);"
                           "font-weight:bold;height: 35px;}";
            QString scrollBarStyle = "QScrollBar { background: rgb(240,207,214); width: 12px; }"
                                     "QScrollBar::handle { background: rgba(136, 193, 150,0.5); border: 0px solid rgb(255, 255, 255); border-radius: 5px; }"
                                     "QScrollBar::handle:hover { background: rgba(136, 193, 150,0.7); }"
                                     "QScrollBar::sub-line, QScrollBar::add-line { background: rgb(210,225,243); }"
                                     "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background: rgb(210,225,243); }";
            ui->tableWidget_BZList->horizontalHeader()->setVisible(true);//显示水平头
            ui->tableWidget_BZList->verticalHeader()->setVisible(false);//显示垂直头
            ui->tableWidget_BZList->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidget_BZList->setEditTriggers(QAbstractItemView::NoEditTriggers);//用户不能直接在表格或列表的单元格中进行编辑
            ui->tableWidget_BZList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);// 设置表头自适应列宽
            ui->tableWidget_BZList->horizontalHeader()->setStyleSheet(horQss);
            ui->tableWidget_BZList->verticalScrollBar()->setStyleSheet(scrollBarStyle);
            ui->tableWidget_BZList->horizontalScrollBar()->setStyleSheet(scrollBarStyle);

            //管理200个标注名,初始化
            for (int i = 1; i <= 200; i++)
            {
                BZName.insert(i, QString());
            }
        }
        ui->btn_Map_addBZ->clicked();
    }
    //------------------------------------------热感显示---------------------------------------------------------
    {
        //        this->ui->Map_2->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}"));//注册组件ID
        //        this->ui->Map_2->setFocusPolicy(Qt::StrongFocus);//设置控件接收键盘焦点的方式：鼠标单击、Tab键
        //        this->ui->Map_2->setProperty("DisplayAlerts",false);//不显示警告信息
        //        this->ui->Map_2->setProperty("DisplayScrollBars",true);//不显示滚动条
        //        this->ui->Map_2->setProperty("Silent", true);//屏蔽脚本错误
        //        QString webstr=QString("http://192.168.67.202/");//设置要打开的网页
        //        this->ui->Map_2->dynamicCall("Navigate(const QString&)",webstr);//显示网页
        QNetworkProxyFactory::setUseSystemConfiguration(false);   // 设置使用自定义代理配置
        m_webView = new QWebEngineView(this);
        QStackedLayout *layout =new QStackedLayout(ui->Map_2);//#1、引入布局，用于存放QWebengineView； 2、指定的父项ui->frame是在ui界面引入了一个QFrame,用于加载QWebengineView。
        ui->frame->setLayout(layout);//设置frame的布局为layout。
        layout->addWidget(m_webView);
        m_webView->load(QUrl("http://192.168.177.194/"));
        connect(ui->btn_Map2_ReLoad,&QPushButton::clicked,this,[=]{
            m_webView->reload();
        });
    }
    //------------------------------------------页面初始化---------------------------------------------------------
    {
        RealTimePageInit();//RealTimePage页面初始化---3D模型
        OperationControlPageInit();//页面初始化---操作控制
    }
    //------------------------------------------- 控制台 -------------------------------------------------------
}
//---------- 窗口移动事件(重写) ----------
//鼠标按下事件
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bIsPressed = true;
        m_lastPt = event->globalPos() - this->pos();
        event->accept();
    }
}
//鼠标释放事件
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_bIsPressed = false;	//鼠标按下标志位还原置为false
    Q_UNUSED(event);
}
//鼠标移动事件
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bIsPressed && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_lastPt);
        event->accept();
    }
}
//关闭窗口事件
void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"窗口关闭"<<endl;
    menu->deleteLater();
    waterTop->deleteLater();
    waterBottom->deleteLater();
    systemState->deleteLater();
    log->deleteLater();
    m_videoThread->deleteLater();
    m_client->deleteLater();
    Q_UNUSED(event);
}
//DockWidget窗口初始化
void MainWindow::DockWidgetInit()
{
    //--------------------------- Dock_Top-菜单栏 ----------------------------
    {
        QDockWidget *Dock_Top=new QDockWidget;
        //隐藏dw_top标题栏
        QWidget* pTitleWidget = Dock_Top ->titleBarWidget();
        QWidget* pWidget = new QWidget;
        Dock_Top ->setTitleBarWidget(pWidget );
        delete pTitleWidget ;
        //实例化菜单界面
        menu=new Menu(this);
        Dock_Top->setWidget(menu);
        Dock_Top->setFeatures(QDockWidget::NoDockWidgetFeatures);//设置不可移动、停靠
        addDockWidget(Qt::TopDockWidgetArea, Dock_Top);//设置在顶部
        //菜单界面信号的绑定
        connect(menu,&Menu::showMin,this,[=]{showMinimized();});//最小化
        connect(menu,&Menu::showMax,this,[=]{
            showMaximized();
            QString cmd = QString("showMax();");
            mainMapView->page()->runJavaScript(cmd);
            qDebug()<<"放大";
        });//最大化
        connect(menu,&Menu::showNormal,this,[=]{showNormal();
            QString cmd = QString("showNone();");
            mainMapView->page()->runJavaScript(cmd);
            qDebug()<<"还原";
        });//还原
        connect(menu,&Menu::showClose,this,&MainWindow::close);//关闭
        connect(menu,&Menu::toRealTimePage,this,[=]{ui->stackedWidget->setCurrentIndex(0);});
        connect(menu,&Menu::toRecordQueryPage,this,[=]{ui->stackedWidget->setCurrentIndex(1);});
        connect(menu,&Menu::toControlPage,this,[=]{ui->stackedWidget->setCurrentIndex(2);});
        connect(menu,&Menu::toFaultDetectPage,this,[=]{ui->stackedWidget->setCurrentIndex(3);});
        connect(menu,&Menu::toHotVideoPage,this,[=]{ui->stackedWidget->setCurrentIndex(4);});
    }
    QString LabelStyleSheet="background:rgb(36,61,91);color: rgb(122, 175, 227);font:14pt '楷体';font-weight:bold";
    QString DockStyleSheet="border:1px solid rgb(19,39,67);";
    //--------------------------- Dock_Left-数据集① ---------------------------
    {
        QDockWidget *Dock_Left_Top=new QDockWidget("数据集①");
        Dock_Left_Top->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Left_Top_Title=new QLabel;
        Dock_Left_Top_Title->setStyleSheet(LabelStyleSheet);
        Dock_Left_Top_Title->setText("数据集①");
        Dock_Left_Top_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Left_Top->setTitleBarWidget(Dock_Left_Top_Title);
        waterTop=new WaterTop;
        Dock_Left_Top->setWidget(waterTop);
        addDockWidget(Qt::LeftDockWidgetArea, Dock_Left_Top,Qt::Orientation::Vertical);//设置在左侧，第三个参数表示DockWidget的方向是垂直的
    }

    //--------------------------- Dock_Left-数据集② ---------------------------
    {
        QDockWidget *Dock_Left_Bottom=new QDockWidget("数据集②");
        Dock_Left_Bottom->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Left_Bottom_Title=new QLabel;
        Dock_Left_Bottom_Title->setStyleSheet(LabelStyleSheet);
        Dock_Left_Bottom_Title->setText("数据集②");
        Dock_Left_Bottom_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Left_Bottom->setTitleBarWidget(Dock_Left_Bottom_Title);
        waterBottom=new WaterBottom;
        Dock_Left_Bottom->setWidget(waterBottom);
        addDockWidget(Qt::LeftDockWidgetArea, Dock_Left_Bottom,Qt::Orientation::Vertical);//设置在左侧，第三个参数表示DockWidget的方向是垂直的
    }

    //--------------------------- Dock_Right-系统状态 ---------------------------
    {
        QDockWidget *Dock_Right_Top=new QDockWidget("系统状态");
        Dock_Right_Top->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Right_Top_Title=new QLabel;
        Dock_Right_Top_Title->setStyleSheet(LabelStyleSheet);
        Dock_Right_Top_Title->setText("系统状态");
        Dock_Right_Top_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Right_Top->setTitleBarWidget(Dock_Right_Top_Title);
        systemState=new SystemState;
        Dock_Right_Top->setWidget(systemState);
        addDockWidget(Qt::RightDockWidgetArea, Dock_Right_Top,Qt::Orientation::Vertical);//设置在右，第三个参数表示DockWidget的方向是垂直的
    }

    //--------------------------- Dock_Left-日志 ---------------------------
    {
        QDockWidget *Dock_Right_Bottom=new QDockWidget("日志");
        Dock_Right_Bottom->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Right_Bottom_Title=new QLabel;
        Dock_Right_Bottom_Title->setStyleSheet(LabelStyleSheet);
        Dock_Right_Bottom_Title->setText("日志");
        Dock_Right_Bottom_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Right_Bottom->setTitleBarWidget(Dock_Right_Bottom_Title);
        log=new Log;
        Dock_Right_Bottom->setWidget(log);
        addDockWidget(Qt::RightDockWidgetArea, Dock_Right_Bottom,Qt::Orientation::Vertical);//设置在右，第三个参数表示DockWidget的方向是垂直的
    }
}
//主页面初始化
void MainWindow::RealTimePageInit()
{
    player = new QMediaPlayer;
    videoWidget = new QVideoWidget;
    player->setVideoOutput(videoWidget);
    player->setMedia(QUrl::fromLocalFile("../SBOT.avi"));
    videoWidget->show();
    videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);// 设置视频比例自适应
    // 连接视频状态改变的信号与槽
    connect(player, &QMediaPlayer::stateChanged, this, &MainWindow::onVideoStateChanged);
    ui->ModelShow->addWidget(videoWidget);
    player->play();
}
//操作控制页面初始化
void MainWindow::OperationControlPageInit()
{
    ui->stackedWidgetControl->setCurrentIndex(0);
    QString defaultStyleSheet = "QToolButton{color: rgb(122, 175, 227);font: 10pt '楷体';border-radius:6px;}\
            QToolButton:hover{background-color: rgb(19, 48, 80);background-color: rgb(3, 57, 103);font: 10pt '楷体';border-radius:6px;border:2px solid rgb(13,39,67);}";
            QString pressedStyleSheet = "QToolButton{background-color: rgb(19, 48, 80);background-color: rgb(3, 57, 103);font: 10pt '楷体';border-radius:6px;border:2px solid rgb(13,39,67);}";
    //-----------------切换按钮-----------------------------------
    connect(ui->btn_Run,&QPushButton::clicked,this,[=]{
        ui->stackedWidgetControl->setCurrentIndex(0);
        ui->btn_Run->setStyleSheet(pressedStyleSheet);
        ui->btn_JXB->setStyleSheet(defaultStyleSheet);
        ui->btn_YY->setStyleSheet(defaultStyleSheet);
        ui->btn_XRQS->setStyleSheet(defaultStyleSheet);
    });
    connect(ui->btn_JXB,&QPushButton::clicked,this,[=]{
        ui->stackedWidgetControl->setCurrentIndex(1);
        ui->btn_Run->setStyleSheet(defaultStyleSheet);
        ui->btn_JXB->setStyleSheet(pressedStyleSheet);
        ui->btn_YY->setStyleSheet(defaultStyleSheet);
        ui->btn_XRQS->setStyleSheet(defaultStyleSheet);
    });
    connect(ui->btn_YY,&QPushButton::clicked,this,[=]{
        ui->stackedWidgetControl->setCurrentIndex(2);
        ui->btn_Run->setStyleSheet(defaultStyleSheet);
        ui->btn_JXB->setStyleSheet(defaultStyleSheet);
        ui->btn_YY->setStyleSheet(pressedStyleSheet);
        ui->btn_XRQS->setStyleSheet(defaultStyleSheet);
    });
    connect(ui->btn_XRQS,&QPushButton::clicked,this,[=]{
        ui->stackedWidgetControl->setCurrentIndex(3);
        ui->btn_Run->setStyleSheet(defaultStyleSheet);
        ui->btn_JXB->setStyleSheet(defaultStyleSheet);
        ui->btn_YY->setStyleSheet(defaultStyleSheet);
        ui->btn_XRQS->setStyleSheet(pressedStyleSheet);
    });
    //----------------运动控制按钮---------------------------------
    //按钮信号槽
    {
        /*----------------------- 运动控制 -----------------------*/
        //前进
        connect(ui->btn_Run_up,&QPushButton::clicked,this,[=]{
            TankZL=1;
            TankTimer->start(800);
            log->appendLog("前进指令");
        });
        //后退
        connect(ui->btn_Run_down,&QPushButton::clicked,this,[=]{
            TankZL=2;
            TankTimer->start(800);
            log->appendLog("后退指令");
        });
        //左转
        connect(ui->btn_Run_left,&QPushButton::clicked,this,[=]{
            TankZL=3;
            TankTimer->start(800);
            log->appendLog("左转指令");
        });
        //右转
        connect(ui->btn_Run_right,&QPushButton::clicked,this,[=]{
            TankZL=4;
            TankTimer->start(800);
            log->appendLog("右转指令");
        });
        //停止
        connect(ui->btn_Run_stop,&QPushButton::clicked,this,[=]{
            TankZL=5;
            TankTimer->start(800);
            log->appendLog("停止指令");
        });
    }
    //----------------机械臂控制按钮--------------------------------
    {
        connect(ui->JDT_JXB_J0,&QSlider::valueChanged,this,[=](int value){
            ui->JDT_JXB_J0_lbl->setText(QString("%1").arg(value));
        });
        connect(ui->JDT_JXB_J0,&QSlider::sliderReleased,this,[=]{
            int value=ui->JDT_JXB_J0_lbl->text().toInt();
            QJsonObject obj;
            obj.insert("0",1);
            obj.insert("1",value);
            QJsonDocument doc(obj);
            QByteArray msg= doc.toJson();
            m_client->publish(QString("Arm"),msg);
            QString jxbmsg;
            if(value>500)
                jxbmsg=QString("底座左转 %1 ").arg(value-500);
            else if (value==500)
                jxbmsg=QString("底座复位");
            else
                jxbmsg=QString("底座右转 %1 ").arg(500-value);
            log->appendLog(jxbmsg);
        });
        connect(ui->JDT_JXB_J1,&QSlider::valueChanged,this,[=](int value){
            ui->JDT_JXB_J1_lbl->setText(QString("%1").arg(value));
        });
        connect(ui->JDT_JXB_J1,&QSlider::sliderReleased,this,[=]{
            int value=ui->JDT_JXB_J1_lbl->text().toInt();
            QJsonObject obj;
            obj.insert("0",2);
            obj.insert("1",value);
            QJsonDocument doc(obj);
            QByteArray msg= doc.toJson();
            m_client->publish(QString("Arm"),msg);
            QString jxbmsg;
            if(value>500)
                jxbmsg=QString("后臂向下 %1 ").arg(value-500);
            else if (value==500)
                jxbmsg=QString("后臂复位");
            else
                jxbmsg=QString("后臂向上 %1 ").arg(500-value);
            log->appendLog(jxbmsg);
        });
        connect(ui->JDT_JXB_J2,&QSlider::valueChanged,this,[=](int value){
            ui->JDT_JXB_J2_lbl->setText(QString("%1").arg(value));
        });
        connect(ui->JDT_JXB_J2,&QSlider::sliderReleased,this,[=]{
            int value=ui->JDT_JXB_J2_lbl->text().toInt();
            QJsonObject obj;
            obj.insert("0",3);
            obj.insert("1",value);
            QJsonDocument doc(obj);
            QByteArray msg= doc.toJson();
            m_client->publish(QString("Arm"),msg);
            QString jxbmsg;
            if(value>500)
                jxbmsg=QString("中束向下 %1 ").arg(value-500);
            else if (value==500)
                jxbmsg=QString("中束复位");
            else
                jxbmsg=QString("中束向上 %1 ").arg(500-value);
            log->appendLog(jxbmsg);
        });
        connect(ui->JDT_JXB_J3,&QSlider::valueChanged,this,[=](int value){
            ui->JDT_JXB_J3_lbl->setText(QString("%1").arg(value));
        });
        connect(ui->JDT_JXB_J3,&QSlider::sliderReleased,this,[=]{
            int value=ui->JDT_JXB_J3_lbl->text().toInt();
            QJsonObject obj;
            obj.insert("0",4);
            obj.insert("1",value);
            QJsonDocument doc(obj);
            QByteArray msg= doc.toJson();
            m_client->publish(QString("Arm"),msg);
            QString jxbmsg;
            if(value>500)
                jxbmsg=QString("前臂向下 %1 ").arg(value-500);
            else if (value==500)
                jxbmsg=QString("前臂复位");
            else
                jxbmsg=QString("前臂向上 %1 ").arg(500-value);
            log->appendLog(jxbmsg);
        });
        connect(ui->JDT_JXB_J4,&QSlider::valueChanged,this,[=](int value){
            ui->JDT_JXB_J4_lbl->setText(QString("%1").arg(value));
        });
        connect(ui->JDT_JXB_J4,&QSlider::sliderReleased,this,[=]{
            int value=ui->JDT_JXB_J4_lbl->text().toInt();
            QJsonObject obj;
            obj.insert("0",5);
            obj.insert("1",value);
            QJsonDocument doc(obj);
            QByteArray msg= doc.toJson();
            m_client->publish(QString("Arm"),msg);
            QString jxbmsg;
            if(value>500)
                jxbmsg=QString("云台左转 %1 ").arg(value-500);
            else if (value==500)
                jxbmsg=QString("云台复位");
            else
                jxbmsg=QString("云台右转 %1 ").arg(500-value);
            log->appendLog(jxbmsg);
        });
    }
    connect(ui->btn_JXBKZ_FW,&QPushButton::clicked,this,[=]{
        QJsonObject obj;
        obj.insert("0",10);
        QJsonDocument doc(obj);
        QByteArray msg= doc.toJson();
        m_client->publish(QString("Arm"),msg);
        log->appendLog("机械臂复位");
        ui->JDT_JXB_J0->setValue(500);
        ui->JDT_JXB_J0_lbl->setNum(500);
        ui->JDT_JXB_J1->setValue(500);
        ui->JDT_JXB_J1_lbl->setNum(500);
        ui->JDT_JXB_J2->setValue(500);
        ui->JDT_JXB_J2_lbl->setNum(500);
        ui->JDT_JXB_J3->setValue(500);
        ui->JDT_JXB_J3_lbl->setNum(500);
        ui->JDT_JXB_J4->setValue(500);
        ui->JDT_JXB_J4_lbl->setNum(500);
    });
    //----------------语音控制按钮---------------------------------
    {
        connect(ui->btn_YY1,&QPushButton::clicked,this,[=]{
            TankZL=6;
            TankTimer->start(800);
            log->appendLog("语音1指令");
        });
        connect(ui->btn_YY2,&QPushButton::clicked,this,[=]{
            TankZL=7;
            TankTimer->start(800);
            log->appendLog("语音2指令");
        });
        connect(ui->btn_YY3,&QPushButton::clicked,this,[=]{
            TankZL=8;
            TankTimer->start(800);
            log->appendLog("语音3指令");
        });
        connect(ui->btn_YY4,&QPushButton::clicked,this,[=]{
            TankZL=9;
            TankTimer->start(800);
            log->appendLog("语音4指令");
        });
        connect(ui->btn_YY5,&QPushButton::clicked,this,[=]{
            TankZL=10;
            TankTimer->start(800);
            log->appendLog("语音5指令");
        });
        connect(ui->btn_YY6,&QPushButton::clicked,this,[=]{
            TankZL=11;
            TankTimer->start(800);
            log->appendLog("语音6指令");
        });
        connect(ui->btn_YY7,&QPushButton::clicked,this,[=]{
            TankZL=12;
            TankTimer->start(800);
            log->appendLog("语音停止指令");
        });
    }
    //----------------寻人启示------------------------------------
    {
        connect(ui->btn_XRQS_Upload,&QPushButton::clicked,this,[=]{
            QString ptrPath = QFileDialog::getOpenFileName(nullptr, tr("选择照片"), "", tr("寻人照片 (*.jpg *.png *.JPG *.PNG)"));
            ui->XRQS_ptr->setScaledContents(true);  //图像自适应大小
            ui->XRQS_ptr->setPixmap(QPixmap(ptrPath));
            log->appendLog("上传了寻人启示照片！");
        });
    }
}
//处理来自小程序中间件的消息
void MainWindow::handleAppletMsg(QString Msg)
{
    if(Msg.split(',').at(0)=="运动控制")
    {
        if(Msg.split(',').at(1)=="前进")
            ui->btn_Run_up->clicked();
        if(Msg.split(',').at(1)=="左转")
            ui->btn_Run_left->clicked();
        if(Msg.split(',').at(1)=="暂停")
            ui->btn_Run_stop->clicked();
        if(Msg.split(',').at(1)=="右转")
            ui->btn_Run_right->clicked();
        if(Msg.split(',').at(1)=="后退")
            ui->btn_Run_down->clicked();
    }

    if(Msg.split(',').at(0)=="机械臂控制")
    {
        if(Msg.split(',').at(2)=="复位")
            ui->btn_JXBKZ_FW->clicked();
        if(Msg.split(',').at(1)=="进度条")
        {
            if(Msg.split(',').at(2)=="J0")
            {
                ui->JDT_JXB_J0->valueChanged(Msg.split(',').at(3).toInt());
                ui->JDT_JXB_J0->sliderReleased();
            }
            if(Msg.split(',').at(2)=="J1")
            {
                ui->JDT_JXB_J1->valueChanged(Msg.split(',').at(3).toInt());
                ui->JDT_JXB_J1->sliderReleased();
            }
            if(Msg.split(',').at(2)=="J2")
            {
                ui->JDT_JXB_J2->valueChanged(Msg.split(',').at(3).toInt());
                ui->JDT_JXB_J2->sliderReleased();
            }
            if(Msg.split(',').at(2)=="J3")
            {
                ui->JDT_JXB_J3->valueChanged(Msg.split(',').at(3).toInt());
                ui->JDT_JXB_J3->sliderReleased();
            }
            if(Msg.split(',').at(2)=="J4")
            {
                ui->JDT_JXB_J4->valueChanged(Msg.split(',').at(3).toInt());
                ui->JDT_JXB_J4->sliderReleased();
            }
        }
    }
}
//接收图像-槽函数
void MainWindow::onFrameReady(QImage frame)
{
    // 在UI界面上显示视频的每一帧图像
    ui->Video_show->setPixmap(QPixmap::fromImage(frame));
}
//视频播放按钮
void MainWindow::on_btn_video_start_clicked()
{
    if(ui->btn_video_start->text()=="播放")
    {
        m_videoThread->setUrl(ui->video_url->text());
        // m_videoThread->start();//线程启动开始工作
        m_videoThread->start_video();
        ui->btn_video_start->setText("暂停");
    }
    else {
        m_videoThread->stop();
        ui->btn_video_start->setText("播放");
    }
}
//重复播放视频
void MainWindow::onVideoStateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState) {
        // 当视频播放结束时，将其设置到开头重新播放
        player->setPosition(0);
        player->play();
    }
}
//--------------------Map--------------------
//接收点击信号的槽函数
void MainWindow::recv_click_slot(QString lon, QString lat)
{
    qDebug()<<"点击的lon:"<<lon<<"lat:"<<lat;
    ui->line_Map_lon->setText(lon);
    ui->line_Map_lat->setText(lat);
}
//接收移动信号的槽函数
void MainWindow::recv_mousemove_slot(QString str)
{
    Q_UNUSED(str);
    ui->line_Map_lon->setText(str.split(",").at(0));
    ui->line_Map_lat->setText(str.split(",").at(1));
    //    ui->lbl_Map_zoom->setText("地标等级:"+str.split(",").at(2));
}
//btn-Map-添加标注
void MainWindow::on_btn_Map_addBZ_clicked()
{
    if(ui->line_Map_lon->text()==nullptr||ui->line_Map_lat->text()==nullptr)
    {
        QMessageBox::warning(this,"提示","没有获取到要添加标注的经纬度!");
        return ;
    }
    QString title=ui->line_Map_title->text();
    if(title==nullptr)
    {
        title=QString("null");
    }
    QByteArray encodedText = title.toUtf8();
    QString getBzName=BiaoZhuName();//为新标注分配标注名称
    QByteArray HeadText=getBzName.toUtf8();
    QString cmd = QString("myMarker(%1,%2,'%3','%4')").arg(ui->line_Map_lon->text()).arg(ui->line_Map_lat->text()).arg(encodedText.constData()).arg(HeadText.constData());
    qDebug()<<cmd;
    mainMapView->page()->runJavaScript(cmd);//执行js的myMarker函数

    BiaoZhuWidget *bzWidget=new BiaoZhuWidget;
    bzWidget->setLabelText(title);
    bzWidget->setCheckBoxText(getBzName);
    bzWidget->setLng(ui->line_Map_lon->text());
    bzWidget->setLat(ui->line_Map_lat->text());
    //绑定checkBox的选中-标注
    connect(bzWidget,&BiaoZhuWidget::send_Lng_Lat,this,[=](QString lng,QString lat,bool checked){
        if(checked)
        {
            QString cmd = QString("changeMarkerColor_blue(%1,%2)").arg(lng).arg(lat);
            qDebug()<<cmd;
            mainMapView->page()->runJavaScript(cmd);//执行js的removeMarkerr函数
        }
        else {
            QString cmd = QString("changeMarkerColor_red(%1,%2)").arg(lng).arg(lat);
            qDebug()<<cmd;
            mainMapView->page()->runJavaScript(cmd);//执行js的removeMarkerr函数
        }
    });
    //将标注对象存入管理容器
    BzVector.append(bzWidget);
    //将标注添加到表单上
    int nowRow=ui->tableWidget_BZList->rowCount();
    ui->tableWidget_BZList->insertRow(nowRow);
    ui->tableWidget_BZList->setRowHeight(nowRow,55);//设置行高度
    ui->tableWidget_BZList->setCellWidget(nowRow,0,bzWidget);
    log->appendLog("添加了一个标注");
    //清除备注框内容
    ui->line_Map_title->clear();
    //UI左侧标注数实时更新
    //    ui->lbl_bzNum->setText(ui->lbl_bzNum->text().split(' ').at(0)+QString(" %1").arg(BzVector.size()));
}
//btn-Map-移除标注
void MainWindow::on_btn_Map_moveBZ_clicked()
{
    if(ui->line_Map_lon->text()==nullptr||ui->line_Map_lat->text()==nullptr)
    {
        QMessageBox::warning(this,"提示","没有获取到要删除的标注的经纬度!");
        return ;
    }

    for (int i=0;i<BzVector.size();++i)
    {
        if(BzVector[i]->getCheckBox_checked())
        {
            qDebug()<<"标注名称:"<<BzVector[i]->checkBoxText();
            int key=BzVector[i]->checkBoxText().split("标注").at(1).toInt();
            qDebug()<<"移除的key:"<<key;
            BZName[key]=QString("");

            QString cmd = QString("removeMarker(%1,%2)").arg(BzVector[i]->getLng()).arg(BzVector[i]->getLat());
            qDebug()<<"执行js的removeMarkerr函数:"<<cmd;
            mainMapView->page()->runJavaScript(cmd);//执行js的removeMarkerr函数
            BzVector.erase(BzVector.begin()+i);//从自定义管理控件中移除
            ui->tableWidget_BZList->removeRow(i);//从tableView上移除--刷新界面
            log->appendLog("移除了一个标注");
            //UI左侧标注数实时更新
            //            ui->lbl_bzNum->setText(ui->lbl_bzNum->text().split(' ').at(0)+QString(" %1").arg(BzVector.size()));
        }

    }

}
//为标注分配名称
QString MainWindow::BiaoZhuName()
{
    for(auto it=BZName.begin();it!=BZName.end();++it)
    {
        if(it.value()=="")
        {
            QString value=QString("标注%1").arg(it.key());
            BZName[it.key()]=value;
            qDebug()<<"分配的key:"<<it.key()<<"value:"<<BZName[it.key()];
            return value;
        }
    }
    return "";
}
//btn-Map-路径规划
void MainWindow::on_btn_Map_LJGH_clicked()
{
    //    mainMapView->page()->load(QUrl("qrc:/baidu.html"));//
    QString lng1,lat1,lng2,lat2;
    int a=0,b=0;
    for(int i=0,j=1;i<BzVector.size();++i)
    {
        if(BzVector[i]->getCheckBox_checked()&&j==1)
        {
            lng1=BzVector[i]->getLng();
            lat1=BzVector[i]->getLat();
            ++j;
            a=i;
        }
        else if(BzVector[i]->getCheckBox_checked()&&j==2)
        {
            lng2=BzVector[i]->getLng();
            lat2=BzVector[i]->getLat();
            b=i;
            break;
        }

    }
    QString cmd = QString("getsearch(%1,%2,%3,%4)").arg(lng1).arg(lat1).arg(lng2).arg(lat2);
    qDebug()<<"执行js的getsearch函数:"<<cmd;
    mainMapView->page()->runJavaScript(cmd);//执行js的getsearch函数
    log->appendLog(QString("路径规划(%1-%2)").arg(BzVector[a]->labelText()).arg(BzVector[b]->labelText()));
}
//btn-Map-清除地图上所有覆盖物
void MainWindow::on_btn_Map_ClearAllBz_clicked()
{
    for(int i=1;i<=BZName.size();++i)
    {
        BZName[i]="";
    }

    BzVector.clear();
    int rowCount = ui->tableWidget_BZList->rowCount();
    for (int i = rowCount - 1; i >= 0; --i) {
        ui->tableWidget_BZList->removeRow(i);
    }
    QString cmd = QString("map.clearOverlays();");//清除地图上所有覆盖物
    qDebug()<<"执行执行-清除地图上所有覆盖物:"<<cmd;
    mainMapView->page()->runJavaScript(cmd);//执行-清除地图上所有覆盖物
    log->appendLog("清除了所有覆盖物");
}
//显示运动轨迹
void MainWindow::mapDrawTrajectory()
{
    QString script = "drawTrajectory([";
    for(int i=0;i<BzVector.size();++i)
    {
        script += QString("{lng: %1,lat: %2}").arg(BzVector[i]->getLng()).arg(BzVector[i]->getLat());
        if (i != BzVector.size() - 1) {
            script += ", ";
        }
    }
    script += "]);";
    qDebug()<<script;
    mainMapView->page()->runJavaScript(script);
}
//btn-Map-开启/关闭轨迹
void MainWindow::on_btn_Map_YDGJ_clicked()
{
    if(!isDrawTrajectoryFlag)
    {
        on_btn_Map_ClearAllBz_clicked();//清除地图上所有覆盖物
        ui->btn_Map_YDGJ->setText("关闭轨迹");
    }
    else
    {
        ui->btn_Map_YDGJ->setText("开启轨迹");
    }
    isDrawTrajectoryFlag=!isDrawTrajectoryFlag;
}
