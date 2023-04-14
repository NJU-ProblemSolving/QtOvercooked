#ifndef RECIPE_H_
#define RECIPE_H_

#include <algorithm>

#include "config.h"
#include "enums.h"
#include "mixture.h"

class Recipe {
  public:
    Recipe() {}
    Recipe(const Mixture &ingredients, const Mixture &result,
           ContainerKind containerKind, TileKind tileKind, int time)
        : ingredients(ingredients), result(result),
          containerKind(containerKind), tileKind(tileKind), time(time) {}

    Mixture ingredients;
    Mixture result;
    ContainerKind containerKind;
    TileKind tileKind;
    int time;
};

extern Recipe GeneralCookRecipe;

#endif
