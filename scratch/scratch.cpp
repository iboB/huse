// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <huse/json/Deserializer.hpp>
#include <huse/Exception.hpp>

int main()
{
    constexpr std::string_view json = R"({"ar": [2.3, -5], "val": 5, "b": false})";

    try
    {
        auto d = huse::json::Deserializer::fromConstString(json);
        int i;
        d.val(i);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
