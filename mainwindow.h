#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <fstream>

#include "./ui_mainwindow.h"
#include "mygetopt.h"
#include "guimanager.h"
#include "controller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class GuiController : public Controller {
    GuiManager *guiManager;

  public:
    GuiController(GameManager *gameManager, GuiManager *guiManager)
        : Controller(gameManager), guiManager(guiManager) {}

    void init(const char *levelFile) override {}

    std::vector<std::string> requestInputs() override {
        std::vector<std::string> res;
        {
            static std::string lastMove;
            std::string direction;
            if (guiManager->getKey(Qt::Key_A)) {
                direction += "L";
            }
            if (guiManager->getKey(Qt::Key_D)) {
                direction += "R";
            }
            if (guiManager->getKey(Qt::Key_W)) {
                direction += "U";
            }
            if (guiManager->getKey(Qt::Key_S)) {
                direction += "D";
            }
            if (direction.size() > 0) {
                lastMove = direction;
            }
            if (guiManager->getKeyDown(Qt::Key_Space)) {
                res.push_back("PutOrPick " + lastMove);
            } else if (guiManager->getKeyDown(Qt::Key_J)) {
                res.push_back("Interact " + lastMove);
            } else {
                res.push_back("Move " + direction);
            }
        }
        {
            static std::string lastMove;
            std::string direction;
            if (guiManager->getKey(Qt::Key_Left)) {
                direction += "L";
            }
            if (guiManager->getKey(Qt::Key_Right)) {
                direction += "R";
            }
            if (guiManager->getKey(Qt::Key_Up)) {
                direction += "U";
            }
            if (guiManager->getKey(Qt::Key_Down)) {
                direction += "D";
            }
            if (direction.size() > 0) {
                lastMove = direction;
            }
            if (guiManager->getKeyDown(Qt::Key_Return)) {
                res.push_back("PutOrPick " + lastMove);
            } else if (guiManager->getKeyDown(Qt::Key_Control)) {
                res.push_back("Interact " + lastMove);
            } else {
                res.push_back("Move " + direction);
            }
        }
        for (int i = 2; i < gameManager->getPlayers().size(); i++) {
            res.push_back("Move ");
        }
        return res;
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void init(int argc, char *argv[]) {
        const char *levelFile = "level1.txt";
        const char *program = nullptr;
        int o;
        while ((o = getopt(argc, argv, "l:p:")) != -1) {
            switch (o) {
            case 'l':
                levelFile = optarg;
                break;
            case 'p':
                program = optarg;
                break;
            default:
                printf("Unknown commandline argument %c\n", o);
                break;
            }
        }

        gameManager->loadLevel(levelFile);
        guiManager->init();

        if (program != nullptr) {
            controller = new CliController(gameManager, program);
        } else {
            controller = new GuiController(gameManager, guiManager);
        }
        controller->init(levelFile);
        guiManager->updateItem();

        auto t = QThread::create([&]() {
            auto lastTime = std::chrono::system_clock::now();
            while (true) {
                emit onInput(controller->requestInputs());
                auto now = std::chrono::system_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - lastTime);
                lastTime = now;
                if (duration.count() < 1000.0f / FPS) {
                    QThread::msleep(1000.0f / FPS - duration.count());
                }
            }
        });
        t->start();
        connect(this, &MainWindow::onInput, this, &MainWindow::step,
                Qt::BlockingQueuedConnection);
    }

    void closeEvent(QCloseEvent *event) override { delete controller; }

    std::pair<int, int> parseDirection(std::string direction) {
        int x = 0;
        int y = 0;
        for (auto c : direction) {
            switch (c) {
            case 'L':
                x -= 1;
                break;
            case 'R':
                x += 1;
                break;
            case 'U':
                y -= 1;
                break;
            case 'D':
                y += 1;
                break;
            }
        }
        if (x < -1) x = -1;
        if (x > 1) x = 1;
        if (y < -1) y = -1;
        if (y > 1) y = 1;
        return {x, y};
    }

  signals:
    void onInput(std::vector<std::string> inputs);

  public slots:
    void step(std::vector<std::string> inputs) {
        assert(inputs.size() == gameManager->getPlayers().size());
        for (int i = 0; i < inputs.size(); i++) {
            auto &input = inputs[i];
            auto &player = gameManager->getPlayers()[i];
            if (input.starts_with("Move")) {
                auto direction = input.substr(4);
                auto [x, y] = parseDirection(direction);
                gameManager->move(i, b2Vec2(x, y));
            } else if (input.starts_with("Interact")) {
                auto direction = input.substr(8);
                auto [x, y] = parseDirection(direction);
                x += player->getBody()->GetPosition().x;
                y += player->getBody()->GetPosition().y;
                gameManager->interact(i, x, y);
            } else if (input.starts_with("PutOrPick")) {
                auto direction = input.substr(9);
                auto [x, y] = parseDirection(direction);
                x += player->getBody()->GetPosition().x;
                y += player->getBody()->GetPosition().y;
                gameManager->putOrPick(i, x, y);
            }
        }
        gameManager->step();
        guiManager->step();
        guiManager->updateItem();
    }

  private:
    Ui::MainWindow *ui;
    GuiManager *guiManager;
    GameManager *gameManager;
    Controller *controller;

  signals:
    void updated();
};
#endif // MAINWINDOW_H_
