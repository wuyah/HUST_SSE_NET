#include "parser.h"
#include "utils.h"

// 解析以太网帧
void parse_ethernet(const u_char *packet, int length, xdb_searcher_t& searcher)
{
    // 以太网帧头部长度为14字节
    const int EthernetHeaderLength = 14;
    if (length < EthernetHeaderLength)
    {
        std::cout << "Invalid Ethernet frame (too short)" << std::endl;
        return;
    }

    // 读取目的MAC地址和源MAC地址
    const u_char *dest_mac = packet;
    const u_char *src_mac = packet + 6;

    // 解析以太网帧类型
    int ethertype = (packet[12] << 8) | packet[13];

    // 打印以太网帧信息
    std::cout << "Ethernet frame: dest=" << std::hex << std::setw(2) << std::setfill('0') << (int)dest_mac[0]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)dest_mac[1]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)dest_mac[2]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)dest_mac[3]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)dest_mac[4]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)dest_mac[5]
              << ", src=" << std::hex << std::setw(2) << std::setfill('0') << (int)src_mac[0]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)src_mac[1]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)src_mac[2]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)src_mac[3]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)src_mac[4]
              << ":" << std::hex << std::setw(2) << std::setfill('0') << (int)src_mac[5]
              << ", type=0x" << std::hex << std::setw(4) << std::setfill('0') << ethertype << std::endl;

    // 根据以太网帧类型进行下一步解析
    switch (ethertype)
    {
    case 0x0800: // IPv4
        parse_ipv4(packet + EthernetHeaderLength, length - EthernetHeaderLength, searcher);
        break;
    case 0x0806: // ARP
        std::cout << "ARP frame" << std::endl;
        break;
    case 0x86dd: // IPv6
        parse_ipv6(packet + EthernetHeaderLength, length - EthernetHeaderLength);
        break;
    default:
        std::cout << "Unknown Ethernet frame type" << std::endl;
        break;
    }
}

// 解析IPv4报文
void parse_ipv4(const u_char *packet, int length, xdb_searcher_t &searcher)
{
    // IPv4报文头部长度为20字节
    const int IPv4HeaderLength = 20;
    if (length < IPv4HeaderLength)
    {
        std::cout << "Invalid IPv4 packet (too short)" << std::endl;
        return;
    }
    // 读取IPv4头部信息
    int header_length = (packet[0] & 0x0f) * 4;
    const u_char *src_ip = packet + 12;
    const u_char *dest_ip = packet + 16;

    // 解析协议类型
    int protocol = packet[9];

    // 打印IPv4报文信息
    std::cout << "IPv4 packet: src=" << std::dec << (int)src_ip[0] << "." << (int)src_ip[1] << "." << (int)src_ip[2] << "." << (int)src_ip[3]
              << ", dest=" << (int)dest_ip[0] << "." << (int)dest_ip[1] << "." << (int)dest_ip[2] << "." << (int)dest_ip[3]
              << ", protocol=" << protocol << std::endl;
    
    // 查找ipv4地理地址
    char region_buffer[256];
    std::string src_ip_s = std::to_string((int)src_ip[0]) + "." + std::to_string( (int)src_ip[1]) + "." + std::to_string((int)src_ip[2]) + "." + std::to_string((int)src_ip[3]);
    std::string dest_ip_s = std::to_string((int)dest_ip[0]) + "." + std::to_string( (int)dest_ip[1]) + "." + std::to_string((int)dest_ip[2]) + "." + std::to_string((int)dest_ip[3]);
    
    // search src_ip
    int err = xdb_search_by_string(&searcher, src_ip_s.c_str(), region_buffer, sizeof(region_buffer));
    if (err != 0)
        printf("failed search(%s) with errno=%d\n", src_ip_s.c_str(), err);
    else
        printf("{region: %s}", region_buffer);

    // update geo data
    std::string region_str(region_buffer);
    std::string province_str;
    extract_province(region_str, province_str);
    if(province_str != inner_ip && province_str != none_ip) {
        update_json(province_str);
    }
    // search dest_ip
    err = xdb_search_by_string(&searcher, dest_ip_s.c_str(), region_buffer, sizeof(region_buffer));
    if (err != 0)
        printf("failed search(%s) with errno=%d\n", dest_ip_s.c_str(), err);
    else
        printf("{region: %s}", region_buffer);

    // update 
    region_str.assign(region_buffer);
    extract_province(region_str, province_str);
    if(province_str != inner_ip && province_str != none_ip) {
        update_json(province_str);
    }
    // 根据协议类型进行下一步解析
    switch (protocol)
    {
    case IPPROTO_TCP:
        parse_tcp(packet + header_length, length - header_length);
        break;
    case IPPROTO_UDP:
        parse_udp(packet + header_length, length - header_length);
        break;
    default:
        std::cout << "Undefined IPv4 protocol type" << std::endl;
        break;
    }
}

