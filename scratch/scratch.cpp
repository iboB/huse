// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <iostream>
#include <huse/json/Deserializer.hpp>
#include <huse/Exception.hpp>

int main()
{
    constexpr std::string_view json = R"({"ar": [2.3, -5], "val": 5, "b": false})";

    try
    {
        auto d = huse::json::Make_Deserializer(json);
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
