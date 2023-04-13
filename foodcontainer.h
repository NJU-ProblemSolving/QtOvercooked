#pragma once

#include <string>

#include "enums.h"
#include "interfaces.h"
#include "mixture.h"
#include "recipe.h"

class FoodContainer {
  public:
    FoodContainer(ContainerKind kind = ContainerKind::None,
                  const Mixture &mixture = Mixture())
        : containerKind(kind), mixture(mixture) {}

    FoodContainer(const FoodContainer &other) = delete;
    FoodContainer(FoodContainer &&other) {
        containerKind = other.containerKind;
        mixture = std::move(other.mixture);
        recipe = other.recipe;
        progress = other.progress;
        other.containerKind = ContainerKind::None;
        other.recipe = nullptr;
        other.propertyChanged = true;
    }
    FoodContainer &operator=(const FoodContainer &other) = delete;
    FoodContainer &operator=(FoodContainer &&other) {
        if (this->containerKind != ContainerKind::None ||
            !this->mixture.isEmpty()) {
            throw std::runtime_error("Cannot move to non-empty container");
        }
        containerKind = other.containerKind;
        mixture = std::move(other.mixture);
        recipe = other.recipe;
        progress = other.progress;
        propertyChanged = true;
        other.containerKind = ContainerKind::None;
        other.recipe = nullptr;
        other.propertyChanged = true;
        return *this;
    }

    bool isNull() {
        return containerKind == ContainerKind::None && mixture.isEmpty();
    }
    ContainerKind getContainerKind() { return containerKind; }
    const Mixture &getMixture() { return mixture; }
    void setMixture(const Mixture &mixture) {
        this->mixture = mixture;
        propertyChanged = true;
    }
    bool put(FoodContainer &other) {
        if (this->containerKind == ContainerKind::DirtyDish) {
            return false;
        }
        // self 有容器，且 other 有食材：将 other 倒入 self
        if (this->containerKind != ContainerKind::None &&
            !other.mixture.isEmpty()) {
            if (other.isWorking()) {
                return false;
            }
            this->mixture.put(other.mixture);
            this->recipe = nullptr;
            propertyChanged = true;
            other.propertyChanged = true;
            return true;
        }
        // self 有容器，但 other 没有食材：尝试从 self 倒入 other
        if (this->containerKind != ContainerKind::None &&
            other.mixture.isEmpty()) {
            if (this->mixture.isEmpty() || this->isWorking()) {
                return false;
            }
            if (other.isNull()) {
                return false;
            }
            return other.put(*this);
        }
        assert(this->containerKind == ContainerKind::None);
        // self 没有容器，但 other 有容器：尝试从 self 倒入 other，并将 other
        // 给予 self
        if (other.containerKind != ContainerKind::None) {
            other.put(*this);
            *this = std::move(other);
            return true;
        }
        // self 没有容器，other 也没有容器：在 self 为空的情况下将 other 给予
        // self
        if (!this->mixture.isEmpty()) {
            return false;
        }
        *this = std::move(other);
        return true;
    }

    bool isPropertyChanged() { return propertyChanged; }
    void setPropertyChanged(bool value) { propertyChanged = value; }

    bool isWorking() { return recipe != nullptr; }
    bool matchRecipe(const Recipe *recipe) {
        return containerKind == recipe->containerKind &&
               mixture == recipe->ingredients;
    }
    void setRecipe(const Recipe *recipe) {
        this->recipe = recipe;
        progress = 0;
    }
    float getProgress() { return (float)progress / this->recipe->time; }
    void step(TileKind tileKind) {
        if (tileKind != recipe->tileKind) {
            return;
        }
        progress++;
        propertyChanged = true;
        if (progress >= recipe->time) {
            setMixture(recipe->result);
            progress = 0;
            recipe = nullptr;
        }
    }

    std::string toString() {
        std::string s;
        switch (containerKind) {
        case ContainerKind::None:
            break;
        case ContainerKind::Pan:
            s = "Pan:";
            break;
        case ContainerKind::Pot:
            s = "Pot:";
            break;
        case ContainerKind::Dish:
            s = "Dish:";
            break;
        case ContainerKind::DirtyDish:
            s = "DirtyDish";
            break;
        }
        s += mixture.toString();
        return s;
    }

  protected:
    ContainerKind containerKind;
    Mixture mixture;

    const Recipe *recipe = nullptr;
    int progress = 0;

    bool propertyChanged = true;
};
