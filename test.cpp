#define BOOST_ERROR_CODE_HEADER_ONLY
#define BOOST_SYSTEM_NO_LIB

#include <iostream>
#include "black_circle/json_parser_inst.cc"
#include "rapidjson/document.h"
#include "picojson.h"
#include "qi_any.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <time.h>

inline void qi_any_test(const std::stringstream& ss){
  for (int n = 0; n < 1000; n++){
    boost::any v = std::move(qi_any_parse(ss.str()));
    auto a = boost::any_cast<std::vector<boost::any>>(boost::any_cast<std::map<std::string,boost::any>>(v)["entries"]);
    int i, l = a.size();
    for(i = 0; i< l; i++) {
      std::cout << boost::any_cast<std::string>(boost::any_cast<std::map<std::string,boost::any>>(a[i])["title"]) << std::endl;
    }
  }
}

inline void circle_test(const std::stringstream& ss){
  for (int n = 0; n < 1000; n++){
    auto v = circle::json::parse_json(ss.str());
    auto a = boost::get<circle::json::json_array_type>(boost::get<circle::json::json_object_map>(v)["entries"]);
    int i, l = a.size();
    for (i = 0; i < l; i++) {
      std::cout << boost::get<std::string>(boost::get<circle::json::json_object_map>(a[i])["title"]) << std::endl;
    }
  }
}

inline void picojson_test(const std::stringstream& ss){
  for (int n = 0; n < 1000; n++){
    picojson::value v;
    picojson::parse(v,ss.str().begin(),ss.str().end(),nullptr);
    picojson::array a = v.get<picojson::object>()["entries"].get<picojson::array>();
    int i, l = a.size();
    for (i = 0; i < l; i++) {
      std::cout << a[i].get<picojson::object>()["title"].to_str() << std::endl;
    }
  }
}

inline void rapidjson_test(const std::stringstream& ss){
  for (int n = 0; n < 1000; n++){
    rapidjson::Document doc;
    if (doc.Parse<0>(ss.str().c_str()).HasParseError()) {
      std::cout << "parse error" << std::endl;
      return;
    }
    rapidjson::Value& entries = doc["entries"];
    int i, l = entries.Size();
    for (i = 0; i < l; i++) {
      std::cout << entries[rapidjson::SizeType(i)]["title"].GetString() << std::endl;
    }
  }
}

int main(int argc, char* argv[]){
  std::stringstream ss;
  std::ifstream f;
  f.open("hoge.json", std::ios::binary);
  ss << f.rdbuf();
  f.close();
  clock_t r,p,c,j;
  r = clock();
  rapidjson_test(ss);
  r = clock() - r;
  p = clock();
  picojson_test(ss);
  p = clock() - p;
  c = clock();
  circle_test(ss);
  c = clock() - c;
  j = clock();
  circle_test(ss);
  j = clock() - j;
  std::cerr << "rapidjson score: " << r << std::endl;
  std::cerr << "picojson score: " << p << std::endl;
  std::cerr << "qi_any score: "<< j << std::endl;
  std::cerr << "circle score: " << c << std::endl;
  return 0;
}
