#pragma once

#include <string>

#include "interfaces.h"
#include "mixture.h"
#include "recipe.h"
#include "enums.h"

class FoodContainer {
  public:
    FoodContainer(ContainerKind kind = ContainerKind::None, const Mixture &mixture = Mixture())
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
        if (this->containerKind != ContainerKind::None || !this->mixture.isEmpty()) {
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

    bool isNull() { return containerKind == ContainerKind::None && mixture.isEmpty(); }
    ContainerKind getContainerKind() { return containerKind; }
    const Mixture &getMixture() { return mixture; }
    void setMixture(const Mixture &mixture) { this->mixture = mixture; propertyChanged = true; }
    bool put(FoodContainer &container) {
        if (this->containerKind == ContainerKind::DirtyDish) {
            return false;
        }
        if (this->containerKind != ContainerKind::None) {
            this->mixture.put(container.mixture);
            this->recipe = nullptr;
            propertyChanged = true;
            container.propertyChanged = true;
            return true;
        }
        if (container.containerKind != ContainerKind::None) {
            container.put(*this);
            *this = std::move(container);
            return true;
        }
        if (!this->mixture.isEmpty()) {
            return false;
        }
        *this = std::move(container);
        return true;
    }

    bool isPropertyChanged() { return propertyChanged; }
    void setPropertyChanged(bool value) { propertyChanged = value; }

    bool isWorking() { return recipe!=nullptr; }
    bool matchRecipe(const Recipe *recipe) {
        return containerKind == recipe->containerKind && mixture == recipe->ingredients;
    }
    void setRecipe(const Recipe *recipe) { this->recipe = recipe;progress=0; }
    float getProgress() { return (float)progress / this->recipe->time; }
    void step(TileKind tileKind) {
        if (tileKind != recipe->tileKind) {
            return;
        }
        progress ++;
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
