#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QTimer>

#include "gamescene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    Ui::MainWindow *ui;
    GameScene *gameScene;
    QTimer *updateTimer;
};
#endif // MAINWINDOW_H_
