#include "KeyHandler.h"

#include <openssl/rand.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <thread>

namespace fs = std::filesystem;

std::vector<fs::path> find_flash_drives() {
    std::vector<fs::path> usb_drives;

    std::string user = current_user();
    if (user.empty()) {
        std::cout << "Cannot get current user.\n";
        return usb_drives;
    }

    fs::path media_path = fs::path(fedoraFlashDrivesDir) / user;

    while (true) {
        usb_drives.clear();

        if (fs::exists(media_path) && fs::is_directory(media_path)) {
            for (auto &entry : fs::directory_iterator(media_path)) {
                if (fs::is_directory(entry.path())) {
                    usb_drives.emplace_back(entry.path());
                }
            }
        }

        if (!usb_drives.empty()) {
            std::cout << "Detected " << usb_drives.size() << " USB drive(s).\n";
            return usb_drives;
        }

        std::cout << "Waiting for USB drive(s) under: " << media_path << "...\n";
        // Sleep for 2 seconds before checking again
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int load_key() {
    auto usb_drives = find_flash_drives();

    auto meta_file = find_meta_sec_file_on_usb(usb_drives);

    if (!meta_file) {
        std::cout << "Cannot find meta file on any USB drive.\n";
        return LOADING_KEY_FAILED;
    }

    const fs::path& meta_path = *meta_file;

    return load_key_from(meta_path);
}

int load_key_from(const fs::path& meta_file) {
    std::ifstream file(meta_file, std::ios::binary);
    if (!file) {
        std::cout << "Failed to open meta.sec at " << meta_file << "\n";
        return LOADING_KEY_FAILED;
    }

    file.read((char*)(key), KEY_SIZE);

    if (file.gcount() != KEY_SIZE) {
        std::cout << "Failed to read full AES key from meta.sec\n";
        return LOADING_KEY_FAILED;
    }

    return LOAD_KEY_SUCCESS;
}

int create_key() {
    auto usb_drives = find_flash_drives();

    std::cout << "Detected USB drives:\n";
    for (size_t i = 0; i < usb_drives.size(); ++i) {
        std::cout << "  [" << i << "] " << usb_drives[i] << "\n";
    }

    size_t choice = 1;

    if (usb_drives.size() != 1) {
        std::cout << "Select a USB drive to write meta.sec (1-" << usb_drives.size() << "): ";
        while (!(std::cin >> choice) || choice < 1 || choice > usb_drives.size()) {
            std::cout << "Invalid selection. Try again (1-" << usb_drives.size() << "): ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    const fs::path& usb_path = usb_drives[choice - 1];
    fs::path meta_file = usb_path / keyFile;

    std::cout << "Selected USB: " << usb_path << "\n";
    std::cout << "meta.sec will be written to: " << meta_file << "\n";

    if (fs::exists(meta_file)) {
        std::cout << "meta.sec already exists at " << meta_file << "\n";
        std::cout << "Load old meta key from :" << meta_file << "\n";
        load_key_from(meta_file);
        return LOAD_KEY_SUCCESS;
    }

    const int status = dump_key_to(meta_file);
    if ( status == LOAD_KEY_SUCCESS) {
        std::cout << "Key successfully created.\n";
    }
    return status;
}

int dump_key_to(const fs::path& meta_file) {
    // if (!RAND_bytes(key, KEY_SIZE)) {
    //     std::cout << "Failed to generate random AES key.\n";
    //     return LOADING_KEY_FAILED;
    // }

    std::ofstream out(meta_file, std::ios::binary);
    if (!out) {
        std::cout << "Failed to open " << meta_file << " for writing.\n";
        return LOADING_KEY_FAILED;
    }

    out.write((char*)(key), KEY_SIZE);
    out.close();

    chmod(meta_file.c_str(), S_IRUSR | S_IWUSR);

    printKey();
    std::cout << "New AES key generated and saved at: " << meta_file << "\n";
    return LOAD_KEY_SUCCESS;
}