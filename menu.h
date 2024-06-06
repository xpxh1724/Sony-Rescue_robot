#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QTimer>
//---------菜单栏---------
namespace Ui {
class Menu;
}

class Menu : public QWidget
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu();
signals:
    //最小化窗口-信号
    void showMin();
    //最大化窗口-信号
    void showMax();
    //还原窗口-信号
    void showNormal();
    //关闭窗口-信号
    void showClose();
    //去实时数据页面-信号
    void toRealTimePage();
    //去记录查询页面-信号
    void toRecordQueryPage();
    //去热成像页面-信号
    void toHotVideoPage();
    //去控制管理页面-信号
    void toControlPage();
    //去故障检测页面-信号
    void toFaultDetectPage();
private:
    Ui::Menu *ui;
    QTimer *UpdataTime;
};

#endif // MENU_H
