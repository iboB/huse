// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <iostream>
#include <huse/json/DeRoot.hpp>
#include <huse/json/SerRoot.hpp>

int main()
{
    constexpr std::string_view json = R"({"ar": [2.3, -5], "val": 5, "b": false})";

    try
    {
        huse::json::DeRoot d(huse::Parse, json);
        int i;
        d.obj().val("val", i);

        huse::json::SerRoot s(std::cout);
        s.val(i);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
