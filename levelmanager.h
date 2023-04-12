#ifndef LEVEL_MANAGER_H_
#define LEVEL_MANAGER_H_

#include <QDebug>
#include <QFile>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QVector>

#include <box2d/box2d.h>

#include "collisionlisener.h"
#include "config.h"
#include "gamescene.h"
#include "interfaces.h"
#include "player.h"
#include "tile.h"

class LevelManager {
  public:
    LevelManager() {}

    void loadLevel(const QString &path) {
        world = new b2World(b2Vec2(0.0f, 0.0f));
        world->SetContactListener(new CollisionListener());

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open file for reading: "
                     << qPrintable(file.errorString());
            return;
        }
        QTextStream in(&file);

        int playerCount;
        in >> playerCount;
        players.resize(playerCount);
        for (int i = 0; i < playerCount; i++) {
            int x, y;
            in >> x >> y;
            players[i] = new Player();
            players[i]->setSpawnPoint(b2Vec2(x + 0.5f, y + 0.5f));
            players[i]->initB2(world);
            players[i]->setLevelManager(this);
            updateList.push_back(players[i]);

            if (scene != nullptr) {
                auto item = new QGraphicsEllipseItem(
                    0, 0, PLAYER_RADIUS * 2 * SCALE, PLAYER_RADIUS * 2 * SCALE);
                item->setZValue(10);
                scene->addItem(item);
                entities.push_back({players[i], item});
            }
        }

        in >> width >> height;
        map.resize(width * height);
        for (int i = 0; i < width * height; i++) {
            char kind;
            do {
                in >> kind;
            } while (kind == '\n' || kind == '\r');
            map[i] = CreateTile(kind);
            map[i]->setPos(b2Vec2(i % width, i / width));
            map[i]->initB2(world);

            if (scene != nullptr) {
                auto item = new QGraphicsRectItem(
                    i % width * SCALE, i / width * SCALE, SCALE, SCALE);
                switch (kind) {
                case '.':
                    item->setPen(QPen(Qt::lightGray));
                    item->setZValue(0);
                    break;
                case '*':
                    item->setPen(QPen(Qt::black));
                    item->setZValue(1);
                    break;
                case ' ':
                    item->setBrush(QBrush(Qt::gray));
                    item->setPen(Qt::NoPen);
                    item->setZValue(0);
                    break;
                }
                scene->addItem(item);
            }
        }
        file.close();
    }

    void step() {
        if (scene != nullptr) {
            {
                b2Vec2 move(0, 0);
                if (scene->getKey(Qt::Key_A)) {
                    move.x -= 1;
                }
                if (scene->getKey(Qt::Key_D)) {
                    move.x += 1;
                }
                if (scene->getKey(Qt::Key_W)) {
                    move.y -= 1;
                }
                if (scene->getKey(Qt::Key_S)) {
                    move.y += 1;
                }
                players[0]->move(move);
            }
            for (int i = 1; i < players.size(); i++) {
                players[i]->move(b2Vec2(0, 0));
            }
        }
        for (auto &i : updateList) {
            i->update();
        }
        world->Step(1.0f / FPS, 6, 2);
        for (auto &i : updateList) {
            i->lateUpdate();
        }
        for (auto &[e, i] : entities) {
            auto pos = e->getBody()->GetPosition();
            i->setPos((pos.x - PLAYER_RADIUS) * SCALE,
                      (pos.y - PLAYER_RADIUS) * SCALE);
        }
    }

    void setScene(GameScene *scene) { this->scene = scene; }

    b2World *getWorld() { return world; }

    Tile *getTile(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return nullptr;
        }
        return map[x + y * width];
    }

  protected:
    b2World *world;

    GameScene *scene;

    int width;
    int height;
    QVector<IUpdatable *> updateList;
    QVector<Player *> players;
    QVector<Tile *> map;
    QVector<QPair<IEntity *, QGraphicsItem *>> entities;
};

#endif // LEVEL_MANAGER_H_
