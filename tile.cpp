#include "tile.h"

#include "gamemanager.h"

bool TileCuttingBoard::interact() {
    if (containerOnTable.isNull()) {
        return false;
    }

    if (!containerOnTable.isWorking()) {
        for (auto &recipe : levelManager->getRecipes()) {
            if (containerOnTable.matchRecipe(&recipe)) {
                containerOnTable.setRecipe(&recipe);
            }
        }
    }

    if (!containerOnTable.isWorking()) {
        return false;
    }

    containerOnTable.step(tileKind);
    return true;
}

void TileStove::lateUpdate() {
    if (containerOnTable.isNull()) {
        return;
    }

    if (!containerOnTable.isWorking()) {
        for (auto &recipe : levelManager->getRecipes()) {
            if (containerOnTable.matchRecipe(&recipe)) {
                containerOnTable.setRecipe(&recipe);
            }
        }
    }

    if (!containerOnTable.isWorking()) {
        return;
    }

    containerOnTable.step(tileKind);
}
