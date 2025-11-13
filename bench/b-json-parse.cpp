// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/JsonDeserializer.hpp>
#include <huse/Deserializer.hpp>
#include <huse/impl/Charconv.hpp>

#include <boost/json.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <simdjson.h>

#include <json-test-data.h>
#include <fstream>
#include <string>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

std::vector<std::string> g_lines;

using result_t = uint64_t;

struct vec {
    int x, y, z;
    int sum() const { return x + y + z; }
};

size_t hash(std::string_view str) {
    return std::hash<std::string_view>{}(str);
}

////////////////////////////////////////////////////////////////////////////////
// huse

void huseDeserialize(huse::DeserializerNode& n, vec& v) {
    auto obj = n.obj();
    obj.val("x", v.x);
    obj.val("y", v.y);
    obj.val("z", v.z);
}

result_t check_ack(huse::DeserializerObject& obj) {
    auto node = obj.optkey("ack");
    if (!node) return 0;
    int ack;
    node->val(ack);
    return ack;
}

result_t check_setSubscriptions(huse::DeserializerObject& d) {
    auto node = d.optkey("setSubscriptions");
    if (!node) return 0;
    result_t res = 0;
    auto obj = node->obj();
    const int len = obj.size();
    for (int i = 0; i < len; ++i) {
        std::string_view key, value;
        obj.keyval(key, value);
        res += hash(key);
        res += hash(value);
    }

    return res;
}

result_t check_setRequestBatch(huse::DeserializerObject& d) {
    auto node = d.optkey("setRequestBatch");
    if (!node) return 0;

    result_t res = 0;
    auto obj = node->obj();
    {
        std::string_view batchId;
        obj.val("batchID", batchId);
        res += hash(batchId);
    }

    auto reqs = obj.obj("requests");
    while (true) {
        auto n = reqs.optkeyval();
        if (!n) break;
        auto req = n->second.obj();
        vec v;
        req.val("min", v);
        res += v.sum();
        req.val("max", v);
        res += v.sum();
    }

    return res;
}

result_t check_ping(huse::DeserializerObject& d) {
    auto node = d.optkey("ping");
    if (!node) return 0;

    auto obj = node->obj();
    std::string_view pl;
    obj.val("payload", pl);

    result_t res = 0;
    HUSE_CHARCONV_NAMESPACE::from_chars(pl.data(), pl.data() + pl.size(), res);
    return res;
}

result_t check_setInteraction(huse::DeserializerObject& d) {
    auto node = d.optkey("setInteraction");
    if (!node) return 0;
    result_t res = 0;
    auto obj = node->obj();

    std::string_view type;
    obj.val("type", type);
    if (type != "PlanarDrag_World") return 42;

    bool done, confirm;
    obj.val("done", done);
    obj.val("confirm", confirm);
    res += done + confirm;

    std::string_view tool;
    obj.val("tool", tool);
    res += hash(tool);

    int id, seq;
    obj.val("id", id);
    obj.val("seq", seq);
    res += id + seq;

    auto structures = obj.ar("structures");
    for (int i = 0; i < structures.size(); ++i) {
        std::string_view sid;
        structures.val(sid);
        res += hash(sid);
    }

    return res;
}

result_t parse(huse::DeserializerRoot& d) {
    auto obj = d.obj();
    result_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
}

void bench_huse(picobench::state& s) {
    auto lines = g_lines;
    result_t res = 0;

    for (auto i : s) {
        auto& line = lines[i];
        auto d = huse::json::Make_Deserializer(line.data(), line.size());
        res += parse(d);
    }

    s.set_result(picobench::result_t(res));
}
PICOBENCH(bench_huse);

////////////////////////////////////////////////////////////////////////////////
// boost

vec from_boost_json(boost::json::object& obj) {
    vec v;
    v.x = int(obj["x"].as_int64());
    v.y = int(obj["y"].as_int64());
    v.z = int(obj["z"].as_int64());
    return v;
}

result_t check_ack(boost::json::object& obj) {
    auto it = obj.find("ack");
    if (it == obj.end()) return 0;
    return it->value().as_int64();
}

result_t check_setSubscriptions(boost::json::object& obj) {
    auto it = obj.find("setSubscriptions");
    if (it == obj.end()) return 0;
    result_t res = 0;
    auto subobj = it->value().as_object();
    for (auto& kv : subobj) {
        res += hash(kv.key());
        res += hash(kv.value().as_string());
    }
    return res;
}

result_t check_setRequestBatch(boost::json::object& obj) {
    auto it = obj.find("setRequestBatch");
    if (it == obj.end()) return 0;
    result_t res = 0;
    auto batchobj = it->value().as_object();
    {
        std::string_view batchId = batchobj["batchID"].as_string();
        res += hash(batchId);
    }
    auto reqs = batchobj["requests"].as_object();
    for (auto& reqv : reqs) {
        auto req = reqv.value().as_object();
        {
            auto minobj = req["min"].as_object();
            res += from_boost_json(minobj).sum();
        }
        {
            auto maxobj = req["max"].as_object();
            res += from_boost_json(maxobj).sum();
        }
    }
    return res;
}

result_t check_ping(boost::json::object& obj) {
    auto it = obj.find("ping");
    if (it == obj.end()) return 0;
    auto pingobj = it->value().as_object();
    std::string_view pl = pingobj["payload"].as_string();
    result_t res = 0;
    HUSE_CHARCONV_NAMESPACE::from_chars(pl.data(), pl.data() + pl.size(), res);
    return res;
}

