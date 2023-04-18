#include "entitymanager.h"

#include "gamemanager.h"

void EntityManager::step() {
    current++;
    while (!respawnQueue.empty() && current >= respawnQueue.top().first) {
        ContainerHolder *container = respawnQueue.top().second;
        respawnQueue.pop();
        auto [x, y] = container->getRespawnPoint();
        auto tile = gameManager->getTile(x, y);
        if (tile->getContainer()->isNull() ||
            (tile->getContainer()->getContainerKind() ==
                 ContainerKind::DirtyPlates &&
             container->getContainerKind() == ContainerKind::DirtyPlates)) {
            auto res = tile->put(*container);
            assert(res);
            delete container;
        } else {
            respawnQueue.push(std::make_pair(current + 1, container));
        }
    }
}
