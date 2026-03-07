#ifndef FILESENCRYPTION_FILES_H
#define FILESENCRYPTION_FILES_H

#include <string_view>
#include <filesystem>
#include <unordered_map>
#include <sys/stat.h>

constexpr auto mapping_file_name = "mapping.dec.data";
constexpr auto mapping_file_tmp_name = "mapping.dec.data.tmp";

namespace fs = std::filesystem;

inline std::unordered_map<std::string,std::string> name_mapping;

void start_encrypt_dir(const fs::path &in , const fs::path &mapfile);
void start_decrypt_dir(const fs::path &in , const fs::path &mapfile);

void load_mapping(const fs::path &root);
void save_mapping(const fs::path &root);

std::string encrypt_name(std::string_view name);
bool encrypt_file(const fs::path &in, const fs::path &out);
bool decrypt_file(const fs::path &in, const fs::path &out);
void process_dir_decrypt(const fs::path &in, const fs::path &out);
void process_dir_encrypt(const fs::path &in, const fs::path &out);

inline fs::path make_temp_dir(const fs::path &base, const std::string& suffix = "_tmp") {
    fs::path tmp = base.string() + suffix;
    int counter = 0;
    while (fs::exists(tmp)) {
        ++counter;
        tmp = base.string() + suffix + std::to_string(counter);
    }
    fs::create_directories(tmp);
    return tmp;
}

inline void protect_file(std::string_view path){
    chmod(path.data(), S_IRUSR | S_IWUSR);
}

#endif