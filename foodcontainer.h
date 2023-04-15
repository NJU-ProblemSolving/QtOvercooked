#pragma once

#include <assert.h>
#include <string>

#include "config.h"
#include "enums.h"
#include "mixture.h"
#include "recipe.h"

class FoodContainer {
  public:
    FoodContainer(ContainerKind kind = ContainerKind::None,
                  const Mixture &mixture = Mixture())
        : containerKind(kind), mixture(mixture) {
        if (kind == ContainerKind::DirtyPlates) {
            dirtyPlateCount = 1;
        } else {
            dirtyPlateCount = 0;
        }
    }

    FoodContainer(FoodContainer &) = delete;
    FoodContainer &operator=(const FoodContainer &other) = delete;

    bool isNull() {
        return containerKind == ContainerKind::None && mixture.isEmpty();
    }
    bool isEmpty() { return mixture.isEmpty(); }
    ContainerKind getContainerKind() { return containerKind; }
    const Mixture &getMixture() { return mixture; }
    void setMixture(const Mixture &mixture) { this->mixture = mixture; }

    bool isWorking() { return recipe != nullptr; }
    bool matchRecipe(const Recipe *recipe) {
        return containerKind == recipe->containerKind &&
               mixture == recipe->ingredients;
    }
    void setRecipe(const Recipe *recipe) { this->recipe = recipe; }

    float getProgress() {
        if (progress >= this->recipe->time) {
            return 1.0;
        }
        return (float)progress / this->recipe->time;
    }
    float getOvercookProgress() {
        if (progress <= this->recipe->time) {
            return 0.0;
        }
        if (progress >= this->recipe->time + OVERCOOK_TIME) {
            return 1.0;
        }
        return (float)(progress - this->recipe->time) / (OVERCOOK_TIME);
    }
    float calcPriceFactor() {
        if (collided) {
            return 0.7;
        }
        if (overcooked) {
            return 0.8;
        }
        return 1.0;
    }

    void setRespawnPoint(std::pair<int, int> point) { respawnPoint = point; }
    std::pair<int, int> getRespawnPoint() { return respawnPoint; }

    void setCollided() { collided = true; }

    void removeOnePlate() {
        assert(this->containerKind == ContainerKind::DirtyPlates);
        dirtyPlateCount -= 1;
        if (dirtyPlateCount == 0) {
            containerKind = ContainerKind::None;
            recipe = nullptr;
        }
    }

    // 将 other 倒入 self
    bool directPut(FoodContainer &other) {
        if (other.containerKind == ContainerKind::DirtyPlates) {
            if (this->containerKind != ContainerKind::DirtyPlates) {
                return false;
            }
            this->dirtyPlateCount += other.dirtyPlateCount;
            other.dirtyPlateCount = 0;
            other.containerKind = ContainerKind::None;
            return true;
        }
        assert(this->containerKind != ContainerKind::None);
        if (other.mixture.isEmpty()) {
            return false;
        }

        // 若是切菜，拒绝混合
        if (this->recipe != nullptr &&
            this->recipe->tileKind == TileKind::ChoppingStation) {
            return false;
        }
        if (other.recipe != nullptr &&
            other.recipe->tileKind == TileKind::ChoppingStation) {
            return false;
        }
        // 若菜品做到一半，而自己为空，拒绝取出
        if (this->isEmpty() && other.recipe != nullptr &&
            other.progress < other.recipe->time) {
            return false;
        }
        // 若烹饪方式不同，拒绝混合
        if (this->recipe != nullptr && other.recipe != nullptr &&
            this->recipe->tileKind != other.recipe->tileKind) {
            return false;
        }

        // 计算混合后的烹饪进度
        if (this->containerKind == ContainerKind::Plate) {
            this->progress = 0;
            other.progress = 0;
        }
        if (this->recipe != nullptr) {
            if (this->progress > this->recipe->time) {
                if (!this->recipe->ingredients.isEmpty()) {
                    setMixture(this->recipe->ingredients);
                }
                this->progress = this->recipe->time;
            }
        } else {
            assert(progress == 0);
        }
        if (other.recipe != nullptr) {
            if (other.progress > other.recipe->time) {
                other.progress = other.recipe->time;
            }
        } else {
            assert(other.progress == 0);
        }

        this->mixture.put(other.mixture);
        this->recipe = nullptr;
        this->progress = (this->progress + other.progress) / 2;
        this->overcooked |= other.overcooked;
        this->collided |= other.collided;
        other.recipe = nullptr;
        other.progress = 0;
        other.overcooked = false;
        other.collided = false;
        return true;
    }

    bool step(TileKind tileKind) {
        if (tileKind != recipe->tileKind) {
            return false;
        }
        progress++;
        if (progress == recipe->time) {
            if (!recipe->result.isEmpty()) {
                setMixture(recipe->result);
            }
            if (recipe->tileKind == TileKind::ChoppingStation || recipe->tileKind == TileKind::Sink) {
                recipe = nullptr;
                progress = 0;
                return true;
            }
        }
        if (progress > recipe->time + OVERCOOK_TIME) {
            overcooked = true;
        }
        return false;
    }

