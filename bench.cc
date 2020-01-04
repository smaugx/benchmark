// export LD_LIBRARY_PATH=/usr/local/lib
// g++ -I msgpack-c/include bench.cc bench.pb.cc   -I ./  -I /usr/local/include/google/protobuf/ -L /usr/local/lib/  -std=c++11 -o bench -Wl,-Bstatic -lprotobuf -Wl,-Bdynamic -pthread

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <random>
#include <mutex>
#include <algorithm>
#include <iomanip>


#include <msgpack.hpp>
#include <bench.pb.h>

#include "xlog.h"
#include "xobject.h"
#include "xthread.h"
#include "xtimer.h"
#include "xcontext.h"
#include "xdata.h"
#include "xpacket.h"
#include "xsocket.h"
#include "xutl.h"



// 循环次数
uint32_t bench_num = 1000000;


const char kHexAlphabet[] = "0123456789abcdef";
const char kHexLookup[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,  8,  9,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15 };

//the following are UBUNTU/LINUX ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

extern "C" int my_create_log_file_cb(const char * log_file_name)
{
    return 0;
}

//return true to prevent to writed into log file,return false as just hook purpose
extern "C" bool my_func_hook_trace_cb(enum_xlog_level level,const char* _module_name,const char* msg,const int msg_length)
{
    return false;
}


std::string HexEncode(const std::string& str) {
    auto size(str.size());
    std::string hex_output(size * 2, 0);
    for (size_t i(0), j(0); i != size; ++i) {
        hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(str[i]) / 16];
        hex_output[j++] = kHexAlphabet[static_cast<unsigned char>(str[i]) % 16];
    }
    return hex_output;
}

std::string HexDecode(const std::string& str) {
    auto size(str.size());
    if (size % 2) return "";

    std::string non_hex_output(size / 2, 0);
    for (size_t i(0), j(0); i != size / 2; ++i) {
        non_hex_output[i] = (kHexLookup[static_cast<int>(str[j++])] << 4);
        non_hex_output[i] |= kHexLookup[static_cast<int>(str[j++])];
    }
    return non_hex_output;
}

uint32_t& rng_seed() {
#ifdef _MSC_VER
    // Work around high_resolution_clock being the lowest resolution clock pre-VS14
    static uint32_t seed([] {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        return static_cast<uint32_t>(t.LowPart);
    }());
#else
    static uint32_t seed(
        static_cast<uint32_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));
#endif
    return seed;
}

std::mt19937& random_number_generator() {
    static std::mt19937 random_number_generator(rng_seed());
    return random_number_generator;
}

std::mutex& random_number_generator_mutex() {
    static std::mutex random_number_generator_mutex;
    return random_number_generator_mutex;
}

template <typename String>
String GetRandomString(size_t size) {
    std::uniform_int_distribution<> distribution(0, 255);
    String random_string(size, 0);
    {
        std::lock_guard<std::mutex> lock(random_number_generator_mutex());
        std::generate(random_string.begin(), random_string.end(),
            [&] { return distribution(random_number_generator()); });
    }
    return random_string;
}

template <typename IntType>
IntType RandomInt() {
  static std::uniform_int_distribution<IntType> distribution(std::numeric_limits<IntType>::min(),
          std::numeric_limits<IntType>::max());
  std::lock_guard<std::mutex> lock(random_number_generator_mutex());
  return distribution(random_number_generator());
}

std::string RandomString(size_t size) { return GetRandomString<std::string>(size); }
uint32_t RandomUint32() { return RandomInt<uint32_t>(); }
uint64_t RandomUint64() { return RandomInt<uint64_t>(); }



class GossipParams;
class RoutingMessage;

