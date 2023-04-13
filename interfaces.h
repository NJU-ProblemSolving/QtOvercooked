#ifndef INTERFACE_H_
#define INTERFACE_H_

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
    b2Body *body;
    BodyKind bodyKind = BodyKind::Unknown;

    void setUserData(BodyKind kind) {
        bodyKind = kind;
        body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
    }
};

#endif // INTERFACE_H_
