//
// Created by zhida.ji1 on 2022/9/14.
//
#include <string.h>
#include "code_generator.h"
#include <utils/file_helper.h>
using namespace zr::utils;

void CodeGenerator::excute(std::stringstream &ss, const char *base_dir, const char *file_path,
                           std::vector<std::string> defines) {
    ss.str("");
    std::vector<std::string> lines;
    add_defines(lines, defines);
    add_file(lines, base_dir, file_path);
    for (int i = 0; i < lines.size(); i++) {
        ss << lines[i] << std::endl;
    }
}

void CodeGenerator::add_defines(std::vector<std::string> &lines,
                                const std::vector<std::string> &defines) {
    lines.insert(lines.end(), defines.begin(), defines.end());
}

void CodeGenerator::add_file(std::vector<std::string> &lines, const char *base_dir,
                             const char *file_path) {
    std::stringstream ss;
    ss << base_dir << file_path;
    std::vector<std::string> fi_lines = FileHelper().get_lines_from_file(ss.str().c_str());
    for (uint32_t i = 0; i < fi_lines.size(); i++) {
        if (starts_with(fi_lines[i].c_str(), "#include")) {
            replace_include(fi_lines[i], lines, base_dir);
        }
        else {
            lines.insert(lines.end(), fi_lines[i]);
        }
    }
}

bool CodeGenerator::starts_with(const char *str1, const char *str2) {
    if (strlen(str1) < strlen(str2)) {
        return false;
    }
    if (strncmp(str1, str2, strlen(str2)) == 0) {
        return true;
    }
    return false;
}

std::string CodeGenerator::trim(const std::string &str) {
    const char* ptr = str.c_str();
    int left_offset = 0;
    while(true) {
        if (ptr[left_offset] == ' ') {
            ++left_offset;
            continue;
        }
        break;
    }

    int right_offset = 0;
    while(true) {
        if (ptr[str.size() - right_offset - 1] == ' ') {
            ++right_offset;
            continue;
        }
        break;
    }
    std::string ret = str.substr(left_offset, str.size() - left_offset - right_offset);
    return std::move(ret);
}

void CodeGenerator::replace_include(const std::string &include_str,
                                    std::vector <std::string> &lines, const char *base_dir) {
    std::string trim_line = trim(include_str.substr(8, include_str.size() - 8));
    std::string file_path = trim_line.substr(1, trim_line.size() - 2);
    add_file(lines, base_dir, file_path.c_str());
}
