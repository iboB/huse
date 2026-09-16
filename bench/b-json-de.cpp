#include <huse/json/DeRoot.hpp>

#include <pojobuf/document.hpp>
#include <pojobuf/document_parse.hpp>
#include <pojobuf/json/parser.hpp>

#include <boost/json.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include <json-test-data.h>
#include <string>
#include <vector>

struct input {
    const char* const path;
    std::vector<std::string> lines;
};

const input& get_input(picobench::state& state) {
    auto data = state.input_data();
    return *reinterpret_cast<const input*>(data);
}

using hash_t = uint64_t;

struct vec {
    int x, y, z;
    hash_t sum() const { return hash_t(x + y + z); }
};

hash_t hash(std::string_view str) {
    return std::hash<std::string_view>{}(str);
}

///////////////////////
// huse

using DeNode = huse::DeNode<huse::json::DeState>;
using DeObj = huse::DeObject<huse::json::DeState>;

void huse_serde(const DeNode& n, vec& v) {
    auto obj = n.obj();
    obj.val("x", v.x);
    obj.val("y", v.y);
    obj.val("z", v.z);
}

hash_t check_ack(const DeObj& obj) {
    auto node = obj.key("ack");
    if (!node) return 0;
    int ack;
    node.val(ack);
    return ack;
}

hash_t check_setSubscriptions(DeObj& d) {
    auto node = d.key("setSubscriptions");
    if (!node) return 0;
    hash_t res = 0;
    auto obj = node.obj();
    const int len = obj.size();
    for (int i = 0; i < len; ++i) {
        std::string_view key, value;
        obj.keyval(key, value);
        res += hash(key);
        res += hash(value);
    }

    return res;
}

hash_t check_setRequestBatch(DeObj& d) {
    auto node = d.key("setRequestBatch");
    if (!node) return 0;

    hash_t res = 0;
    auto obj = node.obj();
    {
        std::string_view batchId;
        obj.val("batchID", batchId);
        res += hash(batchId);
    }

    auto reqs = obj.obj("requests");
    while (true) {
        auto [key, value] = reqs.keyval();
        if (!value) break;
        auto req = value.obj();
        vec v;
        req.val("min", v);
        res += v.sum();
        req.val("max", v);
        res += v.sum();
    }

    return res;
}

hash_t check_ping(DeObj& d) {
    auto node = d.key("ping");
    if (!node) return 0;

    auto obj = node.obj();
    std::string_view pl;
    obj.val("payload", pl);
    return hash(pl);
}

hash_t check_setInteraction(DeObj& d) {
    auto node = d.key("setInteraction");
    if (!node) return 0;
    hash_t res = 0;
    auto obj = node.obj();

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
    for (size_t i = 0; i < structures.size(); ++i) {
        std::string_view sid;
        structures.val(sid);
        res += hash(sid);
    }

    return res;
}

hash_t traverse(const DeNode& node) {
    auto obj = node.obj();
    hash_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
}

///////////////////////
// pojobuf

hash_t check_ack(const pojobuf::value& obj) {
    auto f = obj.find_object_key("ack");
    if (f == obj.compound_length()) return 0;
    return obj.object_value_at(f).int32_value();
}

hash_t check_setSubscriptions(const pojobuf::value& obj) {
    auto f = obj.find_object_key("setSubscriptions");
    if (f == obj.compound_length()) return 0;
    hash_t res = 0;
    auto subs = obj.object_value_at(f);
    const size_t len = subs.compound_length();
    for (size_t i = 0; i < len; ++i) {
        auto key = subs.object_key_at(i);
        auto value = subs.object_value_at(i).string_value();
        res += hash(key);
        res += hash(value);
    }
    return res;
}

vec read_vec(const pojobuf::value& obj) {
    return {
        obj.object_value_at(obj.find_object_key("x")).int32_value(),
        obj.object_value_at(obj.find_object_key("y")).int32_value(),
        obj.object_value_at(obj.find_object_key("z")).int32_value(),
    };
}

hash_t check_setRequestBatch(const pojobuf::value& obj) {
    auto f = obj.find_object_key("setRequestBatch");
    if (f == obj.compound_length()) return 0;
    hash_t res = 0;
    auto batch = obj.object_value_at(f);

    {
        auto batchId = batch.object_value_at(batch.find_object_key("batchID")).string_value();
        res += hash(batchId);
    }

    auto reqs = batch.object_value_at(batch.find_object_key("requests"));
    auto rlen = reqs.compound_length();

    for (size_t i = 0; i < rlen; ++i) {
        auto req = reqs.object_value_at(i);
        res += read_vec(req.object_value_at_key("min")).sum();
        res += read_vec(req.object_value_at_key("max")).sum();
    }

    return res;
}

hash_t check_ping(const pojobuf::value& obj) {
    auto f = obj.find_object_key("ping");
    if (f == obj.compound_length()) return 0;
    auto ping = obj.object_value_at(f);

    auto pl = ping.object_value_at_key("payload").string_value();
    return hash(pl);
}

