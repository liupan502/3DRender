//
// Created by zhida.ji1 on 2022/9/14.
//

#pragma once
#include <sstream>
#include <string>
#include <vector>
namespace zr{
    namespace utils{
        class CodeGenerator{
        public:
            CodeGenerator() = default;

            void excute(std::stringstream& ss, const char* base_dir, const char* file_path, std::vector<std::string> defines);

        private:
            void add_defines(std::vector<std::string>& lines, const std::vector<std::string>& defines);
            void add_file(std::vector<std::string>& lines, const char* base_dir, const char* file_path);
            bool starts_with(const char* str1, const char* str2);
            void replace_include(const std::string& include_str, std::vector<std::string>& lines, const char* base_dir);
            std::string trim(const std::string& str);
        };
    }
}
