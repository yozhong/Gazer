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

    listModel = new QStandardItemModel(this);
    ui->savedList->setModel(listModel);

    // connect the signals and slots
    connect(ui->actionCamera_Information, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(ui->actionOpen_Camera, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(ui->actionCalculate_FPS, SIGNAL(triggered(bool)), this, SLOT(calculateFPS()));
    connect(ui->recordButton, SIGNAL(clicked(bool)), this, SLOT(recordingStartStop()));

    dataLock = new QMutex();

#ifdef GAZER_USE_QT_CAMERA
    videoWidget = new QVideoWidget();
    videoWidget->resize(640, 480);

    ui->gridLayout->addWidget(videoWidget, 0, 0, 1, 3);

    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    camera = new QCamera(cameras[1]);
    camera->stop();

    captureSession = new QMediaCaptureSession();
    captureSession->setVideoOutput(videoWidget);
    captureSession->setCamera(camera);
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
        disconnect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
        disconnect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }

    capturer = new CaptureThread(camID, dataLock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
    connect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
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

void MainWindow::calculateFPS()
{
    if(capturer != nullptr) {
        capturer->startCalcFPS();
    }
}

void MainWindow::updateFPS(float fps)
{
    ui->mainStatusLabel->setText(QString("FPS of current camera is %1").arg(fps));
}

void MainWindow::recordingStartStop() {
    QString text = ui->recordButton->text();

    if(text == "Record" && capturer != nullptr) {
        capturer->setVideoSavingStatus(CaptureThread::STARTING);
        ui->recordButton->setText("Stop Recording");
    } else if(text == "Stop Recording" && capturer != nullptr) {
        capturer->setVideoSavingStatus(CaptureThread::STOPPING);
        ui->recordButton->setText("Record");
    }
}

void MainWindow::appendSavedVideo(QString name)
{
    QString cover = Utilities::getSavedVideoPath(name, "jpg");
    QStandardItem *item = new QStandardItem();
    listModel->appendRow(item);
    QModelIndex index = listModel->indexFromItem(item);
    listModel->setData(index, QPixmap(cover).scaledToHeight(145), Qt::DecorationRole);
    listModel->setData(index, name, Qt::DisplayRole);
}
