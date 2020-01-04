// export LD_LIBRARY_PATH=/usr/local/lib
// g++ -I msgpack-c/include bench.cc bench.pb.cc   -I ./  -I /usr/local/include/google/protobuf/ -L /usr/local/lib/  -std=c++11 -o bench -Wl,-Bstatic -lprotobuf -Wl,-Bdynamic -pthread

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <random>
#include <mutex>
#include <algorithm>
#include <iomanip>

// gflags
#include <gflags/gflags.h>

// nlohmann_json
#include <nlohmann/json.hpp>

// msgpack
#include <msgpack.hpp>
// protobuf
#include <bench.pb.h>

// for convenience
using json = nlohmann::json;

// 循环次数
uint32_t bench_num = 100000;


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




json bench_protobuf_serialize(uint32_t binary_data_size = 300) {
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

    json result;
    result["binary_size"] = binary_data_size;
    result["after_serialize_size"] = str.size();
    result["time"] = single_takes;
    result["iorate"] = iorate_num;
    return result;
}

json bench_protobuf_parse_new(uint32_t binary_data_size = 300) {
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

    json result;
    result["time"] = single_takes;
    result["iorate"] = iorate_num;
    return result;
}

json bench_protobuf_parse_reuse(uint32_t binary_data_size = 300) {
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

    json result;
    result["time"] = single_takes;
    result["iorate"] = iorate_num;
    return result;
}

