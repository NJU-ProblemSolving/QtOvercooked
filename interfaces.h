#pragma once

#include <box2d/box2d.h>

class IUpdatable {
  public:
    virtual void update() {}
    virtual void lateUpdate() {}
};

enum class BodyKind {
    Unknown,
    Player,
    Wall,
};

class IBody {
  public:
    b2Body *getBody() { return body; }
    BodyKind getBodyKind() { return bodyKind; }

  protected:
    b2Body *body = nullptr;
    BodyKind bodyKind = BodyKind::Unknown;

    void setUserData(BodyKind kind) {
        bodyKind = kind;
        body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
    }
};
