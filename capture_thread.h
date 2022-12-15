#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H
#include <QString>
#include <QThread>
#include <QMutex>
#include <QTime>

#include <opencv2/opencv.hpp>

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) { running = run; };
    void calculateFPS(cv::VideoCapture &cap);
    void startCalcFPS();

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *dataLock;
    cv::Mat frame;
    // FPS calculating
    bool fpsCalculating;
    float fps;
};

#endif // CAPTURE_THREAD_H
