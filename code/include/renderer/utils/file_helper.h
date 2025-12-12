//
// Created by zhida.ji1 on 2022/8/23.
//

#pragma once
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif
#include <string>
#include <vector>
namespace zr{
    namespace utils{
        class FileHelper{
        public:
#ifdef PLATFORM_ANDROID
            static AAssetManager* asset_mgr;
#endif
            static void set_assert_base_dir(const std::string& dir);
            static std::string assert_base_dir;
        public:
            std::string load_content(const char* path);

            std::vector<std::string> get_lines_from_file(const char* file_path);

            std::string get_dir_name_from_path(const std::string& path);
        };
        
    }
}
