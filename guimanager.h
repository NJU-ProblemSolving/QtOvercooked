#pragma once

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMap>

#include "config.h"
#include "foodcontainer.h"
#include "gamemanager.h"
#include "tile.h"

class GuiItem {
  public:
    GuiItem() {}
    virtual void update() = 0;
    virtual QGraphicsItem *getGraphicsItem() = 0;
};

class GuiFoodContainer : public GuiItem {
  public:
    GuiFoodContainer(ContainerHolder *foodContainer,
                     QGraphicsItem *parentItem = nullptr)
        : foodContainer(foodContainer) {
        // Set text alignment to center
        graphicsItem = new QGraphicsTextItem(parentItem);
        graphicsItem->setPos(0, 0);
        progress = new QGraphicsRectItem(graphicsItem);
        progress->setPos(0, SCALE * 0.8);
        progress->setBrush(QBrush(Qt::green));
        progress->setPen(Qt::NoPen);
        progress->setZValue(1);
        progress->setVisible(false);
        overcookProgress = new QGraphicsRectItem(graphicsItem);
        overcookProgress->setPos(0, SCALE * 0.8);
        overcookProgress->setBrush(QBrush(Qt::red));
        overcookProgress->setPen(Qt::NoPen);
        overcookProgress->setZValue(2);
        overcookProgress->setVisible(false);
    }

    void update() override {
        if (foodContainer->isPropertyChanged()) {
            graphicsItem->setPlainText(
                QString::fromStdString(foodContainer->toString()));
            if (foodContainer->isWorking()) {
                progress->setVisible(true);
                progress->setRect(0, 0, SCALE * foodContainer->getProgress(),
                                  SCALE * 0.2);
                overcookProgress->setVisible(true);
                overcookProgress->setRect(
                    0, 0, SCALE * foodContainer->getOvercookProgress(),
                    SCALE * 0.2);
            } else {
                progress->setVisible(false);
                overcookProgress->setVisible(false);
            }
        }
    }

    QGraphicsItem *getGraphicsItem() override { return graphicsItem; }

  protected:
    ContainerHolder *foodContainer;
    QGraphicsTextItem *graphicsItem;
    QGraphicsRectItem *progress;
    QGraphicsRectItem *overcookProgress;
};

class GuiPlayer : public GuiItem {
  public:
    GuiPlayer(Player *player) : player(player) {
        graphicsItem = new QGraphicsEllipseItem(0, 0, PLAYER_RADIUS * 2 * SCALE,
                                                PLAYER_RADIUS * 2 * SCALE);
        graphicsItem->setZValue(10);
        guiFoodContainer =
            new GuiFoodContainer(player->getOnHand(), graphicsItem);
    }

    void update() override {
        auto pos = player->getBody()->GetPosition();
        graphicsItem->setPos((pos.x - PLAYER_RADIUS) * SCALE,
                             (pos.y - PLAYER_RADIUS) * SCALE);
        guiFoodContainer->update();
    }

    QGraphicsItem *getGraphicsItem() override { return graphicsItem; }

  protected:
    Player *player;
    QGraphicsEllipseItem *graphicsItem;
    GuiFoodContainer *guiFoodContainer;
};

class GuiTile : public GuiItem {
  public:
    GuiTile(Tile *tile) : tile(tile) {
        graphicsItem = new QGraphicsRectItem(0, 0, SCALE, SCALE);
        graphicsItem->setPos(tile->getPos().x * SCALE,
                             tile->getPos().y * SCALE);
        auto kind = tile->getTileKind();
        switch (kind) {
        case TileKind::Void:
            graphicsItem->setBrush(QBrush(Qt::lightGray));
            graphicsItem->setPen(Qt::NoPen);
            graphicsItem->setZValue(0);
            break;
        case TileKind::Floor:
            graphicsItem->setPen(QPen(Qt::lightGray));
            graphicsItem->setZValue(0);
            break;
        case TileKind::Table:
            graphicsItem->setPen(QPen(Qt::black));
            graphicsItem->setZValue(1);
            break;
        default:
            graphicsItem->setPen(QPen(Qt::black));
            graphicsItem->setZValue(1);
            auto text =
                new QGraphicsTextItem(QString(getAbbrev(kind)), graphicsItem);
            if (kind == TileKind::Pantry) {
                auto pantry = static_cast<TilePantry *>(tile);
                text->setPlainText(
                    QString::fromStdString(pantry->getIngredient()));
            }
            text->setPos(0, 0.4 * SCALE);
            break;
        }
        auto foodContainer = tile->getContainer();
        if (foodContainer != nullptr) {
            guiFoodContainer =
                new GuiFoodContainer(foodContainer, graphicsItem);
        }
    }

    void update() override {
        if (guiFoodContainer != nullptr)
            guiFoodContainer->update();
    }

    QGraphicsItem *getGraphicsItem() override { return graphicsItem; }

