#pragma once

#include <string>
#include <vector>
#include <memory>

// protobuf
#include <bench.pb.h>

::bench::protobuf::RoutingMessage bench_protobuf_init_message_original(uint32_t binary_data_size = 300);

bool protobuf_message_comp(google::protobuf::Message* messagea, google::protobuf::Message* messageb);

void PrintProtbufMessage(google::protobuf::Message* gmessage);

bool add_field_protobuf_compatibility();
bool protobuf_compatibility();
