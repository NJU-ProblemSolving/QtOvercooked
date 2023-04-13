#pragma once

#include <box2d/box2d.h>
#include <optional>

#include "foodcontainer.h"
#include "interfaces.h"
#include "mixture.h"
#include "recipe.h"

class GameManager;

class Tile : public IBody {
  public:
    Tile() {}

    b2Vec2 getPos() { return position; }
    void setPos(b2Vec2 position) { this->position = position; }

    virtual void initB2(b2World *world) {}
    void setLevelManager(GameManager *levelManager) {
        this->levelManager = levelManager;
    }

    virtual bool put(FoodContainer &container) { return false; }
    virtual FoodContainer pick() { return FoodContainer(ContainerKind::None); }
    virtual bool interact() { return false; }
    virtual FoodContainer *getContainer() { return nullptr; }

    TileKind getTileKind() const { return tileKind; }

  protected:
    b2Vec2 position;
    TileKind tileKind = TileKind::None;
    GameManager *levelManager = nullptr;
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

    virtual bool put(FoodContainer &container) override {
        return containerOnTable.put(container);
    }

    virtual FoodContainer pick() override {
        return std::move(containerOnTable);
    }

    virtual FoodContainer *getContainer() override { return &containerOnTable; }

    void setContainer(FoodContainer &&container) {
        containerOnTable = std::move(container);
    }

  protected:
    FoodContainer containerOnTable;
};

class TilePantry : public TileTable {
  public:
    TilePantry() { tileKind = TileKind::Pantry; }

    void setIngredient(std::string ingredient) {
        this->ingredient = ingredient;
    }

    FoodContainer pick() override {
        auto container = TileTable::pick();
        if (container.isNull()) {
            container = FoodContainer(ContainerKind::None, Mixture(ingredient));
        }
        return std::move(container);
    }

  protected:
    std::string ingredient;
};

class TileCuttingBoard : public TileTable {
  public:
    TileCuttingBoard() { tileKind = TileKind::CuttingBoard; }

    bool put(FoodContainer &container) override {
        if (containerOnTable.isWorking()) {
            return false;
        }
        return TileTable::put(container);
    }

    FoodContainer pick() override {
        if (containerOnTable.isWorking()) {
            return FoodContainer(ContainerKind::None);
        }
        return TileTable::pick();
    }

    bool interact() override;
};

class TileServingHatch : public TileWall {
  public:
    TileServingHatch() { tileKind = TileKind::ServingHatch; }

    bool put(FoodContainer &container) override {
        if (container.getContainerKind() != ContainerKind::Dish) {
            return false;
        }
        container = FoodContainer();
        return true;
    }
};

class TileDishTable : public TileWall {
  public:
    TileDishTable() { tileKind = TileKind::DishTable; }

    FoodContainer pick() override { return FoodContainer(ContainerKind::Dish); }
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
    case TileKind::CuttingBoard:
        return new TileCuttingBoard();
    case TileKind::ServingHatch:
        return new TileServingHatch();
    case TileKind::Pantry:
        return new TilePantry();
    case TileKind::DishTable:
        return new TileDishTable();
    default:
        throw "Unexpected tile kind";
    }
}
