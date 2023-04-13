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
#include "foodcontainer.h"
#include "interfaces.h"
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
            if (s == "-cut->") {
                containerKind = ContainerKind::None;
                tileKind = TileKind::CuttingBoard;
            } else if (s == "-pod->") {
                containerKind = ContainerKind::Pot;
                tileKind = TileKind::Stove;
            } else if (s == "-pan->") {
                containerKind = ContainerKind::Pan;
                tileKind = TileKind::Stove;
            } else {
                throw "Unknown recipe kind";
            }

            Mixture results;
            while (ss.good()) {
                ss >> s;
                results.add(s);
            }

            recipes.push_back(
                Recipe(ingredients, results, containerKind, tileKind, time));
        }

        in >> width >> height;
        map.resize(width * height);
        in >> std::noskipws;
        for (int i = 0; i < width * height; i++) {
            char kindChar;
            do {
                in >> kindChar;
            } while (kindChar == '\n' || kindChar == '\r');

            switch (kindChar) {
            case 'a':
                addTile(i, kindChar);
                static_cast<TileTable *>(map[i])->setContainer(
                    ContainerKind::Pan);
                break;
            case 'o':
                addTile(i, kindChar);
                static_cast<TileTable *>(map[i])->setContainer(
                    ContainerKind::Pot);
                break;
            default:
                if (kindChar >= 'A' && kindChar <= 'Z') {
                    addTile(i, kindChar);
                    static_cast<TilePantry *>(map[i])->setIngredient(
                        std::string{kindChar});
                } else {
                    addTile(i, kindChar);
                }
                break;
            }
        }
        in >> std::skipws;

        int playerCount;
        in >> playerCount;
        for (int i = 0; i < playerCount; i++) {
            int x, y;
            in >> x >> y;
            addPlayer(x, y);
        }

        in.close();
    }

    void move(int playerId, b2Vec2 direction) {
        players[playerId]->move(direction);
    }

    void step() {
        for (auto &i : updateList) {
            i->update();
        }
        world->Step(1.0f / FPS, 6, 2);
        for (auto &i : updateList) {
            i->lateUpdate();
        }
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

  protected:
    friend class GuiManager;

    b2World *world;

    int width;
    int height;
    std::vector<IUpdatable *> updateList;
    std::vector<Player *> players;
    std::vector<Tile *> map;
    std::vector<Recipe> recipes;

    void addPlayer(int x, int y) {
        auto player = new Player();
        player->setSpawnPoint(b2Vec2(x + 0.5f, y + 0.5f));
        player->initB2(world);
        player->setLevelManager(this);

        players.push_back(player);
        updateList.push_back(player);
    }

    void addTile(int i, char kindChar) {
        auto kind = getTileKind(kindChar);
        map[i] = CreateTile(kind);
        map[i]->setPos(b2Vec2(i % width, i / width));
        map[i]->initB2(world);
        map[i]->setLevelManager(this);

        auto iUpdatable = dynamic_cast<IUpdatable *>(map[i]);
        if (iUpdatable != nullptr) {
            updateList.push_back(iUpdatable);
        }
    }
};
