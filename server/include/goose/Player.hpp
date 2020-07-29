#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <core/MovingEntity.hpp>
#include <core/Timer.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <mutex>
struct Player{
	Player() = default;
	Player(const Player& other) = default;
	Player(int x, int y, std::shared_ptr<ObjMap>& map) : entity(x, y, 32, 32, map){}
	Rect<float> getHitbox();
	Rect<float> getAtkHitbox();
	void Render(rapidjson::Writer<rapidjson::StringBuffer>& writer);
	MovingEntity entity;
	std::string name = "";
	int direction = 1;
	int type = 1;
	float health = 10.f;
	float maxHealth = 10.f;
	int state = 1;
	bool attacked = true;
	Timer attackTimer;
	int ap = 0;
	int totalattack = 0;
	int totalkill = 0;
	std::vector<int> unlocked = {1, 21};
	std::shared_ptr<std::mutex> mtx = std::make_shared<std::mutex>();
};
#endif
