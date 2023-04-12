#ifndef GAMESCENE_H_
#define GAMESCENE_H_

#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMap>

class GameScene final : public QGraphicsScene {
    Q_OBJECT

  public:
    GameScene() {}
    ~GameScene() {}
    GameScene(const GameScene &) = delete;
    GameScene &operator=(const GameScene &) = delete;

  public:
    bool getKey(Qt::Key key) { return keyTable[key]; }
    bool getKeyDown(Qt::Key key) { return keyDownTable[key]; }
    bool getKeyUp(Qt::Key key) { return keyUpTable[key]; }

    // Require clearing keyDownArray and keyUpArray after update()
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

#endif // GAMESCENE_H_
