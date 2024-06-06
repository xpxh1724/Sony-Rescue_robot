#include "gifthread.h"

GifThread::GifThread(const QString gifpath)
{
    gifPath = gifpath;
     label = new QLabel;
}

void GifThread::run()
{
    gif = new QMovie(gifPath);
       label->setMovie(gif);
       gif->setScaledSize(QSize(400,300));
       gif->setSpeed(500);
       gif->start();
       while(1)
       {
   //        qDebug()<<currentThreadId();
   //        msleep(100);
           qApp->processEvents();
       }
}
