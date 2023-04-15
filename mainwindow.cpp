#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QElapsedTimer>
#include <QTimer>

#include "gamemanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    gameManager = new GameManager();
    guiManager = new GuiManager(gameManager);
    guiManager->setParent(this);
    auto view = new QGraphicsView(guiManager, this);
    view->setFrameStyle(QFrame::NoFrame);
    view->resize(this->size());
    view->setSceneRect(QRect(0, 0, 400, 400));
}

MainWindow::~MainWindow() { delete ui; }
