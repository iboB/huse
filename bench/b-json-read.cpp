#include <splat/warnings.h>
#include <huse/json/JsonDeserializer.hpp>
#include <huse/Deserializer.hpp>
#include <boost/json.hpp>
#include <simdjson.h>
#include <json-test-data.h>
#include <fstream>

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
    auto d = huse::json::Make_Deserializer(content.data(), content.size());
    s.stop_timer();

    s.set_result(d.root().obj().length());
}

void bench_boost(const char* path, picobench::state& s) {
    auto content = readFile(path);

    s.start_timer();
    auto jv = boost::json::parse(content);
    s.stop_timer();

    s.set_result(jv.as_object().size());
}

void bench_simdjson(const char* path, picobench::state& s) {
    auto content = readFile(path);
    s.start_timer();
    simdjson::pad(content);
    simdjson::dom::parser parser;
    auto doc = parser.parse(content);
    s.stop_timer();

    s.set_result(doc.get_object().size());
}

int main(int argc, char* argv[]) {
    picobench::local_runner r;

    r.add_benchmark("huse", [](picobench::state& s) {
        bench_huse(JSON_TEST_DATA_FILE_canada, s);
    });
    r.add_benchmark("boost", [](picobench::state& s) {
        bench_boost(JSON_TEST_DATA_FILE_canada, s);
    });
    r.add_benchmark("simdjson", [](picobench::state& s) {
        bench_simdjson(JSON_TEST_DATA_FILE_canada, s);
    });

    r.set_compare_results_across_samples(true);
    r.set_compare_results_across_benchmarks(true);
    r.set_default_state_iterations({1});
    r.parse_cmd_line(argc, argv);
    return r.run();
}
