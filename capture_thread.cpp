#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), dataLock(lock) {}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), dataLock(lock) {}

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
    }

    cap.release();
    running = false;
}
