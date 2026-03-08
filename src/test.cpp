//
// Created by QinBu_Rua on 2026/3/7.
//

#include <fstream>

#include "json.hpp"

#include "TxtToEpubMaker.h"

using Json = nlohmann::json;
using QinBuRua::txt_to_epub_maker::TxtToEpubMaker;

int main() {
   const Json config = Json::parse(std::ifstream{"./data/TheRisingDawn/novel_config.json"});

   TxtToEpubMaker maker;
   maker.make_from_json(config);
   maker.generate_to("./data");

   return 0;
}
