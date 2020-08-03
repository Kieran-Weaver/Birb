#ifndef GOOSE_FOOD_HPP
#define GOOSE_FOOD_HPP
#include <util/Rect.hpp>
struct Food{
	Food(float x, float y) : hitbox(x, y, 8.f, 8.f){}
	Rect<float> hitbox;
	float replenish = 1.f;
};
#endif
