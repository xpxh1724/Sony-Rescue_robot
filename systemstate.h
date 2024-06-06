#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

#include <QWidget>

namespace Ui {
class SystemState;
}

class SystemState : public QWidget
{
    Q_OBJECT

public:
    explicit SystemState(QWidget *parent = nullptr);
    ~SystemState();
public slots:
    void updateMqttType(int mqttType);
    void updateTcpType(int tcpType);
private:
    Ui::SystemState *ui;
};

#endif // SYSTEMSTATE_H