json bench_protobuf_run(uint32_t binary_data_size = 300) {
    //serialize
    auto sresult = bench_protobuf_serialize(binary_data_size);

    // parse
    auto presult_new = bench_protobuf_parse_new(binary_data_size);
    auto presult_reuse = bench_protobuf_parse_reuse(binary_data_size);
    std::cout << std::endl;

    json result;
    result["binary_size"]            = sresult["binary_size"].get<uint32_t>();
    result["after_serialize_size"]   = sresult["after_serialize_size"].get<uint32_t>();
    result["serialize_time"]         = sresult["time"].get<uint32_t>();
    result["serialize_iorate"]       = sresult["iorate"].get<uint32_t>();
    result["parse_new_time"]         = presult_new["time"].get<uint32_t>();
    result["parse_new_iorate"]       = presult_new["iorate"].get<uint32_t>();
    result["parse_reuse_time"]       = presult_reuse["time"].get<uint32_t>();
    result["parse_reuse_iorate"]     = presult_reuse["iorate"].get<uint32_t>();

    return result;
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

json bench_msgpack_serialize(uint32_t binary_data_size = 300) {
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

    json result;
    result["binary_size"] = binary_data_size;
    result["after_serialize_size"] = str.size();
    result["time"] = single_takes;
    result["iorate"] = iorate_num;
    return result;
}

json bench_msgpack_parse_new(uint32_t binary_data_size = 300) {
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

    json result;
    result["time"] = single_takes;
    result["iorate"] = iorate_num;
    return result;
}

json bench_msgpack_parse_reuse(uint32_t binary_data_size = 300) {
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

    json result;
    result["time"] = single_takes;
    result["iorate"] = iorate_num;
    return result;
}


json bench_msgpack_run(uint32_t binary_data_size = 300) {
    //serialize
    auto sresult = bench_msgpack_serialize(binary_data_size);

    // parse
    auto presult_new = bench_msgpack_parse_new(binary_data_size);
    auto presult_reuse = bench_msgpack_parse_reuse(binary_data_size);
    std::cout << std::endl;

    json result;
    result["binary_size"]            = sresult["binary_size"].get<uint32_t>();
    result["after_serialize_size"]   = sresult["after_serialize_size"].get<uint32_t>();
    result["serialize_time"]         = sresult["time"].get<uint32_t>();
    result["serialize_iorate"]       = sresult["iorate"].get<uint32_t>();
    result["parse_new_time"]         = presult_new["time"].get<uint32_t>();
    result["parse_new_iorate"]       = presult_new["iorate"].get<uint32_t>();
    result["parse_reuse_time"]       = presult_reuse["time"].get<uint32_t>();
    result["parse_reuse_iorate"]     = presult_reuse["iorate"].get<uint32_t>();

    return result;
}


DEFINE_bool(simple, false, "just run simple test");
DEFINE_int32(binary_size, 200, "chain binary size");
DEFINE_int32(bench_num, 100000, "bench num, than get avg value");
DEFINE_bool(bench, false, "do benchmark test and print to stdout");
DEFINE_bool(dump, false, "do more benchmark and dump result to benchmark.json");
DEFINE_int32(dump_num, 30, "dump test chain binary size, from 100 Bytes begin, next will more 100Bytes");

int main(int argc ,char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    bench_num = FLAGS_bench_num;

    if (FLAGS_simple) {
        uint32_t binary_size = FLAGS_binary_size;
        std::cout << RED << "########################################################################" << std::endl;
        bench_msgpack_run(binary_size);
        std::cout << RED << "########################################################################" << std::endl;

        std::cout << GREEN << "########################################################################" << std::endl;
        bench_protobuf_run(binary_size);
        std::cout << GREEN << "########################################################################" << std::endl;
        return 0;
    }

    if (FLAGS_bench) {
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
        return 0;
    }

    if (FLAGS_dump) {
        json result;
        result["after_serialize_size"]["msgpack"]    = json::array();
        result["after_serialize_size"]["protobuf"]   = json::array();
        result["serialize_time"]["msgpack"]          = json::array();
        result["serialize_time"]["protobuf"]         = json::array();
        result["serialize_iorate"]["msgpack"]        = json::array();
        result["serialize_iorate"]["protobuf"]       = json::array();
        result["parse_new_time"]["msgpack"]          = json::array();
        result["parse_new_time"]["protobuf"]         = json::array();
        result["parse_new_iorate"]["msgpack"]        = json::array();
        result["parse_new_iorate"]["protobuf"]       = json::array();
        result["parse_reuse_time"]["msgpack"]        = json::array();
        result["parse_reuse_time"]["protobuf"]       = json::array();
        result["parse_reuse_iorate"]["msgpack"]      = json::array();
        result["parse_reuse_iorate"]["protobuf"]     = json::array();

        for (auto i = 0; i < FLAGS_dump_num; ++i) {
            auto binary_size     =  100 + i * 100;

            auto msgpack_result  = bench_msgpack_run(binary_size);
            auto protobuf_result = bench_protobuf_run(binary_size);

            result["after_serialize_size"]["msgpack"].push_back({binary_size, msgpack_result["after_serialize_size"].get<uint32_t>()});
            result["after_serialize_size"]["protobuf"].push_back({binary_size, protobuf_result["after_serialize_size"].get<uint32_t>()});
            result["serialize_time"]["msgpack"].push_back(   { binary_size, msgpack_result["serialize_time"].get<uint32_t>()  } );
            result["serialize_time"]["protobuf"].push_back( { binary_size, protobuf_result["serialize_time"].get<uint32_t>() } );
            result["serialize_iorate"]["msgpack"].push_back(   { binary_size, msgpack_result["serialize_iorate"].get<uint32_t>()  } );
            result["serialize_iorate"]["protobuf"].push_back( { binary_size, protobuf_result["serialize_iorate"].get<uint32_t>() } );
            result["parse_new_time"]["msgpack"].push_back(   { binary_size, msgpack_result["parse_new_time"].get<uint32_t>()  } );
            result["parse_new_time"]["protobuf"].push_back( { binary_size, protobuf_result["parse_new_time"].get<uint32_t>() } );
            result["parse_new_iorate"]["msgpack"].push_back(   { binary_size, msgpack_result["parse_new_iorate"].get<uint32_t>()  } );
            result["parse_new_iorate"]["protobuf"].push_back( { binary_size, protobuf_result["parse_new_iorate"].get<uint32_t>() } );
            result["parse_reuse_time"]["msgpack"].push_back(   { binary_size, msgpack_result["parse_reuse_time"].get<uint32_t>()  } );
            result["parse_reuse_time"]["protobuf"].push_back( { binary_size, protobuf_result["parse_reuse_time"].get<uint32_t>() } );
            result["parse_reuse_iorate"]["msgpack"].push_back(   { binary_size, msgpack_result["parse_reuse_iorate"].get<uint32_t>()  } );
            result["parse_reuse_iorate"]["protobuf"].push_back( { binary_size, protobuf_result["parse_reuse_iorate"].get<uint32_t>() } );
        }

        //std::cout << result.dump(4) << std::endl;
        std::ofstream out("benchmark.json");
        out << std::setw(4) << result << std::endl;
        return 0;
    }

    // look up help info
    gflags::ShowUsageWithFlags(argv[0]);
    return 0;
}
