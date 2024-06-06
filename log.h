#ifndef LOG_H
#define LOG_H

#include <QWidget>

namespace Ui {
class Log;
}

class Log : public QWidget
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr);
    ~Log();
    void appendLog(QString msg);

private:
    Ui::Log *ui;
    int logRowCount=1;
};

#endif // LOG_H
