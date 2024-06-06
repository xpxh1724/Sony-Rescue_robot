#include "videothread.h"

VideoThread::VideoThread(QObject *parent) : QThread(parent), m_stop(false)
{
    // set up net
     net = readNetFromCaffe(model_text_file, modelFile);
}

VideoThread::~VideoThread()
{
    stop();
    wait();
}

void VideoThread::run()
{
    VideoCapture capture;
    if(url=="0")
    {  capture.open(0);}
    else{
    capture.open(url.toStdString()); // 替换为你的视频路径
        }
    if (capture.isOpened())
    {
        while (!m_stop)
        {
            cv::Mat frame;
            capture >> frame;
            cv::flip(frame,frame,1);
            if (!frame.empty())
            {
                //识别处理
                // 预测
                Mat inputblob = blobFromImage(frame, scaleFactor, Size(width, height), meanVal, false);
                net.setInput(inputblob, "data");
                Mat detection = net.forward("detection_out");

                // 绘制
                Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
                float confidence_threshold = 0.9;//自信区间，越小检测到的物体越多（>=0.25)
                for (int i = 0; i < detectionMat.rows; i++) {
                    float confidence = detectionMat.at<float>(i, 2);
                    if (confidence > confidence_threshold) {
                        size_t objIndex = (size_t)(detectionMat.at<float>(i, 1));
                        float tl_x = detectionMat.at<float>(i, 3) * frame.cols;
                        float tl_y = detectionMat.at<float>(i, 4) * frame.rows;
                        float br_x = detectionMat.at<float>(i, 5) * frame.cols;
                        float br_y = detectionMat.at<float>(i, 6) * frame.rows;
                        Rect object_box((int)tl_x, (int)tl_y, (int)(br_x - tl_x), (int)(br_y - tl_y));
                        rectangle(frame, object_box, Scalar(0, 0, 255), 2, 8, 0);
                        putText(frame, format("%s", classNames[objIndex]), Point(tl_x, tl_y), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 0, 0), 2);
                    }
                }
                // 将OpenCV的Mat转换为Qt的QImage
                QImage img(frame.data, frame.cols, frame.rows, static_cast<int>(frame.step), QImage::Format_RGB888);
                img = img.rgbSwapped();

                // 发送信号将每一帧图像传递给UI界面
                emit frameReady(img);
            }
        }
    }

    capture.release();
}

void VideoThread::stop()
{
        QMutexLocker locker(&m_mutex);
        m_stop = true;
        m_condition.wakeAll();
}

void VideoThread::start_video()
{
    QMutexLocker locker(&m_mutex);
    m_stop = false;
    locker.unlock();
    // 唤醒等待的条件
    m_condition.wakeAll();
    // 如果线程当前没有运行，则启动线程
    if (!isRunning())
        QThread::start();
}
