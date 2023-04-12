#include "player.h"

#include "levelmanager.h"
#include "tile.h"

void Player::lateUpdate() {
    if (respawnCountdown > 0) {
        respawnCountdown--;
        return;
    }

    int tileX = body->GetPosition().x;
    int tileY = body->GetPosition().y;
    Tile *tile = levelManager->getTile(tileX, tileY);
    if (tile->getKind() == TileKind::Void) {
        body->SetTransform(spawnPoint, 0);
        body->SetLinearVelocity(b2Vec2(0, 0));
        respawnCountdown = PLAYER_RESPAWN_TIME;
    }
}
