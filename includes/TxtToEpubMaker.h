//
// Created by QinBu_Rua on 2026/3/7.
//

#ifndef TXT_TO_EPUB_MAKER_INCLUDES_CLASSES_TXTTOEPUBMAKER_H
#define TXT_TO_EPUB_MAKER_INCLUDES_CLASSES_TXTTOEPUBMAKER_H

#include <deque>
#include <filesystem>
#include <string>

#include "json.hpp"
#include "pugixml/pugixml.hpp"

namespace QinBuRua::txt_to_epub_maker {

class TxtToEpubMaker {
public:
   using Xml           = pugi::xml_document;
   using XmlNode       = pugi::xml_node;
   using FilePath      = std::filesystem::path;
   using DirectoryPath = std::filesystem::path;
   using FileList      = std::deque<FilePath>;

   using Json = nlohmann::json;

   struct Epub {
      std::string mimetype;

      struct MetaInf {
         Xml container;
      } meta_inf;

      struct Oebps {
         Xml content_opf;
         Xml toc;
         FileList xhtml;
      } oebps;
   };

public:
   void clear();
   void make_from_json(const Json& config_json);
   void generate_to(const std::string& path);

private:
   Json m_Config;
   Epub m_Epub;

   static Xml f_make_default_xml();
   static void f_create_directory_if_not_exists(const DirectoryPath& directory_path);
   static void f_write_to_file(const FilePath& file_path, const std::string& content);
   static void f_save_xml_file(const FilePath& file_path, const Xml& xml_document);
   static std::string f_get_current_w3c_time();

   void f_initialize_epub();
   void f_initialize_epub_container();
   void f_initialize_epub_content_opf();
   void f_make_metadate(XmlNode& metadata);

   void f_generate_temp_to(const DirectoryPath& directory_path) const;
};

}

#endif //TXT_TO_EPUB_MAKER_INCLUDES_CLASSES_TXTTOEPUBMAKER_H
