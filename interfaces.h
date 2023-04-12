#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <box2d/box2d.h>

class IUpdatable {
  public:
    virtual void update() {}
    virtual void lateUpdate() {}
};

enum class EntityKind {
    Unknown,
    Player,
    Wall,
};

class IEntity {
  public:
    b2Body *getBody() { return body; }
    EntityKind getEntityKind() { return entityKind; }

  protected:
    b2Body *body;
    EntityKind entityKind = EntityKind::Unknown;

    void setUserData(EntityKind kind) {
        entityKind = kind;
        body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
    }
};

#endif // INTERFACE_H_
