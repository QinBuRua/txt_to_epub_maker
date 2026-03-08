//
// Created by QinBu_Rua on 2026/3/7.
//

#include <chrono>
#include <fstream>
#include <ranges>

#include "utfcpp/utf8.h"

#include "TxtToEpubMaker.h"

namespace stdv = std::views;
namespace stdr = std::ranges;
namespace stdf = std::filesystem;
using namespace QinBuRua::txt_to_epub_maker;

void TxtToEpubMaker::clear() {
   m_Epub.meta_inf = {};
   m_Epub.oebps    = {};
}

void TxtToEpubMaker::make_from_json(const Json& config_json) {
   m_Config = config_json;
   if (m_Config.find("temp_path") == m_Config.end()) {
      m_Config["temp_path"] = "./temp";
   }

   f_initialize_epub();

   auto volumesResult = f_register_all_volumes(m_Config["volumes"]);
   if (!volumesResult.has_value()) {
      throw std::runtime_error{"Fail to register volumes! Because volumes is all empty!!!"};
   }
   m_RegisteredVolumes = std::move(volumesResult.value());


}

void TxtToEpubMaker::generate_to(const std::string& path) {
   const DirectoryPath directoryPath{path};
   f_create_directory_if_not_exists(directoryPath);

   f_generate_temp_to(std::string{m_Config["tempPath"]});

}

TxtToEpubMaker::Xml TxtToEpubMaker::f_make_xml_template() {
   Xml xml;
   XmlNode decl = xml.append_child(pugi::node_declaration);

   decl.append_attribute("version")  = "1.0";
   decl.append_attribute("encoding") = "UTF-8";

   return xml;
}

TxtToEpubMaker::Xml TxtToEpubMaker::f_make_xhtml_template() {
   Xml xhtml    = f_make_xml_template();
   auto doctype = xhtml.append_child(pugi::node_doctype);
   doctype.set_value(
      R"(html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd")");

   auto html                         = xhtml.append_child("html");
   html.append_attribute("xmlns")    = "http://www.w3.org/1999/xhtml";
   html.append_attribute("xml:lang") = "zh-CN";
   html.append_attribute("lang")     = "zh-CN";

   auto head  = html.append_child("head");
   auto title = head.append_child("title");

   auto body = html.append_child("body");
   auto h1   = body.append_child("h1");

   return xhtml;
}

void TxtToEpubMaker::f_create_directory_if_not_exists(const DirectoryPath& directory_path) {
   if (!stdf::is_directory(directory_path)) {
      if (!stdf::create_directories(directory_path)) {
         throw std::runtime_error{std::format("Fail to create directory \"{}\"", directory_path.string())};
      }
   }
}

void TxtToEpubMaker::f_write_to_file(const FilePath& file_path, const std::string& content) {
   std::ofstream fout{file_path};
   if (fout.fail()) {
      throw std::runtime_error{std::format("Fail to open file \"{}\"", file_path.string())};
   }
   fout.write(content.c_str(), content.size());
   if (fout.fail()) {
      throw std::runtime_error{std::format("Fail to write file \"{}\"", file_path.string())};
   }
}

void TxtToEpubMaker::f_save_xml_file(const FilePath& file_path, const Xml& xml_document) {
   const DirectoryPath parentPath = file_path.parent_path();
   f_create_directory_if_not_exists(parentPath);
   if (!xml_document.save_file(file_path.c_str())) {
      throw std::runtime_error{std::format("Fail to save xml document \"{}\"", file_path.string())};
   }
}

std::string TxtToEpubMaker::f_get_current_w3c_time() {
   const auto now = std::chrono::system_clock::now();
   auto utcTime   = std::chrono::floor<std::chrono::seconds>(now);
   return std::format("{:%Y-%m-%dT%H:%M:%S}Z", utcTime);
}

std::optional<TxtToEpubMaker::Chapter> TxtToEpubMaker::f_register_chapter(const Json& chapter_json) {
   Chapter chapter;
   chapter.second = FilePath{chapter_json["url"]};
   if (!stdf::is_regular_file(chapter.second)) {
      return std::nullopt;
   }

   chapter.first = utf8::utf8to16(std::string(chapter_json["title"]));
   return chapter;
}

std::optional<TxtToEpubMaker::Volume> TxtToEpubMaker::f_register_volume(const Json& volume_json) {
   Volume volume;
   volume.first   = utf8::utf8to16(std::string(volume_json["volumeName"]));
   auto& chapters = volume.second;
   for (const auto& chapterJson : volume_json["chapters"]) {
      auto result = f_register_chapter(chapterJson);
      if (result.has_value()) {
         chapters.push_back(std::move(result.value()));
      }
   }
   if (chapters.empty()) {
      return std::nullopt;
   }
   return volume;
}

std::optional<TxtToEpubMaker::Volumes> TxtToEpubMaker::f_register_all_volumes(const Json& volumes_json) {
   Volumes volumes;
   for (const auto& volumeJson : volumes_json) {
      auto result = f_register_volume(volumeJson);
      if (result.has_value()) {
         volumes.push_back(std::move(result.value()));
      }
   }
   if (volumes.empty()) {
      return std::nullopt;
   }
   return volumes;
}

