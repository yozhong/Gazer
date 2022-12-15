#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), dataLock(lock)
{
    fpsCalculating = false;
    fps = 0.0;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), dataLock(lock)
{
    fpsCalculating = false;
    fps = 0.0;
}

CaptureThread::~CaptureThread() {}

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    cv::Mat tmp_frame;

    while(running) {
        cap >> tmp_frame;
        if (tmp_frame.empty()) {
            break;
        }
        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        dataLock->lock();
        frame = tmp_frame;
        dataLock->unlock();
        emit frameCaptured(&frame);

        if(fpsCalculating) {
            calculateFPS(cap);
        }
    }

    cap.release();
    running = false;
}

void CaptureThread::calculateFPS(cv::VideoCapture &cap)
{
    const int countToRead = 100;
    cv::Mat tmpFrame;
    QTime startTime = QTime::currentTime();

    for(int i = 0; i < countToRead; i++) {
        cap >> tmpFrame;
    }

    QTime endTime = QTime::currentTime();

    int elapsedMs = startTime.msecsTo(endTime);
    fps = countToRead / (elapsedMs / 1000.0);
    fpsCalculating = false;
    emit fpsChanged(fps);
}

void CaptureThread::startCalcFPS()
{
    fpsCalculating = true;
}
