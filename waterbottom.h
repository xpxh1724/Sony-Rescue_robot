#ifndef WATERBOTTOM_H
#define WATERBOTTOM_H

#include <QWidget>

namespace Ui {
class WaterBottom;
}

class WaterBottom : public QWidget
{
    Q_OBJECT

public:
    explicit WaterBottom(QWidget *parent = nullptr);
    ~WaterBottom();
    //通过值来判断对应范围数值的背景颜色：正常、提示、警告
    void ValueColorIsChanged(double nowValue, double zcValue1, double zcValue2, double infoValue1, double infoValue2,double infoValue3,double infoValue4, QWidget *widget);
public slots:
    //设置值
    void setValue(double *Data);
private:
    Ui::WaterBottom *ui;
};

#endif // WATERBOTTOM_H
