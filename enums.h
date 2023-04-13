#pragma once

enum class ContainerKind {
    None,
    Pan,
    Pot,
    Dish,
    DirtyDish,
};

enum class TileKind {
    None,
    Void,
    Floor,
    Wall,
    Table,
    CuttingBoard,
    ServingHatch,
    WashBowl,
    Stove,
    Trashbin,
    DishTable,
    Pantry,
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
    case 'a':
        return TileKind::Stove;
    case 'o':
        return TileKind::Stove;
    case 't':
        return TileKind::Trashbin;
    case 'd':
        return TileKind::DishTable;
    default:
        if (kindChar >= 'A' && kindChar <= 'Z') {
            return TileKind::Pantry;
        } else {
            throw "Unknown tile kind";
        }
    }
}

inline char getAbbrev(TileKind kind) {
    switch (kind) {
    case TileKind::CuttingBoard:
        return 'c';
    case TileKind::ServingHatch:
        return 's';
    case TileKind::WashBowl:
        return 'w';
    case TileKind::Stove:
        return 'a';
    case TileKind::Trashbin:
        return 't';
    case TileKind::DishTable:
        return 'd';
    default:
        if (kind >= TileKind::Pantry) {
            return 'p';
        } else {
            throw "Unknown tile kind";
        }
    }
}
