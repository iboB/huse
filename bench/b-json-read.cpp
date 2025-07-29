#include <splat/warnings.h>
#include <huse/json/JsonDeserializer.hpp>
#include <huse/Deserializer.hpp>
#include <boost/json.hpp>
#include <simdjson.h>
#include <json-test-data.h>
#include <fstream>

#define PICOBENCH_STD_FUNCTION_BENCHMARKS
#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

std::string readFile(const char* path) {
    std::ifstream fin(path);
    if (!fin) {
        throw std::runtime_error("Failed to open file: " + std::string(path));
    }
    std::string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    return content;
}

void bench_huse(const char* path, picobench::state& s) {
    auto content = readFile(path);

    s.start_timer();
    auto d = huse::json::Make_Deserializer(content);
    s.stop_timer();

    if (d.type().is(huse::Type::Object)) {
        s.set_result(d.obj().length());
    }
    else if (d.type().is(huse::Type::Array)) {
        s.set_result(d.ar().length());
    }
}

void bench_boost(const char* path, picobench::state& s) {
    auto content = readFile(path);

    boost::json::monotonic_resource mr(5 * content.size());

    s.start_timer();
    auto jv = boost::json::parse(content, &mr);
    s.stop_timer();

    if (jv.is_object()) {
        s.set_result(jv.as_object().size());
    }
    else if (jv.is_array()) {
        s.set_result(jv.as_array().size());
    }
}

void bench_simdjson(const char* path, picobench::state& s) {
    auto content = readFile(path);
    s.start_timer();
    simdjson::pad(content);
    simdjson::dom::parser parser;
    auto jv = parser.parse(content);
    s.stop_timer();

    if (jv.is_object()) {
        s.set_result(jv.get_object().size());
    }
    else if (jv.is_array()) {
        s.set_result(jv.get_array().size());
    }
}

int main(int argc, char* argv[]) {
    picobench::local_runner r;

    std::string_view files[] = { JSON_TEST_DATA_FILES };

    for (auto f : files) {
        auto fname = f.substr(sizeof(JSON_TEST_DATA_DIR));
        r.set_suite(fname.data());
        r.add_benchmark("huse", [=](picobench::state& s) {
            bench_huse(f.data(), s);
        });
        r.add_benchmark("boost", [=](picobench::state& s) {
            bench_boost(f.data(), s);
        });
        r.add_benchmark("simdjson", [=](picobench::state& s) {
            bench_simdjson(f.data(), s);
        });
    }

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.set_default_state_iterations({1});
    r.parse_cmd_line(argc, argv);
    return r.run();
}
