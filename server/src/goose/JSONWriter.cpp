#include <goose/JSONWriter.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
namespace detail{
	void WriterDeleter::operator()(detail::Writer* p){
		delete p;
	}
	void SBDeleter::operator()(rapidjson::StringBuffer* p){
		delete p;
	}
}
JSONWriter::JSONWriter(){
	data.reset(new rapidjson::StringBuffer);
	internal.reset(new detail::Writer(*data));
}
void JSONWriter::Int(int i){
	this->internal->Int(i);
}
void JSONWriter::StartObject(){
	this->internal->StartObject();
}
void JSONWriter::EndObject(){
	this->internal->EndObject();
}
void JSONWriter::StartArray(){
	this->internal->StartArray();
}
void JSONWriter::EndArray(){
	this->internal->EndArray();
}
void JSONWriter::Key(const std::string& key){
	this->internal->Key(key.c_str());
}
void JSONWriter::Reset(){
	this->data->Clear();
	this->internal->Reset(*data);
}
std::string JSONWriter::get(){
	return data->GetString();
}
