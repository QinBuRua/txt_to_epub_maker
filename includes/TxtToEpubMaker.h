//
// Created by QinBu_Rua on 2026/3/7.
//

#ifndef TXT_TO_EPUB_MAKER_INCLUDES_TXTTOEPUBMAKER_H
#define TXT_TO_EPUB_MAKER_INCLUDES_TXTTOEPUBMAKER_H

#include "json.hpp"
#include "pugixml/pugixml.hpp"

#include <deque>
#include <filesystem>
#include <string>

namespace QinBuRua::txt_to_epub_maker {

class TxtToEpubMaker {
public:
   using Xml           = pugi::xml_document;
   using XmlNode       = pugi::xml_node;
   using FilePath      = std::filesystem::path;
   using DirectoryPath = std::filesystem::path;
   using Chapter       = std::pair<std::u16string, FilePath>;
   using Chapters      = std::deque<Chapter>;
   using Volume        = std::pair<std::u16string, Chapters>;
   using Volumes       = std::deque<Volume>;
   using Json          = nlohmann::json;

   struct Epub {
      std::string mimetype;

      struct MetaInf {
         Xml container;
      } meta_inf;

      struct Oebps {
         Xml content_opf;
         Xml toc;
         Volumes xhtml;
      } oebps;
   };

public:
   void clear();
   void make_from_json(const Json& config_json);
   void generate_to(const std::string& path);

private:
   Json m_Config;
   Epub m_Epub;
   Volumes m_RegisteredVolumes;

   static Xml f_make_xml_template();
   static Xml f_make_xhtml_template();
   static void f_create_directory_if_not_exists(const DirectoryPath& directory_path);
   static void f_write_to_file(const FilePath& file_path, const std::string& content);
   static void f_save_xml_file(const FilePath& file_path, const Xml& xml_document);
   static std::string f_get_current_w3c_time();
   static std::optional<Chapter> f_register_chapter(const Json& chapter_json);
   static std::optional<Volume> f_register_volume(const Json& volume_json);
   static std::optional<Volumes> f_register_all_volumes(const Json& volumes_json);
   static std::optional<Xml> f_make_xhtml_from_txt(const std::string& title, const FilePath& file_path);

   void f_initialize_epub();
   void f_initialize_epub_container();
   void f_initialize_epub_content_opf();
   void f_make_metadate_to(XmlNode& metadata);

   void f_generate_temp_to(const DirectoryPath& directory_path);
   void f_generate_temp_all_xhtmls_to(const DirectoryPath& directory_path, const Volumes& volumes);
   void f_generate_temp_volume_to(const DirectoryPath& directory_path, const Volume& volume);
   void f_generate_temp_chapter_to(const FilePath& file_path, const Chapter& chapter);
   void f_make_manifest_and_spine_to(XmlNode& manifest_node, XmlNode& spine_node);
};

}

#endif //TXT_TO_EPUB_MAKER_INCLUDES_TXTTOEPUBMAKER_H
