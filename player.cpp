#include "player.h"

#include "gamemanager.h"
#include "tile.h"

void Player::lateUpdate() {
    if (respawnCountdown > 0) {
        respawnCountdown--;
        return;
    }

    int tileX = body->GetPosition().x;
    int tileY = body->GetPosition().y;
    auto tile = levelManager->getTile(tileX, tileY);
    if (tile->getTileKind() == TileKind::Void) {
        body->SetTransform(spawnPoint, 0);
        body->SetLinearVelocity(b2Vec2(0, 0));
        body->SetEnabled(false);
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
