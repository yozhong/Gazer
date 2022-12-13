#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H
#include <QString>
#include <QThread>
#include <QMutex>

#include <opencv2/opencv.hpp>

class CaptureThread : public QThread
{
    Q_OBJECT
    public:
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) { running = run; };

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *dataLock;
    cv::Mat frame;
};

#endif // CAPTURE_THREAD_H
