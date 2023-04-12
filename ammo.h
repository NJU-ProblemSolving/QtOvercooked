#ifndef AMMO_H_
#define AMMO_H_

#include <component.h>
#include <gameobject.h>
#include <transform.h>

class Ammo : public Component {
  public:
    explicit Ammo(QPointF velocity);

    void onAttach() override;
    void onUpdate(float deltaTime) override;

  protected:
    Transform *transform = nullptr;
    QGraphicsItem *collider = nullptr;

    float timeToLive = 1;
    QPointF velocity;
};

#endif // AMMO_H_
