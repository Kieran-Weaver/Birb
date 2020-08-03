#include <goose/GameState.hpp>
#include <rapidjson/document.h>
#include <util/Helpers.hpp>
#include <util/PRNG.hpp>
GameState::GameState(const std::string& filename, const std::string& mapname){
	rapidjson::Document document;
	std::string ifsdata = readWholeFile(filename);
	document.Parse(ifsdata.c_str());
	for (auto& type : document["types"].GetArray()){
		Type& typ = this->types.emplace_back();
		typ.atk = type["a"].GetInt();
		typ.def = type["d"].GetInt();
		typ.spd = type["s"].GetInt();
		typ.cost = type["c"].GetInt();
		typ.width = type["w"].GetInt();
		typ.height = type["h"].GetInt();
	}
	this->rng = SeedRNG();
	this->xdist = std::uniform_int_distribution<int>(0, 3992);
	this->ydist = std::uniform_int_distribution<int>(0, 2992);
	this->internalMap = ObjMap(mapname);
}
