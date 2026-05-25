#include <filesystem>
#include <iostream>

#include "encryption/files.h"
#include "KeyHandler/KeyHandler.h"

#include <string>

namespace fs = std::filesystem;

void computerEncryption(const bool decryption = false) {
    const std::vector<std::string_view> folders {
        
    };

    for (const auto& folder : folders) {
        fs::path folder_path = folder;
        fs::path mapping_file = folder_path / mapping_file_name;

        if (decryption)
            start_decrypt_dir(folder_path, mapping_file);
        else
            start_encrypt_dir(folder_path, mapping_file);

        std::cout << "Folder done:" << folder_path << "\n";
    }
}

int main() {
    bool key_loaded = false;
    std::string input;

    std::cout << "Interactive AES Encryption CLI\n";
    std::cout << "Type -help for commands.\n";

    do{
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "-exit") {
            std::cout << "Exiting CLI.\n";
            break;
        }

        if (input == "-help") {
            std::cout << "Commands:\n"
                      << " -createKey         : Generate AES key\n"
                      << " -printKey          : Print loaded AES key\n"
                      << " -loadKey           : Load AES key from USB (once)\n"
                      // << " -emergencyKey      : Load fallback/emergency key\n"
                      << " -enc <folder>      : Encrypt folder\n"
                      << " -dec <folder>      : Decrypt folder\n"
                      << " -enc-com           : Encrypt predefined folders\n"
                      << " -dec-com           : Decrypt predefined folders\n"
                      << " -exit              : Exit CLI\n"
                      << " -clear             : Clear CLI\n"
                      << " -clearKey          : Clear Key Context\n";
            continue;
        }

        if (input == "-clear") {
            system("clear");
            continue;
        }

        if (input == "-clearKey") {
            clear_key();
            std::cout << "Clear key context.\n";
            key_loaded = false;
            continue;
        }

        // Generate key
        if (input == "-createKey") {
            int status = create_key();
            if (status == LOAD_KEY_SUCCESS) {
                key_loaded = true;
            } else {
                std::cout << "Failed to create key.\n";
            }
            continue;
        }

        // Load key once
        if (input == "-loadKey") {
            if (load_key() == LOAD_KEY_SUCCESS) {
                std::cout << "Key loaded successfully.\n";
                key_loaded = true;
            } else {
                std::cout << "Failed to load AES key from USB.\n";
            }
            continue;
        }
        
        // Load emergency/fallback key
        if (input == "-emergencyKey") {
            if (load_emergency_key() == LOADING_KEY_FAILED) {
                continue;
            }
            std::cout << "Emergency key loaded.\n";
            key_loaded = true;
            continue;
        }

        if (!key_loaded) {
            std::cout << "AES key not loaded! Use -loadKey or -createKey first.\n";
            continue;
        }

        // Print key
        if (input == "-printKey") {
            printKey();
            continue;
        }

        // Predefined folder encryption
        if (input == "-enc-com") {
            computerEncryption(false);
            continue;
        }

        // Predefined folder decryption
        if (input == "-dec-com") {
            computerEncryption(true);
            continue;
        }

        std::string folder = input.substr(5);
        fs::path folder_path = folder;

        if (!fs::exists(folder_path) || !fs::is_directory(folder_path)) {
            std::cout << "Folder does not exist or is not a directory: " << folder << "\n";
            continue;
        }

        fs::path mapping_file = folder_path / mapping_file_name;

        // Folder-specific commands
        if (input.rfind("-enc ", 0) == 0) {
            std::cout << "Start Encrypting folder: " << folder << "\n";
            start_encrypt_dir(folder_path, mapping_file);
            std::cout << "Encryption done.\n";
            continue;
        }

        if (input.rfind("-dec ", 0) == 0) {
            std::cout << "Start Decrypting folder: " << folder << "\n";
            start_decrypt_dir(folder_path, mapping_file);
            std::cout << "Decryption done.\n";
            continue;
        }

        std::cout << "Unknown command. Type -help for list of commands.\n";
    }while (true);

    return 0;
}