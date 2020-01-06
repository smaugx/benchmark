#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <mutex>
#include <algorithm>

// msgpack
#include <msgpack.hpp>


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

const char kHexAlphabet[] = "0123456789abcdef";
const char kHexLookup[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,  8,  9,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15 };


std::string HexEncode(const std::string& str);
std::string HexSubstr(const std::string& str);
std::string HexDecode(const std::string& str);
uint32_t& rng_seed();
std::mt19937& random_number_generator();
std::mutex& random_number_generator_mutex();

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

std::string RandomString(size_t size);
uint32_t RandomUint32();
uint64_t RandomUint64();




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