  protected:
    Tile *tile;
    QGraphicsRectItem *graphicsItem;
    GuiFoodContainer *guiFoodContainer = nullptr;
};

class GuiOrder : public GuiItem {
    public:
        GuiOrder(OrderManager *orderManager) : orderManager(orderManager) {
            graphicsItem = new QGraphicsTextItem();
            graphicsItem->setPos(-150, 0);
        }
    
        void update() override {
            std::stringstream ss;
            ss << "Frame: " << orderManager->getFrame() << '\n';
            ss << "Fund: " << orderManager->getFund() << '\n';
            ss << "Remain: " << orderManager->getTimeCountdown() << '\n';
            for (auto &order : orderManager->getOrders()) {
            ss << order.time << ' ' << order.price << ' ' << order.mixture.toString();
                ss << '\n';
            }
            graphicsItem->setPlainText(QString::fromStdString(ss.str()));
        }
    
        QGraphicsItem *getGraphicsItem() override { return graphicsItem; }
    
    protected:
        OrderManager *orderManager;
        QGraphicsTextItem *graphicsItem;
};

class GuiManager final : public QGraphicsScene {
    Q_OBJECT

  public:
    GuiManager() {}
    ~GuiManager() {}
    GuiManager(const GuiManager &) = delete;
    GuiManager &operator=(const GuiManager &) = delete;

    void load(std::string filename) {
        gameManager.loadLevel(filename);
        for (auto &player : gameManager.getPlayers()) {
            auto guiPlayer = new GuiPlayer(player);
            addItem(guiPlayer->getGraphicsItem());
            guiItems.push_back(guiPlayer);
        }
        for (auto &tile : gameManager.getTiles()) {
            auto guiTile = new GuiTile(tile);
            addItem(guiTile->getGraphicsItem());
            guiItems.push_back(guiTile);
        }
        auto guiOrder = new GuiOrder(&gameManager.orderManager);
        addItem(guiOrder->getGraphicsItem());
        guiItems.push_back(guiOrder);
    }

  public slots:
    void step() {
        {
            static int lastMove = 1;
            b2Vec2 move(0, 0);
            if (getKey(Qt::Key_A)) {
                move.x -= 1;
                lastMove = 1;
            }
            if (getKey(Qt::Key_D)) {
                move.x += 1;
                lastMove = 2;
            }
            if (getKey(Qt::Key_W)) {
                move.y -= 1;
                lastMove = 3;
            }
            if (getKey(Qt::Key_S)) {
                move.y += 1;
                lastMove = 4;
            }
            if (getKeyDown(Qt::Key_Space)) {
                int x = gameManager.players[0]->getBody()->GetPosition().x;
                int y = gameManager.players[0]->getBody()->GetPosition().y;
                switch (lastMove) {
                case 1:
                    x -= 1;
                    break;
                case 2:
                    x += 1;
                    break;
                case 3:
                    y -= 1;
                    break;
                case 4:
                    y += 1;
                    break;
                }
                gameManager.players[0]->pickOrPut(gameManager.getTile(x, y));
            }
            if (getKeyDown(Qt::Key_J)) {
                int x = gameManager.players[0]->getBody()->GetPosition().x;
                int y = gameManager.players[0]->getBody()->GetPosition().y;
                switch (lastMove) {
                case 1:
                    x -= 1;
                    break;
                case 2:
                    x += 1;
                    break;
                case 3:
                    y -= 1;
                    break;
                case 4:
                    y += 1;
                    break;
                }
                gameManager.players[0]->interact(gameManager.getTile(x, y));
            }
            gameManager.move(0, move);
        }
        for (int i = 1; i < gameManager.players.size(); i++) {
            gameManager.move(i, b2Vec2(0, 0));
        }
        gameManager.step();
        for (auto &guiItem : guiItems) {
            guiItem->update();
        }
        clearKeys();
    }

  private:
    GameManager gameManager;
    std::vector<GuiItem *> guiItems;

  public:
    bool getKey(Qt::Key key) { return keyTable[key]; }
    bool getKeyDown(Qt::Key key) { return keyDownTable[key]; }
    bool getKeyUp(Qt::Key key) { return keyUpTable[key]; }
    void clearKeys() {
        keyDownTable.clear();
        keyUpTable.clear();
    }

  private:
    QMap<int, bool> keyTable, keyDownTable, keyUpTable;
    void keyPressEvent(QKeyEvent *ev) override {
        if (ev->isAutoRepeat())
            return;
        keyTable[ev->key()] = true;
        keyDownTable[ev->key()] = true;
        QGraphicsScene::keyPressEvent(ev);
    }
    void keyReleaseEvent(QKeyEvent *ev) override {
        if (ev->isAutoRepeat())
            return;
        keyTable[ev->key()] = false;
        keyUpTable[ev->key()] = true;
        QGraphicsScene::keyReleaseEvent(ev);
    }
};
