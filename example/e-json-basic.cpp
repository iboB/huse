// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/Serializer.hpp>
#include <huse/json/Deserializer.hpp>
#include <huse/helpers/StdVector.hpp>

#include <iostream>
#include <vector>
#include <string>

template <typename Self>
struct SerializableT
{
    void huseSerialize(huse::SerializerNode& n) const
    {
        Self::serializeT(n, *static_cast<const Self*>(this));
    }
    void huseDeserialize(huse::DeserializerNode& n)
    {
        Self::serializeT(n, *static_cast<Self*>(this));
    }
};

struct Skill : public SerializableT<Skill>
{
    std::string label;
    int mp; // mana to use skill

    template <typename Node, typename Self>
    static void serializeT(Node& n, Self& self)
    {
        auto obj = n.obj();
        obj.val("skill", self.label);
        obj.val("MP", self.mp);
    }
};

struct Familiar : public SerializableT<Familiar>
{
    std::string name;
    int hp;
    Skill skill;

    template <typename Node, typename Self>
    static void serializeT(Node& n, Self& self)
    {
        auto obj = n.obj();
        obj.val("familiar", self.name);
        obj.val("HP", self.hp);
        obj.val("skill", self.skill);
    }
};

struct Character : public SerializableT<Character>
{
    std::string name;
    int hp;
    int mp;
    std::vector<Skill> skills;
    std::optional<Familiar> familiar;

    template <typename Node, typename Self>
    static void serializeT(Node& n, Self& self)
    {
        auto obj = n.obj();
        obj.val("character", self.name);
        obj.val("HP", self.hp);
        obj.val("MP", self.mp);
        obj.val("skills", self.skills);
        obj.val("familiar", self.familiar);
    }
};

int main()
{
    std::string json = R"json([
        {
            "character": "John Snow",
            "HP": 20,
            "MP": 10,
            "skills": [
                {"skill": "Sword", "MP": 1},
                {"skill": "Immortality", "MP": 10}
            ],
            "familiar": {"familiar": "Ghost", "HP": 5, "skill": {"skill": "Biting", "MP": 1}}
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
    auto d = huse::json::Deserializer::fromMutableString(json.data(), json.length());

    // read characters
    d.val(characters);

    // print characters as pretty json
    auto s = huse::json::Make_Serializer(std::cout, true);
    s.root().val(characters);

    std::cout << '\n';
    return 0;
}