class GossipParams {
public:
    uint32_t               neighber_count;
    uint32_t               stop_times;
    uint32_t               gossip_type;
    uint32_t               max_hop_num;
    std::string            header_hash;
    std::string            block;
    uint32_t               msg_hash;
    bool                   allow_up;
    bool                   allow_low;
public:
    void print() {
        std::cout <<  "GossipParams:\n"
            <<  "neighber_count:"            <<    neighber_count            << "\n"
            <<  "stop_times:"                <<    stop_times                << "\n"
            <<  "gossip_type:"               <<    gossip_type               << "\n"
            <<  "max_hop_num:"               <<    max_hop_num               << "\n"
            <<  "header_hash:"               <<    HexEncode(header_hash)    << "\n"
            <<  "block:"                     <<    HexEncode(block)          << "\n"
            <<  "msg_hash:"                  <<    msg_hash                  << "\n"
            <<  "allow_up:"                  <<    allow_up                  << "\n"
            <<  "allow_low:"                 <<    allow_low                 << "\n"
            <<  std::endl;
    }
public:
    MSGPACK_DEFINE(
            neighber_count, stop_times, gossip_type,
            max_hop_num, header_hash, block, msg_hash,
            allow_up, allow_low);

};


class RoutingMessage {
public:
    std::string            src_node_id;
    std::string            des_node_id;
    uint32_t               type;
    std::string            data;
    uint32_t               id;
    uint32_t               hop_num;
    bool                   is_root;
    std::vector<uint64_t>  bloomfilter;
    bool                   broadcast;
    uint32_t               priority;
    GossipParams           gossip;

public:
    void print() {
        std::cout <<  "RoutingMessage:\n" 
            << "src_node_id:"       <<    HexEncode(src_node_id)      << "\n"
            <<  "des_node_id:"      <<    HexEncode(des_node_id)      << "\n"
            <<  "type:"             <<    type                        << "\n"
            <<  "data:"             <<    HexEncode(data)             << "\n"
            <<  "id:"               <<    id                          << "\n"
            <<  "hop_num:"          <<    hop_num                     << "\n"
            <<  "is_root:"          <<    is_root                     << "\n"
            <<  "broadcast:"        <<    broadcast                   << "\n"
            <<  "priority:"         <<    priority                    << "\n"
            <<  std::endl;

        gossip.print();
    }

public:
	MSGPACK_DEFINE(
            src_node_id,  des_node_id, type, data,
            id, hop_num, is_root, bloomfilter, broadcast,
            priority, gossip);
};

class x_gossip {
public:
    uint32_t               neighber_count;
    uint32_t               stop_times;
    uint32_t               gossip_type;
    uint32_t               max_hop_num;
    std::string            header_hash;
    std::string            block;
    uint32_t               msg_hash;
    bool                   allow_up;
    bool                   allow_low;
};


class xrouting_message : public top::base::xdataunit_t
{
public:
    enum{enum_obj_type = enum_xdata_type_string};

public:
    xrouting_message()
    :top::base::xdataunit_t(enum_xdata_type_max)
    {
    }
protected:
public:
    virtual ~xrouting_message()
    {
        clear();
    };
private:
    xrouting_message(const xrouting_message &);
    xrouting_message & operator = (const xrouting_message &);

protected: //not safe for multiple threads
    virtual int32_t    do_write(top::base::xstream_t & stream) override        //serialize whole object to binary
    {
        const int32_t begin_size = stream.size();
        
        stream << src_node_id;
        stream << des_node_id;
        stream << type;
        stream << data;
        stream << id;
        stream << hop_num;
        stream << is_root;
        stream << bloomfilter;
        stream << broadcast;
        stream << priority;

        // x_gossip
        stream << gossip.neighber_count;
        stream << gossip.stop_times;
        stream << gossip.gossip_type;
        stream << gossip.max_hop_num;
        stream << gossip.header_hash;
        stream << gossip.block;
        stream << gossip.msg_hash;
        stream << gossip.allow_up;
        stream << gossip.allow_low;
 

        const int32_t end_size = stream.size();
        return (end_size - begin_size);
    }
    
