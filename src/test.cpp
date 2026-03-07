//
// Created by QinBu_Rua on 2026/3/7.
//

#include "json.hpp"

#include "TxtToEpubMaker.h"

using Json = nlohmann::json;
using QinBuRua::txt_to_epub_maker::TxtToEpubMaker;

int main() {
   const Json config{
      {"novelAuthor", "author"},
      {"novelTitle", "title"},
      {"bookId", "urn:uuid:123e4567-e89b-12d3-a456-426614174000"},
      {"tempPath", "./data/temp"}
   };

   TxtToEpubMaker maker;
   maker.make_from_json(config);
   maker.generate_to("./data");

   return 0;
}