result_t check_setInteraction(boost::json::object& obj) {
    auto it = obj.find("setInteraction");
    if (it == obj.end()) return 0;
    result_t res = 0;
    auto interobj = it->value().as_object();
    std::string_view type = interobj["type"].as_string();
    if (type != "PlanarDrag_World") return 42;
    bool done = interobj["done"].as_bool();
    bool confirm = interobj["confirm"].as_bool();
    res += done + confirm;
    std::string_view tool = interobj["tool"].as_string();
    res += hash(tool);
    int id = int(interobj["id"].as_int64());
    int seq = int(interobj["seq"].as_int64());
    res += id + seq;
    auto structures = interobj["structures"].as_array();
    for (auto& sv : structures) {
        std::string_view sid = sv.as_string();
        res += hash(sid);
    }
    return res;
}

result_t parse(boost::json::value& val) {
    auto obj = val.as_object();
    result_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
}

void bench_boost(picobench::state& s) {
    auto lines = g_lines;
    result_t res = 0;

    std::vector<std::byte> buffer(1024 * 1024);

    for (auto i : s) {
        auto& line = lines[i];
        boost::container::pmr::monotonic_buffer_resource mr(buffer.data(), buffer.size());
        auto jv = boost::json::parse(line, &mr);
        res += parse(jv);
    }

    s.set_result(picobench::result_t(res));
}

PICOBENCH(bench_boost);

////////////////////////////////////////////////////////////////////////////////
// simdjson

vec from_simdjson(simdjson::dom::object& obj) {
    vec v;
    v.x = int(obj.at_key("x").get_int64().value_unsafe());
    v.y = int(obj.at_key("y").get_int64().value_unsafe());
    v.z = int(obj.at_key("z").get_int64().value_unsafe());
    return v;
}

result_t check_ack(simdjson::dom::object& obj) {
    auto ack = obj.at_key("ack");
    if (ack.error()) return 0;
    return ack.get_int64().value_unsafe();
}

result_t check_setSubscriptions(simdjson::dom::object& obj) {
    auto subs = obj.at_key("setSubscriptions");
    if (subs.error()) return 0;
    result_t res = 0;
    auto subobj = subs.get_object().value_unsafe();
    for (auto field : subobj) {
        res += hash(field.key);
        res += hash(field.value.get_string().value_unsafe());
    }
    return res;
}

result_t check_setRequestBatch(simdjson::dom::object& obj) {
    auto batch = obj.at_key("setRequestBatch");
    if (batch.error()) return 0;
    result_t res = 0;
    auto batchobj = batch.get_object().value_unsafe();
    {
        std::string_view batchId = batchobj.at_key("batchID").get_string().value_unsafe();
        res += hash(batchId);
    }
    auto reqs = batchobj.at_key("requests").get_object().value_unsafe();
    for (auto reqfield : reqs) {
        auto req = reqfield.value.get_object().value_unsafe();
        {
            auto minobj = req.at_key("min").get_object().value_unsafe();
            res += from_simdjson(minobj).sum();
        }
        {
            auto maxobj = req.at_key("max").get_object().value_unsafe();
            res += from_simdjson(maxobj).sum();
        }
    }
    return res;
}

result_t check_ping(simdjson::dom::object& obj) {
    auto ping = obj.at_key("ping");
    if (ping.error()) return 0;
    auto pingobj = ping.get_object().value_unsafe();
    std::string_view pl = pingobj.at_key("payload").get_string().value_unsafe();
    result_t res = 0;
    HUSE_CHARCONV_NAMESPACE::from_chars(pl.data(), pl.data() + pl.size(), res);
    return res;
}

result_t check_setInteraction(simdjson::dom::object& obj) {
    auto inter = obj.at_key("setInteraction");
    if (inter.error()) return 0;
    result_t res = 0;
    auto interobj = inter.get_object().value_unsafe();
    std::string_view type = interobj.at_key("type").get_string().value_unsafe();
    if (type != "PlanarDrag_World") return 42;
    bool done = interobj.at_key("done").get_bool().value_unsafe();
    bool confirm = interobj.at_key("confirm").get_bool().value_unsafe();
    res += done + confirm;
    std::string_view tool = interobj.at_key("tool").get_string().value_unsafe();
    res += hash(tool);
    int id = int(interobj.at_key("id").get_int64().value_unsafe());
    int seq = int(interobj.at_key("seq").get_int64().value_unsafe());
    res += id + seq;
    auto structures = interobj.at_key("structures").get_array().value_unsafe();
    for (auto sv : structures) {
        std::string_view sid = sv.get_string().value_unsafe();
        res += hash(sid);
    }
    return res;
}

result_t parse(simdjson::dom::document_stream::iterator::value_type& elem) {
    auto obj = elem.get_object().value_unsafe();
    result_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
}

void bench_simdjson(picobench::state& s) {
    auto lines = g_lines;
    result_t res = 0;
    simdjson::dom::parser parser;
    for (auto i : s) {
        auto& line = lines[i];
        simdjson::pad(line);
        auto doc = parser.parse(line);
        res += parse(doc);
    }
    s.set_result(picobench::result_t(res));
}

PICOBENCH(bench_simdjson);

int main(int argc, char* argv[]) {
    {
        std::ifstream list(JSON_TEST_DATA_FILE_client_traffic_txt);
        while (list) {
            std::string line;
            std::getline(list, line);
            if (!line.empty()) {
                g_lines.push_back(line);
            }
        }
    }

    picobench::runner r;
    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.set_default_state_iterations({int(g_lines.size())});
    r.parse_cmd_line(argc, argv);
    return r.run();
}