#ifndef CONFIG_H_
#define CONFIG_H_

constexpr int FPS = 60;

constexpr float PLAYER_RADIUS = 0.35f;
constexpr float PLAYER_ACCELERATION = 8.0f;
constexpr float PLAYER_DECELERATION = 2.0f;
constexpr float PLAYER_FRICTION = 2.0f;
constexpr float PLAYER_MAX_SPEED = 5.0f;
constexpr float PLAYER_EPISILON_SPEED = PLAYER_DECELERATION / FPS * 1.5f;
constexpr int PLAYER_RESPAWN_TIME = 5 * FPS;

constexpr float SCALE = 30;
constexpr float BORDERWIDTHS = 1;

#endif // CONFIG_H_
