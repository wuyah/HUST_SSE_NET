#pragma once

#include <iostream>
#include <iomanip>
#include <pcap.h>
#include <WinSock2.h>
#include <string>
extern "C" {
    #include "external/xdb_searcher.h"
}

const std::string inner_ip = "内网IP";
const std::string none_ip= "0";

void parse_ipv4(const u_char *packet, int length, xdb_searcher_t &searcher);
void parse_tcp(const u_char *packet, int length);
void parse_udp(const u_char *packet, int length);
void parse_ipv6(const u_char *packet, int length);
void parse_ethernet(const u_char *packet, int length, xdb_searcher_t& searcher);
// void flow_data_record(const char* province);