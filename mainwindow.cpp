#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QElapsedTimer>
#include <QTimer>

#include "gamemanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    guiManager = new GuiManager();
    guiManager->setParent(this);
    auto view = new QGraphicsView(guiManager, this);
    view->setFrameStyle(QFrame::NoFrame);
    view->resize(this->size());
    view->setSceneRect(QRect(0, 0, 400, 400));

    guiManager->load("D:/QtOvercooked/level1.txt");

    updateTimer = new QTimer(this);
    auto time = new QElapsedTimer();
    updateTimer->setInterval(1000.0f / FPS);
    connect(updateTimer, &QTimer::timeout, guiManager,
            &GuiManager::step);
    updateTimer->start();
}

MainWindow::~MainWindow() { delete ui; }
