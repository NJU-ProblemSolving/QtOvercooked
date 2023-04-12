#ifndef COLLISIONLISTENER_H_
#define COLLISIONLISTENER_H_

#include <box2d/box2d.h>

#include "interfaces.h"
#include "player.h"

class CollisionListener : public b2ContactListener {
    void BeginContact(b2Contact *contact) override {
        auto entityA = reinterpret_cast<IEntity *>(
            contact->GetFixtureA()->GetBody()->GetUserData().pointer);
        auto entityB = reinterpret_cast<IEntity *>(
            contact->GetFixtureB()->GetBody()->GetUserData().pointer);

        if (entityA->getEntityKind() == EntityKind::Player) {
            static_cast<Player *>(entityA)->collision(entityB);
        }
        if (entityB->getEntityKind() == EntityKind::Player) {
            static_cast<Player *>(entityB)->collision(entityA);
        }
    }
};

#endif
