#include "tile.h"

#include "gamemanager.h"

bool TileCuttingBoard::interact() {
    if (containerOnTable.isNull()) {
        return false;
    }

    if (!containerOnTable.isWorking()) {
        for (auto &recipe : gameManager->getRecipes()) {
            if (containerOnTable.matchRecipe(&recipe)) {
                containerOnTable.setRecipe(&recipe);
                break;
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
        for (auto &recipe : gameManager->getRecipes()) {
            if (containerOnTable.matchRecipe(&recipe)) {
                containerOnTable.setRecipe(&recipe);
                break;
            }
        }
    }

    if (!containerOnTable.isWorking()) {
        if (!containerOnTable.isEmpty()) {
            containerOnTable.setRecipe(&GeneralCookRecipe);
        } else {
            return;
        }
    }

    containerOnTable.step(tileKind);
}

bool TileServingHatch::put(ContainerHolder &container) {
    if (container.getContainerKind() != ContainerKind::Dish) {
        return false;
    }

    auto dish = container.getMixture();
    int price = gameManager->orderManager.serveDish(dish);
    float factor = container.calcPriceFactor();
    gameManager->orderManager.addFund(price * factor);

    Tile *dishTable = nullptr;
    Tile *dirtyDishTable = nullptr;
    for (auto &tile : gameManager->getTiles()) {
        if (tile->getTileKind() == TileKind::DirtyDishTable) {
            dirtyDishTable = tile;
        }
        if (tile->getTileKind() == TileKind::DishTable) {
            dishTable = tile;
        }
    }
    if (dirtyDishTable != nullptr) {
        auto dish = ContainerHolder(ContainerKind::DirtyDishes, Mixture());
        dish.setRespawnPoint(std::make_pair(dirtyDishTable->getPos().x, dirtyDishTable->getPos().y));
        gameManager->entityManager.scheduleRespawn(
            std::move(dish),
            DISH_RECYCLE_DELAY);
    } else {
        auto dish = ContainerHolder(ContainerKind::Dish, Mixture());
        dish.setRespawnPoint(std::make_pair(dishTable->getPos().x, dishTable->getPos().y));
        gameManager->entityManager.scheduleRespawn(
            std::move(dish),
            DISH_RECYCLE_DELAY);
    }

    ContainerHolder nullContainer;
    nullContainer.put(container);
    return true;
}
