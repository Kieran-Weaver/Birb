#ifndef JSONWRITER_HPP
#define JSONWRITER_HPP
#include <rapidjson/fwd.h>
#include <string>
#include <memory>
namespace detail{
	using Writer = rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<char>,
		rapidjson::UTF8<char>, rapidjson::CrtAllocator,
		0>;
	struct WriterDeleter{
		void operator()(Writer* p);
	};
	struct SBDeleter{
		void operator()(rapidjson::StringBuffer *p);
	};
};
class JSONWriter{
public:
	JSONWriter();
	void Key(const std::string& key);
	void Int(int i);
	void StartObject();
	void EndObject();
	void StartArray();
	void EndArray();
	void Reset();
	std::string get();
private:
	std::unique_ptr<detail::Writer, detail::WriterDeleter> internal;
	std::unique_ptr<rapidjson::StringBuffer, detail::SBDeleter> data;
};
#endif
