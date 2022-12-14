#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QMutex>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>

#include "capture_thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat*);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *imageScene;
    cv::Mat currentFrame;
    // for capture thread
    QMutex *dataLock;
    CaptureThread *capturer;
#ifdef GAZER_USE_QT_CAMERA
    QCamera *camera;
    QMediaCaptureSession *captureSession;
    QVideoWidget *videoWidget;
#endif  /* GAZER_USE_QT_CAMERA */
};
#endif // MAINWINDOW_H
