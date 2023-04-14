#pragma once

#include <queue>

#include "foodcontainer.h"

class GameManager;

// A comparer that only compares the first element of the pair
struct CompareFirst {
    template <typename T, typename U>
    bool operator()(const std::pair<T, U> &left, const std::pair<T, U> &right) {
        return left.first > right.first;
    }
};

class EntityManager {
    std::priority_queue<std::pair<int, ContainerHolder *>,
                        std::vector<std::pair<int, ContainerHolder *>>,
                        CompareFirst>
        respawnQueue;
    int current = 0;

    GameManager *gameManager;

  public:
    EntityManager() {}

    void scheduleRespawn(ContainerHolder &&container, int delay) {
        auto p = new ContainerHolder();
        *p = std::move(container);
        respawnQueue.push(std::make_pair(current + delay, p));
    }

    void step();

    void setGameManager(GameManager *gameManager) {
        this->gameManager = gameManager;
    }
};
