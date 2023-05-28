#include "utils.h"
const char *flow_data_path = "D:\\Codes\\CN_1\\data\\flow_data.json";
std::ifstream flow_data;
rapidjson::Document flow_document;

void initailize_json()
{
    std::ifstream ifs(flow_data_path);
    rapidjson::IStreamWrapper isw(ifs);
    flow_document.ParseStream(isw);
    std::cout << "Load json success!" << std::endl;
}

void update_json(const std::string province_name)
{
    rapidjson::Value &data = flow_document["data"];
    for (rapidjson::Value::ValueIterator itr = data.Begin(); itr != data.End(); itr++)
    {
        if (strcmp(std::string((*itr)["name"].GetString()).c_str(), province_name.c_str()) == 0)
        {
            (*itr)["value"].SetInt((*itr)["value"].GetInt() + 1);
            break;
        }
    }
    write_json();
}

void show_json()
{
    rapidjson::Value &data = flow_document["data"];
    for (rapidjson::Value::ValueIterator itr = data.Begin(); itr != data.End(); ++itr)
    {
        std::cout << (*itr)["name"].GetString() << ": " << (*itr)["value"].GetInt() << std::endl;
    }
}

void test_jion(xdb_searcher_t &searcher)
{
    const char *ip = "114.44.227.87";
    char region_buffer[256];
    int err = xdb_search_by_string(&searcher, ip, region_buffer, sizeof(region_buffer));
    if (err != 0)
        printf("failed search(%s) with errno=%d\n", ip, err);
    else
        printf("{region: %s}", region_buffer);
    std::string region_string(region_buffer);
    std::string province_info;
    extract_province(region_string, province_info);
    std::cout << "province:" << province_info << std::endl;
}

void extract_province(const std::string &region_str, std::string &province_str) {
    size_t first_pipe = region_str.find('|');
    if (first_pipe != std::string::npos) {
        size_t second_pipe = region_str.find('|', first_pipe + 1);
        size_t third_pipe = region_str.find('|', second_pipe + 1);

        if (second_pipe != std::string::npos) {
            // 提取第二个 "|" 和第三个 "|" 之间的信息
            province_str = region_str.substr(second_pipe + 1, third_pipe - second_pipe - 1);
        }
    }
}

void write_json() {
    std::ofstream ofs(flow_data_path);
    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    flow_document.Accept(writer);
}

void flush_json() {
    std::cout << "start flush" << std::endl;
    rapidjson::Value &data = flow_document["data"];
    for (rapidjson::Value::ValueIterator itr = data.Begin(); itr != data.End(); itr++)
    {
        
        (*itr)["value"].SetInt(0);
    }
    write_json();
}