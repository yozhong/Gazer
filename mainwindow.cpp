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
}

MainWindow::~MainWindow()
{
    delete ui;
}

