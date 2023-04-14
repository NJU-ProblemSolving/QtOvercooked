#pragma once

#include <box2d/box2d.h>
#include <optional>

#include "foodcontainer.h"
#include "interfaces.h"
#include "mixture.h"

class GameManager;

class Tile : public IBody {
  public:
    Tile() {}

    b2Vec2 getPos() { return position; }
    void setPos(b2Vec2 position) { this->position = position; }

    virtual void initB2(b2World *world) {}
    void setGameManager(GameManager *levelManager) {
        this->gameManager = levelManager;
    }

    virtual bool put(ContainerHolder &container) { return false; }
    virtual ContainerHolder pick() { return ContainerHolder(); }
    virtual bool interact() { return false; }
    virtual ContainerHolder *getContainer() { return nullptr; }

    TileKind getTileKind() const { return tileKind; }

  protected:
    b2Vec2 position{};
    TileKind tileKind = TileKind::None;
    GameManager *gameManager = nullptr;
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

        setUserData(BodyKind::Wall);
    }
};

class TileTable : public TileWall {
  public:
    TileTable() { tileKind = TileKind::Table; }

    virtual bool put(ContainerHolder &container) override {
        return containerOnTable.put(container);
    }

    virtual ContainerHolder pick() override {
        return std::move(containerOnTable);
    }

    virtual ContainerHolder *getContainer() override {
        return &containerOnTable;
    }

    void setContainer(ContainerHolder &&container) {
        containerOnTable = std::move(container);
    }

  protected:
    ContainerHolder containerOnTable;
};

class TilePantry : public TileTable {
  public:
    TilePantry() { tileKind = TileKind::Pantry; }

    std::string getIngredient() { return ingredient; }
    void init(std::string ingredient, int price) {
        this->ingredient = ingredient;
        this->price = price;
    }

    ContainerHolder pick() override {
        auto container = TileTable::pick();
        if (container.isNull()) {
            container =
                ContainerHolder(ContainerKind::None, Mixture(ingredient));
        }
        return std::move(container);
    }

  protected:
    std::string ingredient;
    int price;
};

class TileTrashbin : public TileWall {
  public:
    TileTrashbin() { tileKind = TileKind::Trashbin; }

    bool put(ContainerHolder &container) override {
        container.setMixture(Mixture());
        return true;
    }
};

class TileCuttingBoard : public TileTable {
  public:
    TileCuttingBoard() { tileKind = TileKind::CuttingBoard; }

    bool interact() override;
};

class TileStove : public TileTable, public IUpdatable {
  public:
    TileStove() { tileKind = TileKind::Stove; }

    void lateUpdate() override;
};

class TileServingHatch : public TileWall {
  public:
    TileServingHatch() { tileKind = TileKind::ServingHatch; }

    bool put(ContainerHolder &container) override;
};

class TileDishTable : public TileTable {
  public:
    TileDishTable() { tileKind = TileKind::DishTable; }
};

class TileDirtyDishTable : public TileTable {
  public:
    TileDirtyDishTable() { tileKind = TileKind::DirtyDishTable; }
};

inline Tile *CreateTile(TileKind kind) {
    switch (kind) {
    case TileKind::Void:
        return new TileVoid();
    case TileKind::Floor:
        return new TileFloor();
    case TileKind::Wall:
        return new TileWall();
    case TileKind::Table:
        return new TileTable();
    case TileKind::Trashbin:
        return new TileTrashbin();
    case TileKind::CuttingBoard:
        return new TileCuttingBoard();
    case TileKind::Stove:
        return new TileStove();
    case TileKind::ServingHatch:
        return new TileServingHatch();
    case TileKind::Pantry:
        return new TilePantry();
    case TileKind::DishTable:
        return new TileDishTable();
    case TileKind::DirtyDishTable:
        return new TileDirtyDishTable();
    default:
        throw std::runtime_error("Unknown tile kind");
    }
}
