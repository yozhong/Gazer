#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QTime>

#include <opencv2/opencv.hpp>

#include "utilities.h"

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

    enum VideoSavingStatus {
        STARTING,
        STARTED,
        STOPPING,
        STOPPED
    };
    void setVideoSavingStatus(VideoSavingStatus status) {videoSavingStatus = status; };

    void setMotionDetectingStatus(bool status) {
        motionDetectingStatus = status;
        motionDetected = false;
        if(videoSavingStatus != STOPPED) videoSavingStatus = STOPPING;
    };

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *dataLock;
    cv::Mat frame;
    // FPS calculating
    bool fpsCalculating;
    float fps;
    // video saving
    int frameWidth;
    int frameHeight;
    VideoSavingStatus videoSavingStatus;
    QString savedVideoName;
    cv::VideoWriter *videoWriter;
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();
    // motion analysis
    void motionDetect(cv::Mat &frame);
    bool motionDetectingStatus;
    bool motionDetected;
    cv::Ptr<cv::BackgroundSubtractorMOG2> segmentor;
};

#endif // CAPTURE_THREAD_H
