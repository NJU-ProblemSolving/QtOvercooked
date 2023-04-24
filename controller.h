#pragma once

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>

#include <tiny-process-library/process.hpp>

#include "gamemanager.h"

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
        std::fstream fin(levelFile);
        ss << fin.rdbuf();
        ss << '\0';
        log << ">>> Request: \n" << ss.str() << "\n>>>" << std::endl;
        process->write(ss.str());
    }

    std::vector<std::string> requestInputs() override {
        std::stringstream ss;
        auto orderManager = &gameManager->orderManager;
        ss << "Frame " << orderManager->getFrame() << "\n";
        ss << orderManager->getTimeCountdown() << " ";
        ss << orderManager->getFund() << "\n";
        ss << orderManager->getOrders().size() << "\n";
        for (auto &order : orderManager->getOrders()) {
            ss << order.countdown << ' ' << order.price << ' '
               << order.mixture.toString();
            ss << '\n';
        }
        ss << gameManager->getPlayers().size() << "\n";
        for (auto &player : gameManager->getPlayers()) {
            auto x = player->getBody()->GetPosition().x;
            auto y = player->getBody()->GetPosition().y;
            auto vx = player->getBody()->GetLinearVelocity().x;
            auto vy = player->getBody()->GetLinearVelocity().y;
            ss << x << ' ' << y << ' ' << vx << ' ' << vy;
            ss << " " << player->getRespawnCountdown();
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

        auto timeout =
            (frame == 0 ? FIRST_RESPONSE_TIMEOUT : NORMAL_RESPONSE_TIMEOUT);
        if (nextInput.size() == 0) {
            int exit_status;
            if (process->try_get_exit_status(exit_status)) {
                throw std::runtime_error(
                    "Process exited unexpectedly with status " +
                    std::to_string(exit_status));
            }
            std::unique_lock<std::mutex> lk(m);
            cv.wait_for(lk, std::chrono::milliseconds(timeout),
                        [&] { return nextInput.size() > 0; });
        }
        auto res = nextInput;
        nextInput.clear();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - requestTime);
        if (res.size() == 0 || duration > timeout) {
            log << "!!! Response timeout: " << duration.count() << "ms"
                << std::endl;
            res.clear();
            for (auto &player : gameManager->getPlayers()) {
                res.push_back("Move");
            }
        } else {
            log << "!!! Response time: " << duration.count() << "ms"
                << std::endl;
        }

        frame += 1;

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
