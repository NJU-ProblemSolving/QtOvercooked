#pragma once

#include <box2d/box2d.h>

#include "interfaces.h"
#include "player.h"

class CollisionListener : public b2ContactListener {
    void BeginContact(b2Contact *contact) override {
        auto entityA = reinterpret_cast<IBody *>(
            contact->GetFixtureA()->GetBody()->GetUserData().pointer);
        auto entityB = reinterpret_cast<IBody *>(
            contact->GetFixtureB()->GetBody()->GetUserData().pointer);

        if (entityA->getBodyKind() == BodyKind::Player) {
            static_cast<Player *>(entityA)->collision(entityB);
        }
        if (entityB->getBodyKind() == BodyKind::Player) {
            static_cast<Player *>(entityB)->collision(entityA);
        }
    }
};
