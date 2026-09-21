// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/SerRoot.hpp>
#include <huse/json/DeRoot.hpp>
#include <huse/ext/ValStdVector.hpp>
#include <huse/ext/ValStdOptional.hpp>

#include <iostream>
#include <vector>
#include <string>

struct Skill {
    std::string label;
    int mp; // mana to use skill

    template <typename Node, typename Self>
    static void huse_serde(Node& n, Self& self) {
        auto obj = n.obj();
        obj.renderCompact();
        obj.val("skill", self.label);
        obj.val("MP", self.mp);
    }
};

struct Familiar {
    std::string name;
    int hp;
    Skill skill;

    template <typename Node, typename Self>
    static void huse_serde(Node& n, Self& self) {
        auto obj = n.obj();
        obj.val("familiar", self.name);
        obj.val("HP", self.hp);
        obj.val("skill", self.skill);
    }
};

struct Character {
    std::string name;
    int hp;
    int mp;
    std::vector<Skill> skills;
    std::optional<Familiar> familiar;

    template <typename Node, typename Self>
    static void huse_serde(Node& n, Self& self) {
        auto obj = n.obj();
        obj.val("character", self.name);
        obj.val("HP", self.hp);
        obj.val("MP", self.mp);
        obj.val("skills", self.skills);
        obj.val("familiar", self.familiar);
    }
};

int main() {
    std::string json = R"json([
        {
            "character": "John Snow",
            "HP": 20,
            "MP": 10,
            "skills": [
                {"skill": "Sword", "MP": 1},
                {"skill": "Immortality", "MP": 10}
            ],
            "familiar": {"familiar": "Ghost", "HP": 5, "skill": {"skill": "Bite", "MP": 1}}
        },
        {
            "character": "Hodor",
            "HP": 40,
            "MP": 0,
            "skills": [
                {"skill": "Hodor", "MP": 0}
            ]
        }
    ])json";
    std::vector<Character> characters;

    // read characters
    huse::json::DeRoot d(huse::Parse, json);
    d.val(characters);

    // print characters as pretty json
    huse::json::SerRoot s(std::cout, true);
    s.val(characters);

    std::cout << '\n';
    return 0;
}
