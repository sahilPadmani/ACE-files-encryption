//
// Created by sahil on 3/7/26.
//

#ifndef FILESENCRYPTION_KEYHANDLER_H
#define FILESENCRYPTION_KEYHANDLER_H

#include <cstring>
#include <string_view>
#include <filesystem>
#include <iostream>
#include <pwd.h>
#include <unistd.h>
#include <vector>

#include "defines.h"
#include "key.h"

namespace fs = std::filesystem;

constexpr auto fedoraFlashDrivesDir = "/run/media";
constexpr auto keyFile = "meta.sec";

inline unsigned char key[KEY_SIZE];

int load_key();
int create_key();
// emergency_key is a hardcoded fallback key used for emergency recovery.
// It is defined in key.h, which is excluded from version control via .gitignore.
inline void load_emergency_key() {
    std::memcpy(key, emergency_key, KEY_SIZE);
}

inline void printKey() {
    std::cout << "{";
    for (size_t i = 0; i < KEY_SIZE; ++i) {
        std::cout << "0x"
                  << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << (int)key[i];

        if (i != KEY_SIZE - 1)
            std::cout << ", ";
    }
    std::cout << "}\n";
}

std::vector<fs::path> find_flash_drives() ;
std::string find_mount(std::string_view device);

inline  std::string current_user() {
    struct passwd *pw = getpwuid(getuid());
    if (!pw) return "";
    return std::string(pw->pw_name);
}
inline std::optional<fs::path> find_meta_sec_file_on_usb(std::span<fs::path> usb_drives) {
    for (const auto& entry : usb_drives) {
        if (fs::path file = entry / keyFile; fs::exists(file)) {
            return file;
        }
    }
    return std::nullopt;
}

#endif //FILESENCRYPTION_KEYHANDLER_H