// 解析TCP报文
void parse_tcp(const u_char *packet, int length)
{
    // TCP报文头部长度为20字节
    const int TCPHeaderLength = 20;
    if (length < TCPHeaderLength)
    {
        std::cout << "Invalid TCP packet (too short)" << std::endl;
        return;
    } 

    const char* tcp_header = (const char*)packet;
    USHORT src_port = ntohs(*(USHORT*)&tcp_header[0]);
    USHORT dest_port = ntohs(*(USHORT*)&tcp_header[2]);
    ULONG sequence_number = ntohl(*(ULONG*)&tcp_header[4]);
    ULONG ack_number = ntohl(*(ULONG*)&tcp_header[8]);
    BYTE data_offset = (tcp_header[12] >> 4) & 0x0F;
    BYTE flags = tcp_header[13];
    USHORT window_size = ntohs(*(USHORT*)&tcp_header[14]);
    USHORT checksum = ntohs(*(USHORT*)&tcp_header[16]);
    USHORT urgent_pointer = ntohs(*(USHORT*)&tcp_header[18]);

    // 打印TCP报文信息
    std::cout << "TCP packet: src_port=" << std::dec << src_port 
              << ", dest_port=" << dest_port 
              << ", sequence_number=" << sequence_number 
              << ", ack_number=" << ack_number 
              << ", data_offset=" << (int)data_offset 
              << ", flags=" << std::hex << (int)flags 
              << ", window_size=" << window_size 
              << ", checksum=" << checksum 
              << ", urgent_pointer=" << urgent_pointer << std::dec << std::endl;
}

// 解析UDP报文
void parse_udp(const u_char *packet, int length)
{
    // UDP报文头部长度为8字节
    const int UDPHeaderLength = 8;
    if (length < UDPHeaderLength)
    {
        std::cout << "Invalid UDP packet (too short)" << std::endl;
        return;
    }

    // 读取UDP头部信息
    uint16_t src_port = (packet[0] << 8) | packet[1];
    uint16_t dest_port = (packet[2] << 8) | packet[3];

    // 打印UDP报文信息
    std::cout << "UDP packet: src_port=" << src_port << ", dest_port=" << dest_port << std::endl;
}

// 解析IPv6报文
void parse_ipv6(const u_char *packet, int length)
{
    // IPv6报文头部长度为40字节
    const int IPv6HeaderLength = 40;
    if (length < IPv6HeaderLength)
    {
        std::cout << "Invalid IPv6 packet (too short)" << std::endl;
        return;
    } // 读取IPv6头部信息
    const u_char *src_ip = packet + 8;
    const u_char *dest_ip = packet + 24;

    // 解析协议类型
    int next_header = packet[6];

    // 打印IPv6报文信息
    std::cout << "IPv6 packet: src=";
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)src_ip[i];
        if (i < 7)
            std::cout << ":";
    }
    std::cout << ", dest=";
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)dest_ip[i];
        if (i < 7)
            std::cout << ":";
    }
    std::cout << ", next_header=" << next_header << std::endl;

    // 根据协议类型进行下一步解析
    switch (next_header)
    {
    case IPPROTO_TCP:
        parse_tcp(packet + IPv6HeaderLength, length - IPv6HeaderLength);
        break;
    case IPPROTO_UDP:
        parse_udp(packet + IPv6HeaderLength, length - IPv6HeaderLength);
        break;
    default:
        std::cout << "Unknown IPv6 protocol type" << std::endl;
        break;
    }
}

