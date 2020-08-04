#ifndef GOOSE_MAP_HPP
#define GOOSE_MAP_HPP
#include <vector>
#include <string>
#include <random>
#include <util/Rect.hpp>
#include <rapidjson/document.h>
enum WallType { LWALL=1, RWALL=2, CEIL=4, FLOOR=8, ONEWAY=16  };
struct Surface{
	Rect<float> hitbox;
	int flags;
	Rect<float> getAABB() const{
		return hitbox;
	}
};
struct ObjMap {
	ObjMap() = default;
	ObjMap(const std::string& filename);
	void loadFromFile(const std::string& filename);
	uint32_t addSurface(const Surface& wall);
	Rect<float> bbox;
	std::vector<Surface> surfaces;
};
#endif
