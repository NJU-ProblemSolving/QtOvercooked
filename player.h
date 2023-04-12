#ifndef PLAYER_H_
#define PLAYER_H_

#include <QDebug>
#include <box2d/box2d.h>

#include "config.h"
#include "interfaces.h"

class LevelManager;

class Player : public IUpdatable, public IEntity {
  public:
    Player() { entityKind = EntityKind::Player; }

    void setLevelManager(LevelManager *levelManager) {
        this->levelManager = levelManager;
    }
    void setSpawnPoint(b2Vec2 point) { spawnPoint = point; }

    void initB2(b2World *world) {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(spawnPoint.x, spawnPoint.y);
        bodyDef.fixedRotation = true;
        body = world->CreateBody(&bodyDef);
        b2CircleShape shape;
        shape.m_radius = PLAYER_RADIUS;
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;
        body->CreateFixture(&fixtureDef);

        setUserData(EntityKind::Player);
    }

    void move(b2Vec2 direction) {
        if (respawnCountdown > 0) {
            return;
        }

        b2Vec2 velocity = body->GetLinearVelocity();
        b2Vec2 velocityDirection = velocity;
        velocityDirection.Normalize();
        float friction = PLAYER_FRICTION;

        if (direction.Length() > 0.1f) {
            direction.Normalize();
            direction *= PLAYER_ACCELERATION;
            body->ApplyForceToCenter(direction, true);
        } else {
            friction += PLAYER_DECELERATION;
        }

        if (velocity.Length() > PLAYER_EPISILON_SPEED) {
            auto force = velocityDirection;
            force *= -friction;
            body->ApplyForceToCenter(force, true);
        } else {
            body->SetLinearVelocity(b2Vec2(0, 0));
        }

        if (velocity.Length() > PLAYER_MAX_SPEED) {
            auto speed = velocityDirection;
            speed *= PLAYER_MAX_SPEED;
            body->SetLinearVelocity(speed);
        }
    }

    void update() override {}

    void lateUpdate() override;

    void collision(IEntity *entity) {
        qDebug() << "Player collided with entity";
    }

  protected:
    int respawnCountdown = 0;

    b2Vec2 spawnPoint;
    LevelManager *levelManager;
};

#endif
