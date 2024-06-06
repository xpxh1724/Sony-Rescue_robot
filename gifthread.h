#ifndef GIFTHREAD_H
#define GIFTHREAD_H

#include <QObject>
#include <QThread>
#include <QMovie>
#include <QLabel>
#include <QDebug>

class GifThread:public QThread
{
    Q_OBJECT
public:
    GifThread(const QString gifpath);

    QString gifPath;
    QLabel *label;
    QMovie *gif;
    void run();
};

#endif // GIFTHREAD_H
