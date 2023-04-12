#ifndef TILE_KIND_H_
#define TILE_KIND_H_

enum class TileKind {
    Void,
    Floor,
    Wall,
    Table,
    Pantry,
    CuttingBoard,
    WashBowl,
    Stove,
    Trashbin,
};

enum class EntityKind {
    Player,
    Pan,
    Pot,
    Plate,
    Ingredient,
};

#endif // TILE_KIND_H_
