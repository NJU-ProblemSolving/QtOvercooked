#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QElapsedTimer>
#include <QTimer>

#include "levelmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    gameScene = new GameScene();
    gameScene->setParent(this);
    auto view = new QGraphicsView(gameScene, this);
    view->setFrameStyle(QFrame::NoFrame);
    view->resize(this->size());
    view->setSceneRect(QRect(0, 0, 400, 400));

    auto levelManager = new LevelManager();
    levelManager->setScene(gameScene);
    levelManager->loadLevel("D:/QtOvercooked/level1.txt");

    updateTimer = new QTimer(this);
    auto time = new QElapsedTimer();
    updateTimer->setInterval(1000.0f / FPS);
    connect(updateTimer, &QTimer::timeout, this, [=]() { levelManager->step(); });
    updateTimer->start();
}

MainWindow::~MainWindow() { delete ui; }
