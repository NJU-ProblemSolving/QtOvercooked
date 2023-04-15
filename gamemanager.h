#pragma once

#include <QDebug>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <box2d/box2d.h>

#include "collisionlisener.h"
#include "config.h"
#include "entitymanager.h"
#include "foodcontainer.h"
#include "interfaces.h"
#include "ordermanager.h"
#include "player.h"
#include "recipe.h"
#include "tile.h"

class GameManager {
  public:
    GameManager() {}

    void loadLevel(const std::string &path) {
        world = new b2World(b2Vec2(0.0f, 0.0f));
        world->SetContactListener(new CollisionListener());

        std::ifstream in(path);

        in >> width >> height;
        map.resize(width * height);
        in >> std::noskipws;
        for (int i = 0; i < width * height; i++) {
            char kindChar;
            do {
                in >> kindChar;
            } while (kindChar == '\n' || kindChar == '\r');

            if (kindChar >= 'A' && kindChar <= 'Z') {
                map[i] = nullptr;
            } else {
                addTile(i, getTileKind(kindChar));
            }
        }
        in >> std::skipws;

        int illustrationCount;
        in >> illustrationCount;
        for (int i = 0; i < illustrationCount; i++) {
            std::string s;
            int x, y;
            in >> s >> x >> y;
            int pos = y * width + x;
            if (s == "IngredientBox") {
                std::string ingredient;
                int price;
                in >> ingredient >> price;
                assert(map[pos] == nullptr);
                addTile(pos, TileKind::IngredientBox);
                auto pantry = static_cast<TileIngredientBox *>(map[pos]);
                pantry->init(ingredient, price);
            } else {
                throw std::runtime_error("Invalid illustration");
            }
        }

        int recipeCount;
        in >> recipeCount;
        for (int i = 0; i < recipeCount; i++) {
            std::string s;
            do {
                getline(in, s);
            } while (s.length() == 0);
            std::stringstream ss(s);

            int time;
            ss >> time;

            ss >> s;
            Mixture ingredients;
            while (!s.starts_with("-")) {
                ingredients.add(s);
                ss >> s;
            }

            ContainerKind containerKind;
            TileKind tileKind;
            if (s == "-chop->") {
                containerKind = ContainerKind::None;
                tileKind = TileKind::ChoppingStation;
            } else if (s == "-pot->") {
                containerKind = ContainerKind::Pot;
                tileKind = TileKind::Stove;
            } else if (s == "-pan->") {
                containerKind = ContainerKind::Pan;
                tileKind = TileKind::Stove;
            } else {
                throw std::runtime_error("Invalid recipe");
            }

            Mixture results;
            while (ss.good()) {
                ss >> s;
                results.add(s);
            }

            recipes.push_back(
                Recipe(ingredients, results, containerKind, tileKind, time));
        }

        int totalTime, randomizeSeed, orderTemplateCount;
        in >> totalTime >> randomizeSeed >> orderTemplateCount;
        orderManager.setTimeCountdown(totalTime);
        for (int i = 0; i < orderTemplateCount; i++) {
            std::string s;
            do {
                getline(in, s);
            } while (s.length() == 0);
            std::stringstream ss(s);

            int time, price, weight;
            ss >> time >> price >> weight;

            Mixture ingredients;
            while (ss.good()) {
                ss >> s;
                ingredients.add(s);
            }

            orderManager.addOrderTemplates(
                OrderTemplate(ingredients, price, time, weight));
        }
        for (int i = 0; i < 4; i++) {
            orderManager.generateOrder();
        }

        int playerCount;
        in >> playerCount;
        for (int i = 0; i < playerCount; i++) {
            int x, y;
            in >> x >> y;
            int pos = y * width + x;
            assert(map[pos]->getTileKind() == TileKind::Floor);
            addPlayer(x, y);
        }

        int entityCount;
        in >> entityCount;
        entityManager.setGameManager(this);
        for (int i = 0; i < entityCount; i++) {
            int x, y;
            std::string s;
            in >> x >> y >> s;
            int pos = y * width + x;
            if (s == "Pot") {
                auto table = static_cast<TileTable *>(map[pos]);
                auto pot = ContainerHolder(ContainerKind::Pot, Mixture());
                pot.setRespawnPoint(std::make_pair(x, y));
                table->put(pot);
            } else if (s == "Pan") {
                auto table = static_cast<TileTable *>(map[pos]);
                auto pan = ContainerHolder(ContainerKind::Pan, Mixture());
                pan.setRespawnPoint(std::make_pair(x, y));
                table->put(pan);
            } else if (s == "Plate") {
                assert(map[pos]->getTileKind() == TileKind::Table);
                auto table = static_cast<TileTable *>(map[pos]);
                auto dish = ContainerHolder(ContainerKind::Plate, Mixture());
                dish.setRespawnPoint(std::make_pair(x, y));
                table->put(dish);
            } else {
                throw std::runtime_error("Unknown entity kind");
            }
        }

        in.close();
    }

    void step() {
        for (auto &i : updateList) {
            i->update();
        }
        world->Step(1.0f / FPS, 6, 2);
        for (auto &i : updateList) {
            i->lateUpdate();
        }
        orderManager.step();
        entityManager.step();
    }

    const b2World *getWorld() { return world; }

    Tile *getTile(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return nullptr;
        }
        return map[x + y * width];
    }

    const std::vector<Player *> &getPlayers() { return players; }
    const std::vector<Tile *> &getTiles() { return map; }
    const std::vector<Recipe> &getRecipes() { return recipes; }
    const std::vector<Order> &getOrders() { return orderManager.getOrders(); }

    void move(int playerId, b2Vec2 direction) {
        players[playerId]->move(direction);
    }
    void interact(int playerId, int x, int y) {
        players[playerId]->interact(getTile(x, y));
    }
    void putOrPick(int playerId, int x, int y) {
        players[playerId]->putOrPick(getTile(x, y));
    }

    friend class GuiManager;

    OrderManager orderManager;
    EntityManager entityManager;

  protected:
    b2World *world;

    int width;
    int height;
    std::vector<Player *> players;
    std::vector<Tile *> map;
    std::vector<Recipe> recipes;

    std::vector<IUpdatable *> updateList;

    void addPlayer(int x, int y) {
        auto player = new Player();
        player->setSpawnPoint(b2Vec2(x + 0.5f, y + 0.5f));
        player->initB2(world);
        player->setLevelManager(this);

        players.push_back(player);
        updateList.push_back(player);
    }

    void addTile(int i, TileKind kind) {
        map[i] = CreateTile(kind);
        map[i]->setPos(b2Vec2(i % width, i / width));
        map[i]->initB2(world);
        map[i]->setGameManager(this);

        auto iUpdatable = dynamic_cast<IUpdatable *>(map[i]);
        if (iUpdatable != nullptr) {
            updateList.push_back(iUpdatable);
        }
    }
};
