#include "controller.h"
#include "gamemanager.h"
#include "mygetopt.h"

std::pair<int, int> parseDirection(std::string direction) {
    int x = 0;
    int y = 0;
    for (auto c : direction) {
        switch (c) {
        case 'L':
            x -= 1;
            break;
        case 'R':
            x += 1;
            break;
        case 'U':
            y -= 1;
            break;
        case 'D':
            y += 1;
            break;
        }
    }
    if (x < -1) x = -1;
    if (x > 1) x = 1;
    if (y < -1) y = -1;
    if (y > 1) y = 1;
    return {x, y};
}

int main(int argc, char* argv[]) {
    const char *levelFile = "level1.txt";
    const char *program = "a.out";
    int o;
    while ((o = getopt(argc, argv, "l:p:")) != -1) {
        switch (o) {
        case 'l':
            levelFile = optarg;
            break;
        case 'p':
            program = optarg;
            break;
        default:
            printf("Unknown commandline argument %c\n", o);
            break;
        }
    }

    auto gameManager = new GameManager();
    gameManager->loadLevel(levelFile);

    auto controller = new CliController(gameManager, program);
    controller->init(levelFile);

    int frame = gameManager->orderManager.getTimeCountdown();
    for (int i = 0; i < frame; i++) {
        auto inputs = controller->requestInputs();
        assert(inputs.size() == gameManager->getPlayers().size());
        for (int i = 0; i < inputs.size(); i++) {
            auto &input = inputs[i];
            auto &player = gameManager->getPlayers()[i];
            if (input.starts_with("Move")) {
                auto direction = input.substr(4);
                auto [x, y] = parseDirection(direction);
                gameManager->move(i, b2Vec2(x, y));
            } else if (input.starts_with("Interact")) {
                auto direction = input.substr(8);
                auto [x, y] = parseDirection(direction);
                x += player->getBody()->GetPosition().x;
                y += player->getBody()->GetPosition().y;
                gameManager->interact(i, x, y);
            } else if (input.starts_with("PutOrPick")) {
                auto direction = input.substr(9);
                auto [x, y] = parseDirection(direction);
                x += player->getBody()->GetPosition().x;
                y += player->getBody()->GetPosition().y;
                gameManager->putOrPick(i, x, y);
            }
        }
        gameManager->step();
    }

    printf("%d\n", gameManager->orderManager.getFund());
}
