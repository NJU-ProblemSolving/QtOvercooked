#include "recipe.h"

Recipe GeneralCookingRecipe(Mixture(), Mixture(), ContainerKind::None,
                         TileKind::Stove, GENERAL_COOKING_TIME);

Recipe PlateWashingRecipe(Mixture(), Mixture(), ContainerKind::DirtyPlates,
                         TileKind::Sink, PLATE_WASHING_TIME);
