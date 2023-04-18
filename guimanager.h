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
        graphicsItem->setPos(0, -0.5 * SCALE);
        picItem = new QGraphicsPixmapItem(graphicsItem);
        picItem->setPos(0, 0.5 * SCALE);
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
            if (foodContainer->getContainerKind() == ContainerKind::Plate) {
                QPixmap image(":/img/images/Plate.png");
                picItem->setPixmap(image.scaled(SCALE, SCALE));
                picItem->setVisible(true);
                graphicsItem->setPlainText(
                    QString::fromStdString(foodContainer->toString()));
            } else if (foodContainer->getContainerKind() ==
                       ContainerKind::Pan) {
                QPixmap image(":/img/images/Pan.PNG");
                picItem->setPos(-SCALE / 4, SCALE / 3);
                picItem->setPixmap(image.scaled(SCALE * 1.5, SCALE * 1.5));
                picItem->setVisible(true);
                graphicsItem->setPlainText(
                    QString::fromStdString(foodContainer->toString()));
            } else if (foodContainer->getContainerKind() ==
                       ContainerKind::Pot) {
                QPixmap image(":/img/images/Pot.PNG");
                picItem->setPos(-SCALE / 2, 0);
                picItem->setPixmap(image.scaled(SCALE * 2, SCALE * 2));
                picItem->setVisible(true);
                graphicsItem->setPlainText(
                    QString::fromStdString(foodContainer->toString()));
            } else {
                graphicsItem->setPlainText(
                    QString::fromStdString(foodContainer->toString()));
                picItem->setVisible(false);
            }
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
    QGraphicsPixmapItem *picItem;
};

class GuiPlayer : public GuiItem {
  public:
    GuiPlayer(Player *player) : player(player) {
        graphicsItem = new QGraphicsEllipseItem(0, 0, PLAYER_RADIUS * 2 * SCALE,
                                                PLAYER_RADIUS * 2 * SCALE);
        graphicsItem->setZValue(10);
        graphicsItem->setBrush(QBrush(QColor("#0066FF")));
        guiFoodContainer =
            new GuiFoodContainer(player->getOnHand(), graphicsItem);
    }

    void update() override {
        auto pos = player->getBody()->GetPosition();
        graphicsItem->setPos((pos.x - PLAYER_RADIUS) * SCALE,
                             (pos.y - PLAYER_RADIUS) * SCALE);
        guiFoodContainer->update();
        guiFoodContainer->getGraphicsItem()->setPos(-0.15f * SCALE, -SCALE);
        graphicsItem->setVisible(player->getRespawnCountdown() == 0);
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
            graphicsItem->setBrush(QBrush(Qt::white));
            graphicsItem->setPen(Qt::NoPen);
            graphicsItem->setZValue(0);
            break;
        case TileKind::Floor:
            graphicsItem->setBrush(QBrush(QColor("#FFFFCC")));
            graphicsItem->setPen(QPen(Qt::lightGray));
            graphicsItem->setZValue(0);
            break;
        case TileKind::Table:
            graphicsItem->setBrush(QBrush(QColor("#99CC00")));
            graphicsItem->setPen(QPen(Qt::black));
            graphicsItem->setZValue(1);
            break;
        case TileKind::ServiceWindow:
            graphicsItem->setBrush(QBrush(QColor("#FFC0CB")));
            graphicsItem->setPen(QPen(Qt::black));
            graphicsItem->setZValue(1);
            break;
        default:
            graphicsItem->setPen(QPen(Qt::black));
            graphicsItem->setZValue(1);
            auto text =
                new QGraphicsTextItem(QString(getAbbrev(kind)), graphicsItem);
            auto pic = new QGraphicsPixmapItem(graphicsItem);
            if (kind == TileKind::IngredientBox) {
                auto pantry = static_cast<TileIngredientBox *>(tile);
                std::string label = pantry->getIngredient();
                if (label == "fish") {
                    QPixmap image(":/img/images/fish.png");
                    pic->setPixmap(image.scaled(SCALE, SCALE));
                    pic->setPos(0, 0);
                } else if (label == "kelp") {
                    QPixmap image(":/img/images/kelp.png");
                    pic->setPixmap(image.scaled(SCALE, SCALE));
                    pic->setPos(0, 0);
                } else if (label == "rice") {
                    QPixmap image(":/img/images/rice.png");
                    pic->setPixmap(image.scaled(SCALE, SCALE));
                    pic->setPos(0, 0);
                } else {
                    assert(0);
                }
            } else if (kind == TileKind::ChoppingStation) {
                QPixmap image(":/img/images/ChoppingStation.png");
                pic->setPixmap(image.scaled(SCALE, SCALE));
                pic->setPos(0, 0);
            } else if (kind == TileKind::Stove) {
                QPixmap image(":/img/images/Stove.png");
                pic->setPixmap(image.scaled(SCALE, SCALE));
                pic->setPos(0, 0);
            } else if (kind == TileKind::Trashbin) {
                QPixmap image(":/img/images/Trashbin.png");
                pic->setPixmap(image.scaled(SCALE, SCALE));
                pic->setPos(0, 0);
            } else if (kind == TileKind::PlateReturn) {
                QPixmap image(":/img/images/PlateReturn.png");
                pic->setPixmap(image.scaled(SCALE, SCALE));
                pic->setPos(0, 0);
            } else if (kind == TileKind::Sink) {
                QPixmap image(":/img/images/Sink.png");
                pic->setPixmap(image.scaled(SCALE, SCALE));
                pic->setPos(0, 0);
            } else if (kind == TileKind::PlateRack) {
                QPixmap image(":/img/images/PlateRack.png");
                pic->setPixmap(image.scaled(SCALE, SCALE));
                pic->setPos(0, 0);
            } else if (kind == TileKind::ServiceWindow) {

                text->setPos(0, 0.4 * SCALE);
            }
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
        ss << "Frame: " << orderManager->getFrame() << "/"
           << orderManager->getFrame() + orderManager->getTimeCountdown()
           << '\n';
        ss << "Fund: " << orderManager->getFund() << '\n';
        for (auto &order : orderManager->getOrders()) {
            ss << order.time << ' ' << order.price << ' '
               << order.mixture.toString();
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
    GuiManager(GameManager *gameManager) : gameManager(gameManager) {}
    ~GuiManager() {}
    GuiManager(const GuiManager &) = delete;
    GuiManager &operator=(const GuiManager &) = delete;

    void init() {
        for (auto &player : gameManager->getPlayers()) {
            auto guiPlayer = new GuiPlayer(player);
            addItem(guiPlayer->getGraphicsItem());
            guiItems.push_back(guiPlayer);
        }
        for (auto &tile : gameManager->getTiles()) {
            auto guiTile = new GuiTile(tile);
            addItem(guiTile->getGraphicsItem());
            guiItems.push_back(guiTile);
        }
        auto guiOrder = new GuiOrder(&gameManager->orderManager);
        addItem(guiOrder->getGraphicsItem());
        guiItems.push_back(guiOrder);
    }

    void step() { clearKeys(); }

  public slots:
    void updateItem() {
        for (auto &guiItem : guiItems) {
            guiItem->update();
        }
        this->update();
    }

  private:
    GameManager *gameManager;
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
