#include <fstream>
#include <rapidjson/stringbuffer.h>
#include <core/Map.hpp>
#include <sstream>
#include <util/PRNG.hpp>
#include <util/Helpers.hpp>
#ifndef NDEBUG
#include <iostream>
#endif
ObjMap::ObjMap(const std::string& filename){
	this->loadFromFile(filename);
}
void ObjMap::loadFromFile(const std::string& filename){
	std::string jsondata = readWholeFile(filename);
	rapidjson::Document document;
	rapidjson::ParseResult result = document.Parse(jsondata.c_str());
	if (!result) {
#ifndef NDEBUG
		std::cerr << "ERROR: Invalid JSON file " << filename << std::endl; 
#endif
		std::exit(1);
	}
	const rapidjson::Value& surfacesNode = document["surfaces"];
	for (auto& surfaceNode : surfacesNode.GetArray()){
		Surface s;
		s.hitbox = {surfaceNode["x"].GetFloat(),surfaceNode["y"].GetFloat(),surfaceNode["w"].GetFloat(),surfaceNode["h"].GetFloat()};
		s.flags = surfaceNode["f"].GetInt() & 0x1F;
		if (s.flags == 0){
#ifndef NDEBUG
			std::cerr << " Warning: Invalid surface flags: " << surfaceNode["f"].GetInt() << std::endl;
#endif
			s.flags = 1;
		}
		surfaces.emplace_back(s);
	}
	this->bbox.left = document["x"].GetInt();
	this->bbox.top = document["y"].GetInt();
	this->bbox.width = document["w"].GetInt();
	this->bbox.height = document["h"].GetInt();
}
