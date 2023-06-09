#include "parser.h"
#include "utils.h"

extern "C" {
    #include "external/xdb_searcher.h"
}
// IP过滤规则
const char filter_str[100] = "";

const char db_path[64] = "D:\\Codes\\CN_1\\data\\ip2region.xdb";
char name_buf[100];

void choose_dev();
void set_filter(const char *filter, pcap_t *handle);

int main()
{
    char errbuf[PCAP_ERRBUF_SIZE];

    xdb_content_t *c_buffer;
    xdb_searcher_t searcher;
    // char region_buffer[256], ip_buffer[16], *ip = "1.2.3.4";

    // 从 db_path 加载整个 xdb 的数据。
    c_buffer = xdb_load_content_from_file(db_path);
    if (c_buffer == NULL)
    {
        printf("failed to load xdb content from `%s`\n", db_path);
        return 1;
    }
    else
        std::cout << "load xdb success!" << std::endl;

    // 使用全局的 c_buffer 变量创建一个完全基于内存的 xdb 查询对象
    int err = xdb_new_with_buffer(&searcher, c_buffer);
    if (err != 0)
    {
        printf("failed to create content cached searcher with errcode=%d\n", err);
        return 2;
    }
    else
        std::cout << "create xdb buffer success" << std::endl;

    initailize_json();
    flush_json();
    // 选择设备
    choose_dev();

    // 初始化Npcap
    pcap_t *pcap = pcap_open_live(name_buf, 65536, 1, 1000, errbuf);
    if (pcap == NULL)
    {
        std::cerr << "Failed to initialize Npcap: " << errbuf << std::endl;
        return 1;
    }
    // 设置ip过滤规则
    set_filter(filter_str, pcap);
    pcap_pkthdr header;
    const u_char *packet;
    while (true)
    {
        packet = pcap_next(pcap, &header);
        if (packet == NULL)
            continue;

        // 读取数据包类型
        uint16_t ethernet_type = (packet[12] << 8) | packet[13];
        std::cout << "Ethernet type: " << std::hex << ethernet_type << std::dec << std::endl;

        // Parse Ethernet
        parse_ethernet(packet, header.len, searcher);
    }
    // 关闭Npcap
    pcap_close(pcap);
    return 0;
}

void choose_dev()
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int i = 0;
    char errbuf[PCAP_ERRBUF_SIZE];
    /* 获取本地机器设备列表 */
    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs_ex: %s\n", errbuf);
        exit(1);
    }
    /* 打印列表 */
    for (d = alldevs; d != NULL; d = d->next)
    {
        printf("%d. %s", ++i, d->name);
        // 设备描述
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            std::cout << " (No description available)\n"
                      << std::endl;
        // 设备IP
        if (d->addresses)
        {
            std::cout << "Show device address:" << std::endl;
            pcap_addr_t *addr;
            // 遍历，一个设备可以有多个物理端口，每个物理端口都可以分配一个地址
            for (addr = d->addresses; addr != nullptr; addr = addr->next)
            {
                if (addr->addr != nullptr)
                {
                    std::cout << "Device address family: " << addr->addr->sa_family << std::endl;
                    char ip[INET6_ADDRSTRLEN];
                    switch (addr->addr->sa_family)
                    {
                    // 查看设备的ipv4地址
                    case AF_INET:
                    {
                        sockaddr_in *sin = reinterpret_cast<sockaddr_in *>(addr->addr);
                        inet_ntop(AF_INET, &(sin->sin_addr), ip, INET_ADDRSTRLEN);
                        std::cout << "Device IPv4 address: " << ip << std::endl;
                        break;
                    }
                    // ipv6
                    case AF_INET6:
                    {
                        sockaddr_in6 *sin6 = reinterpret_cast<sockaddr_in6 *>(addr->addr);
                        inet_ntop(AF_INET6, &(sin6->sin6_addr), ip, INET6_ADDRSTRLEN);
                        std::cout << "Device IPv6 address: " << ip << std::endl;
                        break;
                    }
                    default:
                        std::cout << "Unknown address family" << std::endl;
                        break;
                    }
                }
                else
                    std::cout << "addr->addr is nullptr" << std::endl;
            }
        }
        else
            std::cout << " (No addresses!)" << std::endl;
        // 展示是否支持TCP协议
        if (d->addresses != nullptr && d->addresses->next != nullptr &&
            d->addresses->addr->sa_family == AF_INET &&
            (d->flags & PCAP_IF_LOOPBACK) == 0)
        {
            std::cout << "Support TCP!" << std::endl;
        }
    }
    if (i == 0)
    {
        std::cout << "No interfaces found! Make sure WinPcap is installed." << std::endl;
        return;
    }
    // 选择要使用的设备
    // 应当使用5/6号，主要的TCP内容都在其中
    int choice;
    std::cout << "Choose a device (1-" << i << "): ";
    std::cin >> choice;
    if (choice < 1 || choice > i)
    {
        std::cerr << "Invalid choice." << std::endl;
        return;
    }
    // 定位所选设备,链表遍历
    d = alldevs;
    for (int j = 1; j < choice; j++)
    {
        d = d->next;
    }
    const size_t len = strlen(d->name);
    strncpy_s(name_buf, d->name, len);
    // 解析
    pcap_freealldevs(alldevs);
    return;
}

void set_filter(const char *filter, pcap_t *handle)
{
    struct bpf_program fp;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (handle == nullptr)
    {
        std::cerr << "Error opening device: " << errbuf << std::endl;
        return;
    }
    // compile the filter
    if (pcap_compile(handle, &fp, filter, 1, PCAP_NETMASK_UNKNOWN) == -1)
    {
        std::cerr << "Could not compile filter: " << pcap_geterr(handle) << std::endl;
        return;
    }

    // set the filter
    if (pcap_setfilter(handle, &fp) == -1)
    {
        std::cerr << "Could not set filter: " << pcap_geterr(handle) << std::endl;
        return;
    }
}
