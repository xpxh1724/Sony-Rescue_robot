#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
using namespace cv::dnn;
using namespace cv;
class VideoThread : public QThread
{
    Q_OBJECT

public:
    explicit VideoThread(QObject *parent = nullptr);
    ~VideoThread();

    void run() ;
    void stop();
    void start_video();
    void setUrl(QString Url){
        url=Url;
    }

signals:
    void frameReady(QImage frame);
protected:

private:
    bool m_stop;
    QMutex m_mutex;
    QWaitCondition m_condition;
    QString url="0";

    size_t width = 300;
    size_t height = 300;
    float meanVal = 127.5;//均值
    float scaleFactor = 0.007843f;
    const char* classNames[300] = {"background",
                                   "aeroplane", "bicycle", "bird", "boat",
                                   "bottle", "bus", "car", "cat", "chair",
                                   "cow", "diningtable", "dog", "horse",
                                   "motorbike", "person", "pottedplant",
                                   "sheep", "sofa", "train", "tvmonitor" };
    //模型文件
    String modelFile = "../MobileNetSSD_deploy.caffemodel";
    //二进制描述文件
    String model_text_file = "../MobileNetSSD_deploy.prototxt";
    Net net;
};


