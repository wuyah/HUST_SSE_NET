#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <iostream>
#include "parser.h"

extern std::ifstream flow_data;
extern rapidjson::Document flow_document;

void initailize_json();
void show_json();
void update_json(const std::string province_name);
void test_jion(xdb_searcher_t &searcher);
void extract_province(const std::string &region_str, std::string &province_str);
void write_json();
void flush_json();