    std::string toString() {
        std::string s;
        if (overcooked) {
            s += "*";
        }
        if (collided) {
            s += "@";
        }
        switch (containerKind) {
        case ContainerKind::None:
            break;
        case ContainerKind::Pan:
            s += "Pan:";
            break;
        case ContainerKind::Pot:
            s += "Pot:";
            break;
        case ContainerKind::Plate:
            s += "Plate:";
            break;
        case ContainerKind::DirtyPlates:
            s += "DirtyPlatesX" + std::to_string(dirtyPlateCount);
            break;
        }
        s += mixture.toString();
        return s;
    }

  protected:
    std::pair<int, int> respawnPoint;

    ContainerKind containerKind;
    Mixture mixture;

    const Recipe *recipe = nullptr;
    int progress = 0;

    int dirtyPlateCount = 0;

    bool overcooked = false;
    bool collided = false;
};

class ContainerHolder {
  public:
    ContainerHolder() { container = nullptr; }
    ContainerHolder(ContainerKind kind, const Mixture &mixture) {
        container = new FoodContainer(kind, mixture);
    }

    ContainerHolder(const ContainerHolder &other) = delete;
    ContainerHolder(ContainerHolder &&other) {
        if (container != nullptr) {
            delete container;
        }
        container = other.container;
        other.container = nullptr;
        propertyChanged = true;
        other.propertyChanged = true;
    }

    ContainerHolder &operator=(const ContainerHolder &other) = delete;
    ContainerHolder &operator=(ContainerHolder &&other) {
        if (container != nullptr) {
            delete container;
        }
        container = other.container;
        other.container = nullptr;
        propertyChanged = true;
        other.propertyChanged = true;
        return *this;
    }

    ~ContainerHolder() {
        if (container != nullptr) {
            delete container;
        }
    }

    bool isNull() { return container == nullptr || container->isNull(); }
    bool isEmpty() { return container == nullptr || container->isEmpty(); }

    ContainerKind getContainerKind() { return container->getContainerKind(); }
    const Mixture &getMixture() { return container->getMixture(); }
    void setMixture(const Mixture &mixture) {
        container->setMixture(mixture);
        propertyChanged = true;
    }

    bool isWorking() { return !isNull() && container->isWorking(); }
    bool matchRecipe(const Recipe *recipe) {
        return container->matchRecipe(recipe);
    }
    void setRecipe(const Recipe *recipe) {
        container->setRecipe(recipe);
        propertyChanged = true;
    }

    float getProgress() { return container->getProgress(); }
    float getOvercookProgress() { return container->getOvercookProgress(); }
    float calcPriceFactor() { return container->calcPriceFactor(); }

    void setRespawnPoint(std::pair<int, int> point) {
        container->setRespawnPoint(point);
    }
    std::pair<int, int> getRespawnPoint() {
        return container->getRespawnPoint();
    }

    void setCollided() {
        container->setCollided();
        propertyChanged = true;
    }

    void removeOnePlate() {
        container->removeOnePlate();
        propertyChanged = true;
    }

    bool step(TileKind tileKind) {
        propertyChanged = true;
        return container->step(tileKind);
    }

    bool isPropertyChanged() {
        bool ret = propertyChanged;
        propertyChanged = false;
        return ret;
    }

    std::string toString() {
        if (isNull())
            return "";
        return container->toString();
    }

    bool put(ContainerHolder &other) {
        if (other.isNull()) {
            return false;
        }
        if (this->isNull()) {
            *this = std::move(other);
            return true;
        }

        // 倾倒规则

        // 若 this 上存在容器，则会将 other 的物品（食材或容器中的内容）
        // 全部倒入 this 上的容器中。不存在只倾倒一部分的情况。
        if (container->getContainerKind() != ContainerKind::None &&
            !other.container->isEmpty()) {
            auto res = container->directPut(*other.container);
            propertyChanged = true;
            other.propertyChanged = true;
            return res;
        }

        // 若 this 上存在容器，且 other 是空容器时，
        // 将 this 倒入 other 中。
        if (container->getContainerKind() != ContainerKind::None &&
            !container->isEmpty() && other.container->isEmpty()) {
            auto res = other.container->directPut(*container);
            propertyChanged = true;
            other.propertyChanged = true;
            return res;
        }

        // 若 this 上不存在容器（为单个食材），但是 other 上存在容器，
        // 则会将 this 中的食材放入 other 的容器中，再将容器给 this。
        if (other.container->getContainerKind() != ContainerKind::None) {
            auto res = other.container->directPut(*container);
            *this = std::move(other);
            return res;
        }

        auto res = container->directPut(*other.container);
        propertyChanged = true;
        other.propertyChanged = true;
        return res;
    }

  protected:
    FoodContainer *container = nullptr;
    bool propertyChanged = false;
};
