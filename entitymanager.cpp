#include "entitymanager.h"

#include "gamemanager.h"

void EntityManager::step() {
    current++;
    while (!respawnQueue.empty() && current >= respawnQueue.top().first) {
        ContainerHolder *container = respawnQueue.top().second;
        respawnQueue.pop();
        auto [x, y] = container->getRespawnPoint();
        auto tile = gameManager->getTile(x, y);
        if (!tile->put(*container)) {
            respawnQueue.push(std::make_pair(current + 1, container));
        } else {
            delete container;
        }
    }
}
