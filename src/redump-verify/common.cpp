#include "common.hpp"

#include <pugixml.hpp>

#include <string_view>

using namespace std::literals;

namespace xml {
auto attr(const pugi::xml_node &node, const char *name) {
    return node.attribute(name).value();
}
} // namespace xml

auto redump::load(const std::filesystem::path &path) -> std::optional<redump::games> {
    pugi::xml_document xml;
    if (!xml.load_file(path.c_str())) return {};

    std::vector<redump::game> ret;

    for (auto &&node : xml.document_element()) {
        if (node.name() != "game"sv) continue;

        redump::game game{
            .name = xml::attr(node, "name"),
            .category = node.child_value("category"),
            .description = node.child_value("description"),
        };

        for (auto &&s : node) {
            if (s.name() != "rom"sv) continue;

            game.roms.emplace_back( //
                redump::rom{
                    .filename = xml::attr(s, "name"),
                    .size = std::stoull(xml::attr(s, "size")),
                    .crc = xml::attr(s, "crc"),
                    .md5 = xml::attr(s, "md5"),
                    .sha1 = xml::attr(s, "sha1"),
                });
        }

        ret.emplace_back(std::move(game));
    }

    return ret;
}
