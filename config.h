#pragma once

constexpr int FPS = 60;

constexpr float PLAYER_RADIUS = 0.35f;
constexpr float PLAYER_ACCELERATION = 8.0f;
constexpr float PLAYER_DECELERATION = 10.0f;
constexpr float PLAYER_FRICTION = 2.0f;
constexpr float PLAYER_MAX_SPEED = 5.0f;
constexpr float PLAYER_EPISILON_SPEED = PLAYER_DECELERATION / FPS * 1.5f;
constexpr float PLAYER_INTERACT_DISTANCE = 1.3f;

constexpr int PLAYER_RESPAWN_TIME = 5 * FPS;
constexpr int OVERCOOK_TIME = 10 * FPS;
constexpr int GENERAL_COOK_TIME = 10 * FPS;
constexpr int DISH_RECYCLE_DELAY = 5 * FPS;

constexpr float SCALE = 30;
constexpr float BORDERWIDTHS = 1;
