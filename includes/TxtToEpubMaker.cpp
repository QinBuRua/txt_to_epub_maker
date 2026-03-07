//
// Created by QinBu_Rua on 2026/3/7.
//

#include <chrono>
#include <fstream>

#include "TxtToEpubMaker.h"

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
}

void TxtToEpubMaker::generate_to(const std::string& path) {
   const DirectoryPath directoryPath{path};
   f_create_directory_if_not_exists(directoryPath);

   f_generate_temp_to(std::string{m_Config["tempPath"]});

}

TxtToEpubMaker::Xml TxtToEpubMaker::f_make_default_xml() {
   Xml xml;
   XmlNode decl = xml.append_child(pugi::node_declaration);

   decl.append_attribute("version")  = "1.0";
   decl.append_attribute("encoding") = "UTF-8";

   return xml;
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

void TxtToEpubMaker::f_initialize_epub() {
   m_Epub.mimetype = "application/epub+zip";
   f_initialize_epub_container();
   f_initialize_epub_content_opf();
}

void TxtToEpubMaker::f_initialize_epub_container() {
   m_Epub.meta_inf.container = f_make_default_xml();

   auto root                        = m_Epub.meta_inf.container.append_child("container");
   root.append_attribute("version") = "1.0";
   root.append_attribute("xmlns")   = "urn:oasis:names:tc:opendocument:xmlns:container";

   auto rootfiles                          = root.append_child("rootfiles");
   auto rootfile                           = rootfiles.append_child("rootfile");
   rootfile.append_attribute("full-path")  = "OEPBS/content.opf";
   rootfile.append_attribute("media-type") = "application/oebps-package+xml";
}

void TxtToEpubMaker::f_initialize_epub_content_opf() {
   m_Epub.oebps.content_opf = f_make_default_xml();

   auto root                                  = m_Epub.oebps.content_opf.append_child("package");
   root.append_attribute("xmlns")             = "http://www.idpf.org/2007/opf";
   root.append_attribute("version")           = "3.0";
   root.append_attribute("unique-identifier") = "book-id";

   auto metadata = root.append_child("metadate");
   f_make_metadate(metadata);
}

void TxtToEpubMaker::f_make_metadate(XmlNode& metadata) {
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

void TxtToEpubMaker::f_generate_temp_to(const DirectoryPath& directory_path) const {
   f_create_directory_if_not_exists(directory_path);

   const FilePath mimetypePath{directory_path / "mimetype"};
   f_write_to_file(mimetypePath, m_Epub.mimetype);

   const DirectoryPath metaInf{directory_path / "META-INF"};
   const FilePath containerXml{metaInf / "container.xml"};
   f_save_xml_file(containerXml, m_Epub.meta_inf.container);

   const DirectoryPath oebps{directory_path / "OEBPS"};
   const FilePath contentOpf{oebps / "content.opf"};
   f_save_xml_file(contentOpf, m_Epub.oebps.content_opf);
}
