#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imageScene = new QGraphicsScene(this);
    ui->imageView->setScene(imageScene);
    ui->mainStatusBar->addPermanentWidget(ui->mainStatusLabel);
    ui->mainStatusLabel->setText("Gazer is Ready");

    // connect the signals and slots
    connect(ui->actionCamera_Information, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(ui->actionOpen_Camera, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));

    dataLock = new QMutex();

#ifdef GAZER_USE_QT_CAMERA
    videoWidget = new QVideoWidget();
    videoWidget->resize(640, 480);

    ui->gridLayout->addWidget(videoWidget, 0, 0, 1, 3);

    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    camera = new QCamera(cameras[1]);
    camera->stop();

    captureSession = new QMediaCaptureSession();
    captureSession->setCamera(camera);
    captureSession->setVideoOutput(videoWidget);
#endif /* GAZER_USE_QT_CAMERA */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showCameraInfo()
{
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QString info = QString("Available Cameras: \n");

    foreach (const QCameraDevice &cameraInfo, cameras) {
        info += cameraInfo.id() + ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras", info);
}

#ifdef GAZER_USE_QT_CAMERA
void MainWindow::openCamera()
{
    camera->start();

    ui->mainStatusLabel->setText(QString("Capturing Camera %1")
        .arg(camera->cameraDevice().description()));
}
#else /* GAZER_USE_QT_CAMERA */
void MainWindow::openCamera()
{
    int camID = 2;

    if(capturer != nullptr) {
        // if a thread is already running, stop it
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }

    capturer = new CaptureThread(camID, dataLock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    capturer->start();

    ui->mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));
}
#endif /* GAZER_USE_QT_CAMERA */

void MainWindow::updateFrame(cv::Mat* mat)
{
    dataLock->lock();
    currentFrame = *mat;
    dataLock->unlock();

    QImage frame(
        currentFrame.data,
        currentFrame.cols,
        currentFrame.rows,
        currentFrame.step,
        QImage::Format_RGB888);

    QPixmap image = QPixmap::fromImage(frame);

    imageScene->clear();
    imageScene->addPixmap(image);
    imageScene->update();

    ui->imageView->resetTransform();
    ui->imageView->setSceneRect(image.rect());
}
