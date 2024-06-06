#include "log.h"
#include "ui_log.h"
#include <QDateTime>
#include<QDebug>
#include<QScrollBar>
#include<QTableWidget>
Log::Log(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Log)
{
    ui->setupUi(this);
    ui->tableWidget->viewport()->installEventFilter(this);
    ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    QString headerStyle = "QHeaderView::section { background: rgb(3, 57, 103); color: rgb(122, 175, 227); border: 1px solid rgb(14, 26, 50); }";
    ui->tableWidget->horizontalHeader()->setStyleSheet(headerStyle);
    ui->tableWidget->horizontalHeader()->setFont(QFont("楷体", 12));
    ui->tableWidget->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget->setRowCount(10);
    // 设置第一列宽度为固定值，其他列自动适应
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidget->setColumnWidth(0, 80); // 固定第一列宽度为 80
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // 其他列自动调整

    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableWidget->setFont(QFont("楷体", 10));
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    QString tableStyle = "QTableWidget { color: rgb(122, 175, 227); selection-background-color: rgb(3, 57, 103); selection-color: rgb(122, 175, 227); background-color: rgb(14, 26, 50); gridline-color: rgb(19, 39, 67); }";
    QString itemStyle = "QTableWidgetItem:hover { background-color: rgb(19, 39, 67); }";
    ui->tableWidget->setStyleSheet(tableStyle + itemStyle);

    QString scrollBarStyle = "QScrollBar { background: rgb(14, 26, 50); width: 12px; }"
                             "QScrollBar::handle { background: rgb(19, 39, 67); border: 0px solid rgb(255, 255, 255); border-radius: 5px; }"
                             "QScrollBar::handle:hover { background: rgb(122, 175, 229); }"
                             "QScrollBar::sub-line, QScrollBar::add-line { background: rgb(19, 48, 80); }"
                             "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background: rgb(19, 48, 80); }";
    ui->tableWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->tableWidget->horizontalScrollBar()->setStyleSheet(scrollBarStyle);
    connect(ui->tableWidget, &QTableWidget::itemActivated, this, [=](QTableWidgetItem *item) {
        qDebug() << "双击了:" << item->text()<< endl;
    });

}

Log::~Log()
{
    delete ui;
}

void Log::appendLog(QString msg)
{
    //获取总行数
    int RowCount=ui->tableWidget->rowCount();
    qDebug()<<"总行数："<<RowCount<<endl;
    QString nowTime=QDateTime::currentDateTime().toString("hh:mm:ss");
    if(logRowCount>RowCount)
    {
        for (int i=0;i<RowCount-1;i++)
        {
            qDebug()<<"i:"<<i;
            ui->tableWidget->item(i,0)->setText(ui->tableWidget->item(i+1,0)->text());
            ui->tableWidget->item(i,1)->setText(ui->tableWidget->item(i+1,1)->text());
        }
        ui->tableWidget->item(RowCount-1,0)->setText(nowTime);
        ui->tableWidget->item(RowCount-1,1)->setText(msg);
    }
    else if (logRowCount<=RowCount) {
        ui->tableWidget->setItem(logRowCount-1, 0, new QTableWidgetItem(nowTime));
        ui->tableWidget->item(logRowCount-1, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(logRowCount-1, 1, new QTableWidgetItem(msg));
        ui->tableWidget->item(logRowCount-1, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        logRowCount++;
    }

}