    virtual int32_t    do_read(top::base::xstream_t & stream) override //serialize from binary and regeneate content of xdataobj_t
    {
        const int32_t begin_size = stream.size();
            
        stream >> src_node_id;
        stream >> des_node_id;
        stream >> type;
        stream >> data;
        stream >> id;
        stream >> hop_num;
        stream >> is_root;
        stream >> bloomfilter;
        stream >> broadcast;
        stream >> priority;

        stream >> gossip.neighber_count;
        stream >> gossip.stop_times;
        stream >> gossip.gossip_type;
        stream >> gossip.max_hop_num;
        stream >> gossip.header_hash;
        stream >> gossip.block;
        stream >> gossip.msg_hash;
        stream >> gossip.allow_up;
        stream >> gossip.allow_low;
            
        const int32_t end_size = stream.size();
        return (begin_size - end_size);
    }
public:
    virtual bool   clear() //relase resource
    {
        src_node_id.clear();
        des_node_id.clear();
        data.clear();
        bloomfilter.clear();
        gossip.header_hash.clear();
        gossip.block.clear();
        return true;
    }
public:
        std::string             src_node_id;
        std::string             des_node_id;
        uint32_t                type;
        std::string             data;
        uint32_t                id;
        uint32_t                hop_num;
        bool                    is_root;
        std::vector<uint64_t>   bloomfilter;       
        bool                    broadcast;
        uint32_t                priority;
        x_gossip                 gossip;
};



::bench::protobuf::RoutingMessage bench_protobuf_init_message(uint32_t binary_data_size = 300) {
    ::bench::protobuf::RoutingMessage gmessage;
    gmessage.set_src_node_id(HexDecode("67000000ff7fff7fffffffffffffffff000000000cc753ef4d35c81deae180c246d9321d"));
    gmessage.set_des_node_id(HexDecode("680000010101ff7fffffffffffffffff00000000a1bfa8d9d820d84e2c4765a08bfe5f0a"));
    gmessage.set_type(1021);
    gmessage.set_id(32311);
    gmessage.set_hop_num(9);
    gmessage.set_is_root(1);
    for (auto i = 0; i < 8; ++i) {
        gmessage.add_bloomfilter(RandomUint64());
    }
    gmessage.set_broadcast(1);
    gmessage.set_priority(1);

    auto ggossip = gmessage.mutable_gossip();
    ggossip->set_neighber_count(4);
    ggossip->set_stop_times(3); 
    ggossip->set_gossip_type(2);
    ggossip->set_max_hop_num(20);
    ggossip->set_header_hash(RandomString(32));
    ggossip->set_block(RandomString(binary_data_size));
    ggossip->set_msg_hash(72992300);
    ggossip->set_allow_up(true);
    ggossip->set_allow_low(false);
    return gmessage;
}




