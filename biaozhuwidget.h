#ifndef BIAOZHUWIDGET_H
#define BIAOZHUWIDGET_H

#include <QWidget>
namespace Ui {
class BiaoZhuWidget;
}

class BiaoZhuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BiaoZhuWidget(QWidget *parent = nullptr);

    ~BiaoZhuWidget();
    QString labelText();
    QString checkBoxText();
    void setLabelText(const QString&text);
    void setCheckBoxText(const QString&text);
    QString getLng();
    QString getLat();
    void setLng(const QString&text);
    void setLat(const QString&text);
    bool getCheckBox_checked();
signals:
    void send_Lng_Lat(QString lng,QString lat,bool checked);
private slots:
    void on_checkBox_clicked(bool checked);

private:
    Ui::BiaoZhuWidget *ui;
    QString lng="0";
    QString lat="0";
};

#endif // BIAOZHUWIDGET_H