std::optional<TxtToEpubMaker::Xml> TxtToEpubMaker::f_make_xhtml_from_txt(
   const std::string& title, const FilePath& file_path
) {
   if (!stdf::is_regular_file(file_path)) {
      return std::nullopt;
   }

   auto xhtml = f_make_xhtml_template();
   auto html  = xhtml.child("html");
   auto head  = html.child("head");
   auto body  = html.child("body");

   head.child("title").text().set(title);
   body.child("h1").text().set(title);

   std::ifstream txtFile{file_path};
   if (!txtFile.is_open()) {
      return std::nullopt;
   }

   std::string line;
   std::string paragraph;
   while (std::getline(txtFile, line)) {
      if (!line.empty()) {
         paragraph += line;
         continue;
      }
      if (paragraph.empty()) {
         continue;
      }
      body.append_child("p").text().set(paragraph);
      paragraph.clear();
   }

   if (!txtFile.eof()) {
      return std::nullopt;
   }

   if (!paragraph.empty()) {
      body.append_child("p").text().set(paragraph);
   }

   return xhtml;
}

void TxtToEpubMaker::f_initialize_epub() {
   m_Epub.mimetype = "application/epub+zip";
   f_initialize_epub_container();
   f_initialize_epub_content_opf();
}

void TxtToEpubMaker::f_initialize_epub_container() {
   m_Epub.meta_inf.container = f_make_xml_template();

   auto root                        = m_Epub.meta_inf.container.append_child("container");
   root.append_attribute("version") = "1.0";
   root.append_attribute("xmlns")   = "urn:oasis:names:tc:opendocument:xmlns:container";

   auto rootfiles                          = root.append_child("rootfiles");
   auto rootfile                           = rootfiles.append_child("rootfile");
   rootfile.append_attribute("full-path")  = "OEPBS/content.opf";
   rootfile.append_attribute("media-type") = "application/oebps-package+xml";
}

void TxtToEpubMaker::f_initialize_epub_content_opf() {
   m_Epub.oebps.content_opf = f_make_xml_template();

   auto root                                  = m_Epub.oebps.content_opf.append_child("package");
   root.append_attribute("xmlns")             = "http://www.idpf.org/2007/opf";
   root.append_attribute("version")           = "3.0";
   root.append_attribute("unique-identifier") = "book-id";

   auto metadata = root.append_child("metadate");
   f_make_metadate_to(metadata);
}

void TxtToEpubMaker::f_make_metadate_to(XmlNode& metadata) {
   metadata.append_attribute("xmlns:dc") = "http://purl.org/dc/elements/1.1/";

   auto dcIdentifier = metadata.append_child("dc:identifier");
   dcIdentifier.text().set(std::string(m_Config["bookId"]));
   dcIdentifier.append_attribute("id") = "book-id";

   auto dcTitle = metadata.append_child("dc:title");
   dcTitle.text().set(std::string(m_Config["novelTitle"]));

   auto dcLanguage = metadata.append_child("dc:language");
   dcLanguage.text().set(std::string("zh-CN"));

   auto dcCreator = metadata.append_child("dc:creator");
   dcCreator.text().set(std::string(m_Config["novelAuthor"]));
   dcCreator.append_attribute("id") = "creator";

   auto meta = metadata.append_child("meta");
   meta.text().set(f_get_current_w3c_time());
   meta.append_attribute("property") = "dcterms:modified";
}

void TxtToEpubMaker::f_generate_temp_to(const DirectoryPath& directory_path) {
   if (stdf::exists(directory_path)) {
      stdf::remove_all(directory_path);
   }
   f_create_directory_if_not_exists(directory_path);

   const FilePath mimetypePath{directory_path / "mimetype"};
   f_write_to_file(mimetypePath, m_Epub.mimetype);

   const DirectoryPath metaInf{directory_path / "META-INF"};
   const FilePath containerXml{metaInf / "container.xml"};
   f_save_xml_file(containerXml, m_Epub.meta_inf.container);

   const DirectoryPath oebps{directory_path / "OEBPS"};
   const FilePath contentOpf{oebps / "content.opf"};
   f_save_xml_file(contentOpf, m_Epub.oebps.content_opf);

   const DirectoryPath xhtmlPath = oebps / "xhtml";
   f_generate_temp_all_xhtmls_to(xhtmlPath, m_RegisteredVolumes);
}

void TxtToEpubMaker::f_generate_temp_all_xhtmls_to(const DirectoryPath& directory_path, const Volumes& volumes) {
   for (const auto& volume : volumes) {
      f_generate_temp_volume_to(directory_path, volume);
   }
}

void TxtToEpubMaker::f_generate_temp_volume_to(const DirectoryPath& directory_path, const Volume& volume) {
   m_Epub.oebps.xhtml.push_back(Volume{volume.first, Chapters{}});
   const auto volumePath = directory_path / std::format("volume{}", m_Epub.oebps.xhtml.size());
   f_create_directory_if_not_exists(volumePath);

   const auto& chapters = volume.second;
   for (const auto& [index, chapter] : chapters | stdv::enumerate) {
      f_generate_temp_chapter_to(volumePath / std::format("chapter{}.xhtml", index), chapter);
   }
}

void TxtToEpubMaker::f_generate_temp_chapter_to(
   const FilePath& file_path, const Chapter& chapter
) {
   const auto result = f_make_xhtml_from_txt(utf8::utf16to8(chapter.first), chapter.second);
   if (!result.has_value()) {
      return;
   }
   f_save_xml_file(file_path, result.value());

   auto& epubBackChapters = m_Epub.oebps.xhtml.back().second;
   epubBackChapters.push_back(Chapter{chapter.first, file_path});
}
