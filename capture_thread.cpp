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

    motionDetectingStatus = false;
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

    motionDetectingStatus = false;
}

CaptureThread::~CaptureThread() {}

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    cv::Mat tmpFrame;

    segmentor = cv::createBackgroundSubtractorMOG2(500, 16, true);

    frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    while(running) {
        cap >> tmpFrame;
        if (tmpFrame.empty()) {
            break;
        }

        if(motionDetectingStatus) {
            motionDetect(tmpFrame);
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

void CaptureThread::motionDetect(cv::Mat &frame)
{
    cv::Mat fgmask;
    segmentor->apply(frame, fgmask);
    if (fgmask.empty()) {
        return;
    }
    //Remove noises and emphasize the objects
    cv::threshold(fgmask, fgmask, 25, 255, cv::THRESH_BINARY);

    int noiseSize = 9;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(noiseSize, noiseSize));
    cv::erode(fgmask, fgmask, kernel);
    kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(noiseSize, noiseSize));
    cv::dilate(fgmask, fgmask, kernel, cv::Point(-1,-1), 3);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(fgmask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    bool hasMotion = contours.size() > 0;
    if(!motionDetected && hasMotion) {
        motionDetected = true;
        setVideoSavingStatus(STARTING);
        qDebug() << "new motion detected, should send a notification.";
        QtConcurrent::run(Utilities::notifyMobile, cameraID);
    } else if (motionDetected && !hasMotion) {
        motionDetected = false;
        setVideoSavingStatus(STOPPING);
        qDebug() << "detected motion disappeared.";
    }

    cv::Scalar color = cv::Scalar(0, 0, 255); // red
    for(size_t i = 0; i < contours.size(); i++) {
        cv::Rect rect = cv::boundingRect(contours[i]);
        cv::rectangle(frame, rect, color, 1);
    }
}
