#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <codecvt>
#include <condition_variable>
#include <fstream>
#include <getopt.h>
#include <locale>
#include <mutex>

#include "./ui_mainwindow.h"
#include "guimanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Controller {
  protected:
    GameManager *gameManager;

  public:
    Controller(GameManager *gameManager) : gameManager(gameManager) {}
    virtual ~Controller() {}

    virtual void init(const char *levelFile) = 0;
    virtual std::vector<std::string> requestInputs() = 0;
};

class CliController : public Controller {
    TinyProcessLib::Process *process;
    int frame = 0;
    std::ofstream log;

    std::mutex m;
    std::condition_variable cv;
    std::vector<std::string> nextInput;

    std::chrono::time_point<std::chrono::system_clock> requestTime;
    std::chrono::time_point<std::chrono::system_clock> responseTime;

  public:
    CliController(GameManager *g, const char *program) : Controller(g) {
        log.open("clilog.txt", std::ios::out | std::ios::trunc);

        process = new TinyProcessLib::Process(
            program, TinyProcessLib::Process::string_type(),
            [&](const char *bytes, size_t n) {
                std::string s(bytes, n);
                if (log.is_open()) {
                    log << "<<< Response: \n" << s << "\n<<<" << std::endl;
                    log.flush();
                }

                std::stringstream ss(s);

                ss >> s;
                assert(s == "Frame");
                int responseFrame;
                ss >> responseFrame;
                std::getline(ss, s);

                if (responseFrame != frame) {
                    log << "!!! Frame mismatch: response " << responseFrame
                        << " != current " << frame << std::endl;
                    log.flush();
                    return;
                }

                responseTime = std::chrono::system_clock::now();
                {
                    std::unique_lock<std::mutex> lk(m);
                    for (int i = 0; i < gameManager->getPlayers().size(); i++) {
                        std::getline(ss, s);
                        nextInput.push_back(s);
                    }
                }
                cv.notify_all();
            },
            [&](const char *bytes, size_t n) {
                std::string s(bytes, n);
                if (log.is_open()) {
                    log << "<<< Stderr: \n" << s << "\n<<<" << std::endl;
                }
            },
            true);
    }

    ~CliController() {
        process->kill(true);
        delete process;
        log.close();
    }

    void init(const char *levelFile) override {
        std::stringstream ss;
        ss << "Frame " << frame;
        std::fstream fin(levelFile);
        ss << fin.rdbuf();
        ss << '\0';
        log << ">>> Request: \n" << ss.str() << "\n>>>" << std::endl;
        process->write(ss.str());
        requestTime = std::chrono::system_clock::now();
    }

    std::vector<std::string> requestInputs() override {
        frame += 1;
        std::stringstream ss;
        auto orderManager = &gameManager->orderManager;
        ss << "Frame " << orderManager->getFrame() << "\n";
        ss << orderManager->getTimeCountdown() << " ";
        ss << orderManager->getFund() << "\n";
        ss << orderManager->getOrders().size() << "\n";
        for (auto &order : orderManager->getOrders()) {
            ss << order.time << ' ' << order.price << ' '
               << order.mixture.toString();
            ss << '\n';
        }
        ss << gameManager->getPlayers().size() << "\n";
        for (auto &player : gameManager->getPlayers()) {
            auto x = player->getBody()->GetPosition().x;
            auto y = player->getBody()->GetPosition().y;
            ss << x << ' ' << y;
            if (!player->getOnHand()->isNull()) {
                ss << " ; ";
                printContainer(ss, player->getOnHand());
            }
            ss << '\n';
        }
        int count = 0;
        for (auto &tile : gameManager->getTiles()) {
            if (tile->getContainer() != nullptr &&
                !tile->getContainer()->isNull()) {
                count++;
            }
        }
        ss << count << "\n";
        for (auto &tile : gameManager->getTiles()) {
            if (tile->getContainer() != nullptr &&
                !tile->getContainer()->isNull()) {
                auto x = tile->getPos().x;
                auto y = tile->getPos().y;
                ss << x << ' ' << y << ' ';
                printContainer(ss, tile->getContainer());
                ss << '\n';
            }
        }
        ss << '\0';
        log << ">>> Request: \n" << ss.str() << "\n>>>" << std::endl;
        process->write(ss.str());
        requestTime = std::chrono::system_clock::now();

        if (nextInput.size() == 0) {
            int exit_status;
            if (process->try_get_exit_status(exit_status)) {
                throw std::runtime_error(
                    "Process exited unexpectedly with status " +
                    std::to_string(exit_status));
            }
            std::unique_lock<std::mutex> lk(m);
            cv.wait_for(lk, std::chrono::milliseconds(50),
                        [&] { return nextInput.size() > 0; });
        }
        auto res = nextInput;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now() - requestTime)
                            .count();
        if (res.size() == 0 || duration > 50) {
            log << "!!! Response timeout: " << duration << "ms" << std::endl;
            res.clear();
            for (auto &player : gameManager->getPlayers()) {
                res.push_back("Move");
            }
        } else {
            log << "!!! Response time: " << duration << "ms" << std::endl;
        }
        nextInput.clear();
        return res;
    }

    void printContainer(std::ostream &os, ContainerHolder *container) {
        os << container->toString();
        if (container->isWorking()) {
            os << " ; " << container->getProgressTick() << " / "
               << container->getProgressTickMax();
        }
    }
};

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
                    qDebug("%f ms", 1000.0f / FPS - duration.count());
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
        assert(x <= 1 && x >= -1);
        assert(y <= 1 && y >= -1);
        return {x, y};
    }

  signals:
    void onInput(std::vector<std::string> inputs);

  public slots:
    void step(std::vector<std::string> inputs) {
        static auto lastTime = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastTime);
        lastTime = now;
        if (duration.count() < 1000.0f / FPS) {
            qDebug("step %f ms", 1000.0f / FPS - duration.count());
        }
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
