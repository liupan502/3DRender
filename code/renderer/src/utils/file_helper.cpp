//
// Created by zhida.ji1 on 2022/8/23.
//
#include <memory>
#include <utils/file_helper.h>
#include <string>
#include <streambuf>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstring>
#include <cassert>
using namespace zr::utils;


#ifdef PLATFORM_ANDROID
AAssetManager* FileHelper::asset_mgr = nullptr;
#endif
std::string FileHelper::assert_base_dir = "";

void FileHelper::set_assert_base_dir(const std::string& dir) {
    FileHelper::assert_base_dir = dir;
}

std::string FileHelper::load_content(const char *path) {
    std::stringstream ss;
    ss << assert_base_dir << path;
#ifdef PLATFORM_ANDROID
    AAsset* asset = AAssetManager_open(asset_mgr,
                                       ss.str().c_str(),
                                       AASSET_MODE_BUFFER);
    uint32_t  buf_len = AAsset_getLength(asset);
    char *buf = new char[buf_len];
    memset(buf, 0, sizeof(char) * buf_len);
    AAsset_read(asset, buf, buf_len);
    AAsset_close(asset);
    
#else
#ifdef WIN32
    std::string path_t = ss.str();
    // ��б���滻Ϊ��б��
    for (char& c : path_t) {
        if (c == '/') {
            c = '\\';
        }
    }
    FILE* f = fopen(path_t.c_str(), "rb");

#else
    FILE* f = fopen(ss.str().c_str(), "rb");
#endif
    
    if (!f) {
        assert(false);
    }
    auto start_pos = ftell(f);
    fseek(f, 0, SEEK_END);
    auto end_pos = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint32_t buf_len = end_pos - start_pos;
    char *buf = new char[buf_len];
    memset(buf, 0, sizeof(char) * buf_len);
    fread(buf, sizeof(char), buf_len, f);
    fclose(f);
#endif
    std::string tmp((const char*) buf, buf_len);
    delete [] buf;
    return tmp;
}

std::vector<std::string> FileHelper::get_lines_from_file(const char *file_path) {

    std::string content(load_content(file_path));
    std::stringbuf string_buf(content.c_str());

    std::istream is(&string_buf);
    std::vector<std::string> lines;
    while (is.peek() != -1) {
        std::string line;
        std::istream::sentry se(is, true);
        std::streambuf *sb = is.rdbuf();
        if (se) {
            bool termial = false;
            for (;;) {
                if (termial) {
                    break;
                }
                int c = sb->sbumpc();
                switch (c) {
                    case '\n':
                        termial = true;
                        break;
                    case '\r':
                        if (sb->sgetc() == '\n') sb->sbumpc();
                        termial = true;
                        break;
                    case EOF:
                        // Also handle the case when the last line has no line ending
                        if (line.empty()) is.setstate(std::ios::eofbit);
                        termial = true;
                        break;
                    default:
                        line += static_cast<char>(c);
                }
            }
        }
        if (!line.empty()) {
            lines.emplace_back(line);
        }
    }

    return lines;
}


std::string FileHelper::get_dir_name_from_path(const std::string &path) {
    size_t pos = path.find_last_of("/");
    pos = (pos == std::string::npos) ? path.find_last_of("\\") : pos;
    if (pos == std::string::npos) {
        return std::string();
    }
    return path.substr(0,pos + 1);
}
