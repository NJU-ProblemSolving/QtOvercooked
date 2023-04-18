#include "tile.h"

#include "gamemanager.h"
#include "recipe.h"

bool TileChoppingStation::interact() {
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
            if (containerOnTable.matchRecipe(&recipe) &&
                recipe.tileKind == tileKind) {
                containerOnTable.setRecipe(&recipe);
                break;
            }
        }
    }

    if (!containerOnTable.isWorking()) {
        if (!containerOnTable.isEmpty() &&
            (containerOnTable.getContainerKind() == ContainerKind::Pan ||
             containerOnTable.getContainerKind() == ContainerKind::Pot)) {
            containerOnTable.setRecipe(&GeneralCookingRecipe);
        } else {
            return;
        }
    }

    containerOnTable.step(tileKind);
}

bool TileServiceWindow::put(ContainerHolder &container) {
    if (container.getContainerKind() != ContainerKind::Plate) {
        return false;
    }

    auto dish = container.getMixture();
    int price = gameManager->orderManager.serveDish(dish);
    float factor = container.calcPriceFactor();
    gameManager->orderManager.addFund(price * factor);

    Tile *plateReturn = nullptr;
    Tile *sink = nullptr;
    for (auto &tile : gameManager->getTiles()) {
        if (tile->getTileKind() == TileKind::Sink) {
            sink = tile;
        }
        if (tile->getTileKind() == TileKind::PlateReturn) {
            plateReturn = tile;
        }
    }
    if (sink != nullptr) {
        auto dish = ContainerHolder(ContainerKind::DirtyPlates, Mixture());
        dish.setRespawnPoint(
            std::make_pair(plateReturn->getPos().x, plateReturn->getPos().y));
        gameManager->entityManager.scheduleRespawn(std::move(dish),
                                                   PLATE_RETURN_DELAY);
    } else {
        auto dish = ContainerHolder(ContainerKind::Plate, Mixture());
        dish.setRespawnPoint(
            std::make_pair(plateReturn->getPos().x, plateReturn->getPos().y));
        gameManager->entityManager.scheduleRespawn(std::move(dish),
                                                   PLATE_RETURN_DELAY);
    }

    ContainerHolder nullContainer;
    nullContainer.put(container);
    return true;
}

bool TileSink::interact() {
    if (containerOnTable.isNull() ||
        containerOnTable.getContainerKind() != ContainerKind::DirtyPlates) {
        return false;
    }

    if (!containerOnTable.isWorking()) {
        containerOnTable.setRecipe(&PlateWashingRecipe);
    }

    if (containerOnTable.step(tileKind)) {
        Tile *rack = nullptr;
        for (auto &tile : gameManager->getTiles()) {
            if (tile->getTileKind() == TileKind::PlateRack) {
                rack = tile;
            }
        }
        assert(rack != nullptr);
        auto dish = ContainerHolder(ContainerKind::Plate, Mixture());
        dish.setRespawnPoint(
            std::make_pair(rack->getPos().x, rack->getPos().y));
        gameManager->entityManager.scheduleRespawn(std::move(dish), 1);
        containerOnTable.removeOnePlate();
    }
    return true;
}
