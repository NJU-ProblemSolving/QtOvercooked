#ifndef TILE_H_
#define TILE_H_

#include <box2d/box2d.h>

#include "interfaces.h"

enum class TileKind {
    None,
    Void,
    Floor,
    Wall,
    Table,
    Pantry,
    CuttingBoard,
    WashBowl,
    Stove,
    Trashbin,
};

class Tile : public IEntity {
  public:
    Tile() {}

    void setPos(b2Vec2 position) { this->position = position; }

    virtual void initB2(b2World *world) {}

    virtual TileKind getKind() { return tileKind; }

  protected:
    b2Vec2 position;
    TileKind tileKind = TileKind::None;
};

class TileVoid : public Tile {
  public:
    TileVoid() { tileKind = TileKind::Void; }
};

class TileFloor : public Tile {
  public:
    TileFloor() { tileKind = TileKind::Floor; }
};

class TileWall : public Tile {
  public:
    TileWall() { tileKind = TileKind::Wall; }

    void initB2(b2World *world) override {
        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position.Set(position.x + 0.5f, position.y + 0.5f);
        body = world->CreateBody(&bodyDef);
        b2PolygonShape shape;
        shape.SetAsBox(0.5f, 0.5f);
        body->CreateFixture(&shape, 0.0f);

        setUserData(EntityKind::Wall);
    }
};

class TileTable : public TileWall {
  public:
    TileTable() { tileKind = TileKind::Table; }
};

inline Tile *CreateTile(char kind) {
    switch (kind) {
    case ' ':
        return new TileVoid();
    case '.':
        return new TileFloor();
    case '*':
        return new TileTable();
    default:
        return new Tile();
    }
}

#endif // TILE_H_
