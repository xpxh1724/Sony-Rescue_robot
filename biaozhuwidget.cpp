#include "biaozhuwidget.h"
#include "ui_biaozhuwidget.h"
#include<QDebug>
BiaoZhuWidget::BiaoZhuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BiaoZhuWidget)
{
    ui->setupUi(this);
    ui->checkBox->setCheckable(true);
}

BiaoZhuWidget::~BiaoZhuWidget()
{
    delete ui;
}

QString BiaoZhuWidget::labelText()
{
    return ui->label->text();
}

QString BiaoZhuWidget::checkBoxText()
{
    return ui->checkBox->text();
}

void BiaoZhuWidget::setLabelText(const QString &text)
{
    ui->label->setText(text);
}

void BiaoZhuWidget::setCheckBoxText(const QString &text)
{
    ui->checkBox->setText(text);
}

QString BiaoZhuWidget::getLng()
{
    return lng;
}

QString BiaoZhuWidget::getLat()
{
    return lat;
}

void BiaoZhuWidget::setLng(const QString&text)
{
    lng=text;
}

void BiaoZhuWidget::setLat(const QString &text)
{
    lat=text;
}

bool BiaoZhuWidget::getCheckBox_checked()
{
    return ui->checkBox->isChecked();
}

void BiaoZhuWidget::on_checkBox_clicked(bool checked)
{
        emit this->send_Lng_Lat(lng,lat,checked);
}
