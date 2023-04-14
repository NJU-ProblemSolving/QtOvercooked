#pragma once

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

class Mixture {
  public:
    Mixture() {}
    Mixture(const std::string &ingredient) : ingredients({ingredient}) {}
    Mixture(const std::vector<std::string> &ingredients)
        : ingredients(ingredients) {
        std::sort(this->ingredients.begin(), this->ingredients.end());
    }

    bool isEmpty() const { return ingredients.size() == 0; }
    const std::vector<std::string> &getIngredients() { return ingredients; }

    void add(const std::string &ingredient) {
        auto it = std::lower_bound(ingredients.begin(), ingredients.end(),
                                   ingredient);
        ingredients.insert(it, ingredient);
    }

    void put(Mixture &mixture) {
        ingredients.insert(ingredients.end(), mixture.ingredients.begin(),
                           mixture.ingredients.end());
        mixture.ingredients.clear();
        std::sort(ingredients.begin(), ingredients.end());
    }

    std::string toString() const {
        std::string s;
        for (auto &ingredient : ingredients) {
            s += ingredient + " ";
        }
        return s;
    }

    friend bool operator==(const Mixture &lhs, const Mixture &rhs) {
        return lhs.ingredients == rhs.ingredients;
    }

  private:
    std::vector<std::string> ingredients;
};
