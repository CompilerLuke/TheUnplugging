#pragma once

typedef unsigned int Layermask;

constexpr unsigned int game_layer = 2 >> 0;
constexpr unsigned int editor_layer = 2 >> 1;
constexpr unsigned int any_layer = ~0;