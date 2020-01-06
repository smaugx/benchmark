#include "protobuf_compatibility.h"

#include <string>
#include <vector>
#include <memory>
#include <iomanip>

#include "bench.h"

// protobuf

::bench::protobuf::RoutingMessage bench_protobuf_init_message_original(uint32_t binary_data_size) {
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
    gmessage.set_data("data");

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

    ::bench::protobuf::RoutingMessage empty_message;
    std::string test_str;
    empty_message.SerializeToString(&test_str);
    std::cout << "empty RoutingMessage size is " << test_str.size() << std::endl;
    return gmessage;
}

bool protobuf_message_comp_with_name(google::protobuf::Message* messagea, google::protobuf::Message* messageb) {
    std::map<std::string, uint32_t> field_name_map;
    field_name_map["src_node_id"]    = google::protobuf::FieldDescriptor::TYPE_BYTES;
    field_name_map["des_node_id"]    = google::protobuf::FieldDescriptor::TYPE_BYTES;
    field_name_map["type"]           = google::protobuf::FieldDescriptor::TYPE_UINT32;
    //field_name_map["data"]           = google::protobuf::FieldDescriptor::TYPE_BYTES;
    field_name_map["id"]             = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["hop_num"]        = google::protobuf::FieldDescriptor::TYPE_UINT32;
    //field_name_map["is_root"]        = google::protobuf::FieldDescriptor::TYPE_BOOL;
    field_name_map["bloomfilter"]    = 4999;  // myself define
    field_name_map["broadcast"]      = google::protobuf::FieldDescriptor::TYPE_BOOL;
    field_name_map["priority"]       = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["gossip"]         = google::protobuf::FieldDescriptor::TYPE_MESSAGE;
    field_name_map["neighber_count"] = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["stop_times"]     = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["gossip_type"]    = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["max_hop_num"]    = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["header_hash"]    = google::protobuf::FieldDescriptor::TYPE_BYTES;
    field_name_map["block    "]      = google::protobuf::FieldDescriptor::TYPE_BYTES;
    field_name_map["msg_hash "]      = google::protobuf::FieldDescriptor::TYPE_UINT32;
    field_name_map["allow_up "]      = google::protobuf::FieldDescriptor::TYPE_BOOL;
    field_name_map["allow_low"]      = google::protobuf::FieldDescriptor::TYPE_BOOL;

    const google::protobuf::Descriptor *descb       = messageb->GetDescriptor();
    const google::protobuf::Reflection *reflb       = messageb->GetReflection();

    const google::protobuf::Descriptor *desc       = messagea->GetDescriptor();
    const google::protobuf::Reflection *refl       = messagea->GetReflection();
    int fieldCount= desc->field_count();
    fprintf(stderr, "The fullname of the message is %s ,fieldcount %d \n", desc->full_name().c_str(), fieldCount);
    for(int i=0; i<fieldCount; i++) {
        const google::protobuf::FieldDescriptor *field = desc->field(i);
        fprintf(stderr, "The name of the %i th element is %s and the type is  %s \n",i,field->name().c_str(),field->type_name());

        auto ifind = field_name_map.find(field->name());
        if (ifind == field_name_map.end()) {
            std::cout << "not in check field name map" << std::endl;
            continue;
        }
        if(field->is_repeated()) {
            fprintf(stderr, "found repeated field\n");
            continue;
        }
        
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES){
            auto valuea = refl->GetString(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetString(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE){
            auto valuea = refl->GetDouble(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetDouble(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT){
            auto valuea = refl->GetFloat(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetFloat(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT64){
            auto valuea = refl->GetInt64(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt64(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64){
            auto valuea = refl->GetUInt64(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetUInt64(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32){
            auto valuea = refl->GetInt32(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt32(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32){
            auto valuea = refl->GetUInt32(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetUInt32(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL){
            auto valuea = refl->GetBool(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetBool(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING){
            auto valuea = refl->GetString(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetString(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_SINT32){
            auto valuea = refl->GetInt32(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt32(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_SINT64){
            auto valuea = refl->GetInt64(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt64(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE){
            const google::protobuf::Message& sub_messagea = refl->GetMessage(*messagea, field);
            const google::protobuf::Message& sub_messageb = reflb->GetMessage(*messageb, field);
            if (!protobuf_message_comp_with_name((google::protobuf::Message*)&sub_messagea, (google::protobuf::Message*)&sub_messageb)) {
                std::cout << "sub_message not equal" << std::endl;
                return false;
            }
        } else {
            std::cout << "not support type:" << field->type() << std::endl;
        }
    } // end for

    return true;
}


bool protobuf_message_comp_auto(google::protobuf::Message* messagea, google::protobuf::Message* messageb) {
    const google::protobuf::Descriptor *descb       = messageb->GetDescriptor();
    const google::protobuf::Reflection *reflb       = messageb->GetReflection();

    const google::protobuf::Descriptor *desc       = messagea->GetDescriptor();
    const google::protobuf::Reflection *refl       = messagea->GetReflection();
    int fieldCount= desc->field_count();
    fprintf(stderr, "The fullname of the message is %s ,fieldcount %d \n", desc->full_name().c_str(), fieldCount);
    for(int i=0; i<fieldCount; i++) {
        const google::protobuf::FieldDescriptor *field = desc->field(i);
        fprintf(stderr, "The name of the %i th element is %s and the type is  %s \n",i,field->name().c_str(),field->type_name());
        if(field->is_repeated()) {
            fprintf(stderr, "found repeated field\n");
            continue;
        }
        
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES){
            auto valuea = refl->GetString(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetString(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE){
            auto valuea = refl->GetDouble(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetDouble(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT){
            auto valuea = refl->GetFloat(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetFloat(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT64){
            auto valuea = refl->GetInt64(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt64(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64){
            auto valuea = refl->GetUInt64(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetUInt64(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32){
            auto valuea = refl->GetInt32(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt32(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32){
            auto valuea = refl->GetUInt32(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetUInt32(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL){
            auto valuea = refl->GetBool(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetBool(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING){
            auto valuea = refl->GetString(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetString(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_SINT32){
            auto valuea = refl->GetInt32(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt32(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_SINT64){
            auto valuea = refl->GetInt64(*messagea, field);
            if (!reflb->HasField(*messageb, field)) {
                std::cout << "messageb not include field" << std::endl;
                return false;
            } else {
                if (valuea != reflb->GetInt64(*messageb, field)) {
                    std::cout << "value not equal" << std::endl;
                    return false;
                }
            }
            continue;
        }
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE){
            const google::protobuf::Message& sub_messagea = refl->GetMessage(*messagea, field);
            const google::protobuf::Message& sub_messageb = reflb->GetMessage(*messageb, field);
            if (!protobuf_message_comp_auto((google::protobuf::Message*)&sub_messagea, (google::protobuf::Message*)&sub_messageb)) {
                std::cout << "sub_message not equal" << std::endl;
                return false;
            }
        } else {
            std::cout << "not support type:" << field->type() << std::endl;
        }
    } // end for

    return true;
}

void PrintProtbufMessage(google::protobuf::Message* gmessage) {
    const google::protobuf::Descriptor *desc       = gmessage->GetDescriptor();
    const google::protobuf::Reflection *refl       = gmessage->GetReflection();
    int fieldCount= desc->field_count();
    std::cout << "fullname:" << desc->full_name() << " fieldcount:" << fieldCount << std::endl;
    for(int i=0; i<fieldCount; i++) {
        const google::protobuf::FieldDescriptor *field = desc->field(i);
        if(field->is_repeated()) {
            std::cout << "found repeated field" << std::endl;
            continue;
        }
        
        if(field->type() == google::protobuf::FieldDescriptor::TYPE_STRING){
            auto value = refl->GetString(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE){
            auto value = refl->GetDouble(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT){
            auto value = refl->GetFloat(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT64){
            auto value = refl->GetInt64(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT64){
            auto value = refl->GetUInt64(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_INT32){
            auto value = refl->GetInt32(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32){
            auto value = refl->GetUInt32(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL){
            auto value = refl->GetBool(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES){
            auto value = refl->GetString(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << HexSubstr(value) << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_SINT32){
            auto value = refl->GetInt32(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_SINT64){
            auto value = refl->GetInt64(*gmessage, field);
            std::cout << std::setw(10) << "element_name:" << std::setw(20) <<  field->name() << std::setw(20) << "element_type:" <<  std::setw(20) << field->type_name() << std::setw(20)  << value << std::endl;
        } else if(field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE){
            const google::protobuf::Message& sub_message = refl->GetMessage(*gmessage, field);
            std::cout << "found message type" << std::endl;
            PrintProtbufMessage((google::protobuf::Message*)&sub_message);
        } else {
            std::cout << "not support type:" << field->type() << std::endl;
        }
    } // end for

    std::cout << "########################################" << std::endl;
    return;
}


bool add_field_protobuf_compatibility() {
    std::string func_name  = "[add_field_protobuf_compatibility]";
    // original message
    ::bench::protobuf::RoutingMessage message = bench_protobuf_init_message_original(300);

    std::string str;
    if (!message.SerializeToString(&str)) {
        std::cout << "RoutingMessage serialize string failed" << std::endl;
        return false;
    }
    std::cout << "RoutingMessage serialize string success, size is " << str.size() << std::endl;

    ::bench::protobuf::RoutingMessage_Add_Delete_Fields message_new;
    if (!message_new.ParseFromArray((const char*)str.data(), str.size())) {
        std::cout << func_name << "RoutingMessage_Add_Delete_Fields parse protobuf from RoutingMessage failed" << std::endl;
        return false;
    } else {
        std::cout << func_name << "RoutingMessage_Add_Delete_Fields parse protobuf from RoutingMessage success" << std::endl;
    }

    std::string str_add;
    if (!message_new.SerializeToString(&str_add)) {
        std::cout << "RoutingMessage_Add_Delete_Fields serialize string failed" << std::endl;
        return false;
    }
    std::cout << "RoutingMessage_Add_Delete_Fields serialize string success, size is " << str_add.size() << std::endl;

    ::bench::protobuf::RoutingMessage rmessage;
    if (!rmessage.ParseFromArray((const char*)str_add.data(), str_add.size())) {
        std::cout << func_name << "RoutingMessage parse protobuf from RoutingMessage_Add_Delete_Fields failed" << std::endl;
        return false;
    } else {
        std::cout << func_name << "RoutingMessage parse protobuf from RoutingMessage_Add_Delete_Fields success" << std::endl;
    }

    if (!protobuf_message_comp_with_name(&message, &rmessage)) {
        std::cout << "RoutingMessage parse protobuf from RoutingMessage_Add_Delete_Fields not equal the original RoutingMessage" << std::endl;
        return false;
    } else {
        std::cout << "RoutingMessage parse protobuf from RoutingMessage_Add_Delete_Fields equal the original RoutingMessage" << std::endl;
    }

    std::cout << BLUE << "original RoutingMessage" << std::endl;
    PrintProtbufMessage(&message);
    std::cout << RED << "RoutingMessage_Add_Delete_Fields" << std::endl;
    PrintProtbufMessage(&message_new);
    std::cout << BOLDYELLOW << "after parse RoutingMessage" << std::endl;
    PrintProtbufMessage(&rmessage);
    return true;

}


bool protobuf_compatibility() {
    /*
    // test print
    ::bench::protobuf::RoutingMessage message = bench_protobuf_init_message_original(300);
    PrintProtbufMessage(&message);
    */

    bool ret = add_field_protobuf_compatibility();
    return ret; 
}
