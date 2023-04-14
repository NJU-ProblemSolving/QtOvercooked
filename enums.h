#pragma once

#include <stdexcept>

enum class ContainerKind {
    None,
    Pan,
    Pot,
    Dish,
    Dishes,
    DirtyDishes,
};

enum class TileKind {
    None,
    Void,
    Floor,
    Wall,
    Table,
    Pantry,
    Trashbin,
    CuttingBoard,
    ServingHatch,
    Stove,
    WashBowl,
    DishTable,
    DirtyDishTable,
};

inline TileKind getTileKind(char kindChar) {
    switch (kindChar) {
    case ' ':
        return TileKind::Void;
    case '.':
        return TileKind::Floor;
    case '*':
        return TileKind::Table;
    case 'c':
        return TileKind::CuttingBoard;
    case '$':
        return TileKind::ServingHatch;
    case 'w':
        return TileKind::WashBowl;
    case 's':
        return TileKind::Stove;
    case 't':
        return TileKind::Trashbin;
    case 'd':
        return TileKind::DishTable;
    case 'p':
        return TileKind::Pantry;
    default:
        throw std::runtime_error("Unknown tile kind");
    }
}

inline char getAbbrev(TileKind kind) {
    switch (kind) {
    case TileKind::CuttingBoard:
        return 'c';
    case TileKind::ServingHatch:
        return '$';
    case TileKind::WashBowl:
        return 'w';
    case TileKind::Stove:
        return 's';
    case TileKind::Trashbin:
        return 't';
    case TileKind::DishTable:
        return 'd';
    default:
        if (kind >= TileKind::Pantry) {
            return 'p';
        } else {
            throw std::runtime_error("Unknown tile kind");
        }
    }
}