void bench_protobuf_serialize(uint32_t binary_data_size = 300) {
    ::bench::protobuf::RoutingMessage gmessage = bench_protobuf_init_message(binary_data_size);

    auto start = std::chrono::high_resolution_clock::now();

    std::string str;
    uint32_t total = 0;
    for (auto i = 0; i < bench_num; ++i) {
        str.clear();
        gmessage.SerializeToString(&str);
        total += str.size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> tm = end - start;	// 毫秒
    // std::chrono::duration<double, std::micro> tm = end - start;  // 微秒
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << YELLOW << "protobuf" << std::setw(20) << "serialize" << std::setw(20) << str.size() << std::setw(10) << "Bytes" << RED <<std::endl;
    std::cout << "protobuf" << std::setw(20) << "serialize" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    return;
}

void bench_protobuf_parse_new(uint32_t binary_data_size = 300) {
    ::bench::protobuf::RoutingMessage gmessage = bench_protobuf_init_message(binary_data_size);
    std::string str;
    gmessage.SerializeToString(&str);

    auto start = std::chrono::high_resolution_clock::now();
    uint32_t total = 0;

    for (auto i = 0; i < bench_num; ++i) {
        ::bench::protobuf::RoutingMessage rgmessage;
        rgmessage.ParseFromArray((const char*)str.data(), str.size());
        total += str.size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << "protobuf" << std::setw(20) << "parse_new" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    return;
}

void bench_protobuf_parse_reuse(uint32_t binary_data_size = 300) {
    ::bench::protobuf::RoutingMessage gmessage = bench_protobuf_init_message(binary_data_size);
    std::string str;
    gmessage.SerializeToString(&str);

    auto start = std::chrono::high_resolution_clock::now();
    uint32_t total = 0;

    ::bench::protobuf::RoutingMessage rgmessage;
    for (auto i = 0; i < bench_num; ++i) {
        rgmessage.ParseFromArray((const char*)str.data(), str.size());
        total += str.size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << "protobuf" << std::setw(20) << "parse_reuse" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    return;
}

void bench_protobuf_run(uint32_t binary_data_size = 300) {
    //serialize
    bench_protobuf_serialize(binary_data_size);

    // parse
    bench_protobuf_parse_new(binary_data_size);
    bench_protobuf_parse_reuse(binary_data_size);
    std::cout << std::endl;
    return;
}

RoutingMessage bench_msgpack_init_message(uint32_t binary_data_size = 300) {
    GossipParams mgossip;
    mgossip.neighber_count = 4;
    mgossip.stop_times = 3;
    mgossip.gossip_type = 2;
    mgossip.max_hop_num = 20;
    mgossip.header_hash = RandomString(32);
    mgossip.block = RandomString(binary_data_size);
    mgossip.msg_hash = 72992300;
    mgossip.allow_up = true;
    mgossip.allow_low = false;

    RoutingMessage message;
    message.src_node_id = HexDecode("67000000ff7fff7fffffffffffffffff000000000cc753ef4d35c81deae180c246d9321d");
    message.des_node_id = HexDecode("680000010101ff7fffffffffffffffff00000000a1bfa8d9d820d84e2c4765a08bfe5f0a");
    message.type = 1021;
    // message.data = RandomString(500);
    message.id = 32311;
    message.hop_num = 9;
    message.is_root = 1;
    std::vector<uint64_t> bl;
    for (auto i = 0; i < 8;++i) {
        bl.push_back(RandomUint64());
    }
    message.bloomfilter = bl;
    message.broadcast = 1;
    message.priority = 1;
    message.gossip = mgossip;
    return message;
}

void bench_msgpack_serialize(uint32_t binary_data_size = 300) {
    RoutingMessage message = bench_msgpack_init_message(binary_data_size);

    auto start = std::chrono::high_resolution_clock::now();

	msgpack::sbuffer str;
    uint32_t total = 0;
    for (auto i = 0; i < bench_num; ++i) {
        str.clear();
	    msgpack::pack(str, message);
        total += str.size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> tm = end - start;	// 毫秒
    // std::chrono::duration<double, std::micro> tm = end - start;  // 微秒
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << YELLOW << "msgpack" << std::setw(20) << "serialize" << std::setw(20) << str.size() << std::setw(10) << "Bytes" << GREEN << std::endl;
    std::cout << "msgpack" << std::setw(20) << "serialize" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    return;
}

void bench_msgpack_parse_new(uint32_t binary_data_size = 300) {
    RoutingMessage message = bench_msgpack_init_message(binary_data_size);
	msgpack::sbuffer str;
	msgpack::pack(str, message);

    auto start = std::chrono::high_resolution_clock::now();
    uint32_t total = 0;

    for (auto i = 0; i < bench_num; ++i) {
        msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());
	    msgpack::object obj =  oh.get();

        RoutingMessage rmessage;
	    obj.convert(rmessage);
        total += str.size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << "msgpack" << std::setw(20) << "parse_new" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    return;
}

void bench_msgpack_parse_reuse(uint32_t binary_data_size = 300) {
    RoutingMessage message = bench_msgpack_init_message(binary_data_size);
	msgpack::sbuffer str;
	msgpack::pack(str, message);

    auto start = std::chrono::high_resolution_clock::now();
    uint32_t total = 0;

    RoutingMessage rmessage;
    msgpack::object_handle oh;
    for (auto i = 0; i < bench_num; ++i) {
        oh = msgpack::unpack(str.data(), str.size());
	    msgpack::object obj =  oh.get();

	    obj.convert(rmessage);
        total += str.size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << "msgpack" << std::setw(20) << "parse_reuse" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    return;
}


void bench_msgpack_run(uint32_t binary_data_size = 300) {
    //serialize
    bench_msgpack_serialize(binary_data_size);

    // parse
    bench_msgpack_parse_new(binary_data_size);
    bench_msgpack_parse_reuse(binary_data_size);
    std::cout << std::endl;
    return;
}


// xdataunit
void bench_xdata_init_message(xrouting_message& message, uint32_t binary_data_size = 300) {
    x_gossip xgossip;
    xgossip.neighber_count = 4;
    xgossip.stop_times = 3;
    xgossip.gossip_type = 2;
    xgossip.max_hop_num = 20;
    xgossip.header_hash = RandomString(32);
    xgossip.block = RandomString(binary_data_size);
    xgossip.msg_hash = 72992300;
    xgossip.allow_up = true;
    xgossip.allow_low = false;

    message.src_node_id = HexDecode("67000000ff7fff7fffffffffffffffff000000000cc753ef4d35c81deae180c246d9321d");
    message.des_node_id = HexDecode("680000010101ff7fffffffffffffffff00000000a1bfa8d9d820d84e2c4765a08bfe5f0a");
    message.type = 1021;
    // message.data = RandomString(500);
    message.id = 32311;
    message.hop_num = 9;
    message.is_root = 1;
    std::vector<uint64_t> bl;
    for (auto i = 0; i < 8;++i) {
        bl.push_back(RandomUint64());
    }
    message.bloomfilter = bl;
    message.broadcast = 1;
    message.priority = 1;
    message.gossip = xgossip;
    return;
}

void bench_xdata_serialize(uint32_t binary_data_size = 300) {
    xrouting_message message;
    bench_xdata_init_message(message, binary_data_size);

    auto start = std::chrono::high_resolution_clock::now();

    top::base::xstream_t stream(top::base::xcontext_t::instance());
    uint32_t total = 0;
    uint32_t writed = 0;
    for (auto i = 0; i < bench_num; ++i) {
        writed = message.serialize_to(stream);
        stream.reset();
        total += writed;
    }

    auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> tm = end - start;	// 毫秒
    // std::chrono::duration<double, std::micro> tm = end - start;  // 微秒
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << YELLOW << "xdata" << std::setw(20) << "serialize" << std::setw(20) << writed << std::setw(10) << "Bytes" << BLUE << std::endl;
    std::cout << "xdata" << std::setw(20) << "serialize" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    //message.release_ref();
    return;
}

void bench_xdata_parse_new(uint32_t binary_data_size = 300) {
    xrouting_message message;
    bench_xdata_init_message(message, binary_data_size);
    top::base::xstream_t stream(top::base::xcontext_t::instance());
    message.serialize_to(stream);

    auto start = std::chrono::high_resolution_clock::now();
    uint32_t total = 0;
    uint32_t readed = 0;

    for (auto i = 0; i < bench_num; ++i) {
        top::base::xstream_t stream2(top::base::xcontext_t::instance(),stream.data(),stream.size());
        xrouting_message rmessage;
        readed = rmessage.serialize_from(stream2);
        total += readed;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << "xdata" << std::setw(20) << "parse_new" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    //message.release_ref();
    return;
}

void bench_xdata_parse_reuse(uint32_t binary_data_size = 300) {
    xrouting_message message;
    bench_xdata_init_message(message, binary_data_size);
    top::base::xstream_t stream(top::base::xcontext_t::instance());
    message.serialize_to(stream);

    auto start = std::chrono::high_resolution_clock::now();
    uint32_t total = 0;
    uint32_t readed = 0;

    xrouting_message rmessage;
    for (auto i = 0; i < bench_num; ++i) {
        top::base::xstream_t stream2(top::base::xcontext_t::instance(),stream.data(),stream.size());
        readed = rmessage.serialize_from(stream2);
        rmessage.clear();
        total += readed;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano>  tm = end - start;  // 纳秒

    double single_takes = tm.count() / bench_num;
    double iorate_num = (total / tm.count()) * 1000 * 1000 * 1000  / (1024 * 1024);  // MB/s
    std::string iorate = std::to_string(iorate_num) + "MB/s"; 
    std::cout << "xdata" << std::setw(20) << "parse_reuse" << std::setw(20) << single_takes << std::setw(10) << "ns" << std::setw(20) << iorate << std::endl;
    //message.release_ref();
    return;
}


void bench_xdata_run(uint32_t binary_data_size = 300) {
    //serialize
    bench_xdata_serialize(binary_data_size);

    // parse
    bench_xdata_parse_new(binary_data_size);
    bench_xdata_parse_reuse(binary_data_size);
    std::cout << std::endl;
    return;
}





int main(int argc ,char* argv[]) {
    xinit_log("/tmp/",true,true);
    xset_log_level(enum_xlog_level_debug);
    
    xset_trace_lines_per_file(1000);
    xset_log_file_hook(my_create_log_file_cb);//as default
    xset_log_trace_hook(my_func_hook_trace_cb);

    if (argc == 2) {
        uint32_t binary_size = atoi(argv[1]);
        std::cout << RED << "########################################################################" << std::endl;
        bench_msgpack_run(binary_size);
        std::cout << RED << "########################################################################" << std::endl;

        std::cout << GREEN << "########################################################################" << std::endl;
        bench_protobuf_run(binary_size);
        std::cout << GREEN << "########################################################################" << std::endl;

        std::cout << BLUE << "########################################################################" << std::endl;
        bench_xdata_run(binary_size);
        std::cout << BLUE << "########################################################################" << std::endl;
        return 0;
    }

    std::cout << RED << "########################################################################" << std::endl;
    // msgpack
    bench_msgpack_run(300);
    std::cout << WHITE << "----------------------------" << RED << std::endl;
    bench_msgpack_run(1024);
    std::cout << WHITE << "----------------------------" << RED << std::endl;
    bench_msgpack_run(10 * 1024);
    std::cout << WHITE << "----------------------------" << RED << std::endl;
    bench_msgpack_run(50 * 1024);
    std::cout << RED << "########################################################################" << std::endl;


    std::cout << GREEN << "########################################################################" << std::endl;
    // protobuf
    bench_protobuf_run(300);
    std::cout << WHITE << "----------------------------" << GREEN << std::endl;
    bench_protobuf_run(1024);
    std::cout << WHITE << "----------------------------" << GREEN << std::endl;
    bench_protobuf_run(10 * 1024);
    std::cout << WHITE << "----------------------------" << GREEN << std::endl;
    bench_protobuf_run(50 * 1024);
    std::cout << GREEN << "########################################################################" << std::endl;


    std::cout << BLUE << "########################################################################" << std::endl;
    // xdata 
    bench_xdata_run(300);
    std::cout << WHITE << "----------------------------" << BLUE << std::endl;
    bench_xdata_run(1024);
    std::cout << WHITE << "----------------------------" << BLUE << std::endl;
    bench_xdata_run(10 * 1024);
    std::cout << WHITE << "----------------------------" << BLUE << std::endl;
    bench_xdata_run(50 * 1024);
    std::cout << BLUE << "########################################################################" << std::endl;


    return 0;
}
