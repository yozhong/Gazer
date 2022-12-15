#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), dataLock(lock)
{
    fpsCalculating = false;
    fps = 0.0;

    frameWidth = 0;
    frameHeight = 0;
    videoSavingStatus = STOPPED;
    savedVideoName = "";
    videoWriter = nullptr;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), dataLock(lock)
{
    fpsCalculating = false;
    fps = 0.0;

    frameWidth = 0;
    frameHeight = 0;
    videoSavingStatus = STOPPED;
    savedVideoName = "";
    videoWriter = nullptr;
}

CaptureThread::~CaptureThread() {}

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    cv::Mat tmpFrame;

    frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    while(running) {
        cap >> tmpFrame;
        if (tmpFrame.empty()) {
            break;
        }

        if(videoSavingStatus == STARTING) {
            startSavingVideo(tmpFrame);
        }
        if(videoSavingStatus == STARTED) {
            videoWriter->write(tmpFrame);
        }
        if(videoSavingStatus == STOPPING) {
            stopSavingVideo();
        }

        cvtColor(tmpFrame, tmpFrame, cv::COLOR_BGR2RGB);
        dataLock->lock();
        frame = tmpFrame;
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

void CaptureThread::startSavingVideo(cv::Mat &firstFrame)
{
    savedVideoName = Utilities::newSavedVideoName();
    QString cover = Utilities::getSavedVideoPath(savedVideoName, "jpg");
    //save the first frame of the video to an image
    cv::imwrite(cover.toStdString(), firstFrame);

    videoWriter = new cv::VideoWriter(
        Utilities::getSavedVideoPath(savedVideoName, "avi").toStdString(),
        cv::VideoWriter::fourcc('M','J','P','G'),
        fps ? fps : 30,
        cv::Size(frameWidth, frameHeight));

    videoSavingStatus = STARTED;
}

void CaptureThread::stopSavingVideo()
{
    videoSavingStatus = STOPPED;
    videoWriter->release();
    delete videoWriter;
    videoWriter = nullptr;
    emit videoSaved(savedVideoName);
}
