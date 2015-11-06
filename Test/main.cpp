//
//  main.cpp
//  Test
//
//  Created by Julian Walker on 11/5/15.
//  Copyright © 2015 FiftyThree. All rights reserved.
//

#define AUTOJSONCXX_HAS_MODERN_TYPES 1
#define AUTOJSONCXX_HAS_RVALUE 1
#define AUTOJSONCXX_HAS_VARIADIC_TEMPLATE 1

#include <iostream>
#include "operations.hpp"
#include <map>
#include <assert.h>
#include <experimental/optional>

using namespace autojsoncxx;
using namespace std;

struct Action
{
    virtual ~Action() {}
};

enum class Origin {
    Remote, Local
};

template<class T>
struct SyncActionT
{
    T Payload;
    Origin Origin;
    
    SyncActionT(const T &payload, enum Origin origin)
    : Payload(payload)
    , Origin(origin)
    {
    }
    
    std::string ToJSON()
    {
        return to_pretty_json_string(Payload);
    }
};


template<class T>
shared_ptr<SyncActionT<T>> ParseAction(bool isLocalChange, const rapidjson::Value &value)
{
    T payload;
    ParsingResult parsingResult;
    
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
    value.Accept(writer);
    
    from_json_string(sb.GetString(), payload, parsingResult);
    
    return make_shared<SyncActionT<T>>(payload, (isLocalChange ? Origin::Local : Origin::Remote));
}

const rapidjson::Value *ChildObject(const rapidjson::Value &parent, const char *name)
{
    if (parent.HasMember(name) && parent[name].IsObject()) {
        return &parent[name];
    } else {
        return nullptr;
    }
}

int main(int argc, const char * argv[]) {

    const string channelMessage = "{                \
        \"channelCommand\": {                       \
            \"channelId\": \"space:23456\",         \
            \"detail\": {                           \
                \"operation\": {                    \
                    \"CreateSpace\":{               \
                        \"SpaceID\":\"34567\"       \
                },                                  \
                \"isLocalChange\":true              \
            }                                       \
        }                                           \
    }}";
    
    rapidjson::Document doc;
    doc.Parse(channelMessage.c_str());
    assert(!doc.HasParseError());
    
    if (auto channelCommand = ChildObject(doc, "channelCommand")) {
        if (auto detail = ChildObject(*channelCommand, "detail")) {
            if (auto operation = ChildObject(*detail, "operation")) {
               
                bool isLocalChange = true;
                if (operation->HasMember("isLocalChange")) {
                    isLocalChange = (*operation)["isLocalChange"].GetBool();
                }
                
                for (auto it = operation->MemberBegin(); it != operation->MemberEnd(); it++) {
                    if (it->name == "CreateSpace") {
                        auto action = ParseAction<CreateSpace>(isLocalChange, it->value);
                        std::cout << action->ToJSON() << std::endl;
                        break;
                    }
                    // ...
                }
            }
        }
    }
   
    return 0;
}
