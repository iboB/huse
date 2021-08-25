// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <huse/JsonSerializer.hpp>

int main()
{
    huse::JsonSerializer j(std::cout, true);
    {
        auto o = j.obj();
        o.val("foo", "bar");
        {
            auto a = o.ar("xxx");
            a.val(1);
            a.ar();
            a.val(4);
            a.obj();
            {
                auto xx = a.obj();
                xx.val("as", 43.3);
            }
            {
                auto xx = a.ar();
                xx.val(1);
                xx.val(3.4);
                xx.val("asd");
            }
        }
        o.ar("yyy");
        o.obj("zzz");
    }
    return 0;
}