hash_t check_setInteraction(const pojobuf::value& obj) {
    auto f = obj.find_object_key("setInteraction");
    if (f == obj.compound_length()) return 0;
    hash_t res = 0;
    auto i = obj.object_value_at(f);

    auto type = i.object_value_at_key("type").string_value();
    if (type != "PlanarDrag_World") return 42;

    res += i.object_value_at_key("done").boolean_value();
    res += i.object_value_at_key("confirm").boolean_value();

    auto tool = i.object_value_at_key("tool").string_value();
    res += hash(tool);

    res += i.object_value_at_key("id").int32_value();
    res += i.object_value_at_key("seq").int32_value();

    auto ss = i.object_value_at_key("structures");
    auto sslen = ss.compound_length();
    for (size_t j = 0; j < sslen; ++j) {
        auto s = ss.array_element_at(j).string_value();
        res += hash(s);
    }

    return res;
}

hash_t traverse(const pojobuf::value& val) {
    hash_t res = 0;

    res += check_ack(val);
    res += check_setSubscriptions(val);
    res += check_setRequestBatch(val);
    res += check_ping(val);
    res += check_setInteraction(val);
    return res;
}

/////////////////////////////////
// boost

vec read_vec(boost::json::object& obj) {
    vec v;
    v.x = int(obj["x"].as_int64());
    v.y = int(obj["y"].as_int64());
    v.z = int(obj["z"].as_int64());
    return v;
}

hash_t check_ack(boost::json::object& obj) {
    auto it = obj.find("ack");
    if (it == obj.end()) return 0;
    return it->value().as_int64();
}

hash_t check_setSubscriptions(boost::json::object& obj) {
    auto it = obj.find("setSubscriptions");
    if (it == obj.end()) return 0;
    hash_t res = 0;
    auto subobj = it->value().as_object();
    for (auto& kv : subobj) {
        res += hash(kv.key());
        res += hash(kv.value().as_string());
    }
    return res;
}

hash_t check_setRequestBatch(boost::json::object& obj) {
    auto it = obj.find("setRequestBatch");
    if (it == obj.end()) return 0;
    hash_t res = 0;
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
            res += read_vec(minobj).sum();
        }
        {
            auto maxobj = req["max"].as_object();
            res += read_vec(maxobj).sum();
        }
    }
    return res;
}

hash_t check_ping(boost::json::object& obj) {
    auto it = obj.find("ping");
    if (it == obj.end()) return 0;
    auto pingobj = it->value().as_object();
    std::string_view pl = pingobj["payload"].as_string();
    return hash(pl);
}

hash_t check_setInteraction(boost::json::object& obj) {
    auto it = obj.find("setInteraction");
    if (it == obj.end()) return 0;
    hash_t res = 0;
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

hash_t traverse(boost::json::value& val) {
    auto obj = val.as_object();
    hash_t res = 0;
    res += check_ack(obj);
    res += check_setSubscriptions(obj);
    res += check_setRequestBatch(obj);
    res += check_ping(obj);
    res += check_setInteraction(obj);
    return res;
}

/////////////////////////////////
// benchmarks

void bench_huse(picobench::state& state) {
    auto lines_copy = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        huse::json::DeRoot root(huse::Parse_withMutableExternalSource, std::span(lines_copy[i]));
        sum += traverse(root);
    }

    state.set_result(sum);
}

void bench_pojobuf(picobench::state& state) {
    auto lines_copy = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto doc = pojobuf::document_parse_with<
                pojobuf::json::parser_charconv_num,
                pojobuf::parse_alloc_strategy::take_source
            >(std::move(lines_copy[i]));
        sum += traverse(doc->root());
    }

    state.set_result(sum);
}

void bench_boost(picobench::state& state) {
    auto& lines = get_input(state).lines;

    hash_t sum = 0;
    for (auto i : state) {
        auto root = boost::json::parse(lines[i]);
        sum += traverse(root);
    }

    state.set_result(sum);
}

std::vector<std::string> read_lines(const std::string& path) {
    std::vector<std::string> ret;
    std::ifstream list(path);
    while (list) {
        std::string line;
        std::getline(list, line);
        if (!line.empty()) {
            ret.push_back(line);
        }
    }
    return ret;
}

int main(int argc, char* argv[]) {
    input inputs[] = {
        {JSON_TEST_DATA_FILE_client_traffic_txt, },
        {JSON_TEST_DATA_FILE_client_traffic_rand_txt, },
    };

    std::vector<picobench::state::input> pb_inputs;
    pb_inputs.reserve(std::size(inputs));

    for (auto& i : inputs) {
        i.lines = read_lines(i.path);
        pb_inputs.push_back({int(i.lines.size()), reinterpret_cast<uintptr_t>(&i)});
    }

    picobench::local_runner r;

    r.add_benchmark("huse", bench_huse).inputs(pb_inputs);
    r.add_benchmark("pojobuf", bench_pojobuf).inputs(pb_inputs);
    r.add_benchmark("boost", bench_boost).inputs(pb_inputs);

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.parse_cmd_line(argc, argv);

    return r.run();
}