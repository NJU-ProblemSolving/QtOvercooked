#include "player.h"

#include "gamemanager.h"
#include "tile.h"

void Player::lateUpdate() {
    if (respawnCountdown > 0) {
        respawnCountdown--;
        if (respawnCountdown == 0) {
            body->SetEnabled(true);
        }
        return;
    }

    int tileX = body->GetPosition().x;
    int tileY = body->GetPosition().y;
    auto tile = gameManager->getTile(tileX, tileY);
    if (tile->getTileKind() == TileKind::Void) {
        body->SetTransform(spawnPoint, 0);
        body->SetLinearVelocity(b2Vec2(0, 0));
        body->SetEnabled(false);
        auto container = ContainerHolder(onHand.getContainerKind(), Mixture());
        container.setRespawnPoint(onHand.getRespawnPoint());
        gameManager->entityManager.scheduleRespawn(std::move(container), CONTAINER_RESPAWN_TIME);
        container = std::move(onHand);
        onHand = ContainerHolder();
        respawnCountdown = PLAYER_RESPAWN_TIME;
        tileInteracting = nullptr;
    }

    if (tileInteracting != nullptr) {
        auto distance = (body->GetPosition() - tileInteracting->getPos() -
                         b2Vec2(0.5f, 0.5f))
                            .Length();
        if (distance > PLAYER_INTERACT_DISTANCE) {
            tileInteracting = nullptr;
        }
    }
    if (tileInteracting != nullptr) {
        if (!tileInteracting->interact()) {
            tileInteracting = nullptr;
        }
    }
}
