#include <filesystem>
#include <iostream>

#include "encryption/files.h"
#include "KeyHandler/KeyHandler.h"

// ===== Main =====
#include <string>

namespace fs = std::filesystem;

void computerEncryption(const bool decryption = false) {
    const std::vector<std::string_view> folders {
        "/home/sahil/Music",
        "/home/sahil/Documents",
        "/home/sahil/Desktop/dd",
        "/home/sahil/Old E",
        "/home/sahil/Videos/jd",
        "/home/sahil/Videos/childhood"
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

// int main(int argc, char** argv) {
//
//     if (argc < 2) {
//         std::cout << "Usage:\n"
//                   << " -key                    # Generate AES key\n"
//                   << " <-enc|-dec> <folder>   # Encrypt or decrypt folder\n";
//         return 1;
//     }
//
//     std::string mode = argv[1];
//
//     if (argc == 4 && strcmp(argv[3],"-e") == 0 ) {
//         std::cout << "fallback Key is use.\n";
//         load_emergency_key();
//         goto skip_key_loading;
//     }
//
//     if (mode == "-key") {
//         int status = create_key();
//         if (status == LOAD_KEY_SUCCESS) {
//             std::cout << "Key successfully created.\n";
//         } else {
//             std::cout << "Failed to create key.\n";
//             return 1;
//         }
//         return 0;
//     }
//
//     if (load_key() != LOAD_KEY_SUCCESS) {
//         std::cout << "Failed to load AES key from USB.\n";
//         return 1;
//     }
//
//     if (mode == "-printKey"){
//         printKey();
//         return 0;
//     }
//
//     if (mode == "-enc-com") {
//         computerEncryption(false);
//         return 0;
//     }
//
//     if (mode == "-dec-com") {
//         computerEncryption(true);
//         return 0;
//     }
//
//     if (argc != 3) {
//         std::cout << "Usage: <-enc|-dec> <folder>\n";
//         return 1;
//     }
//
//     skip_key_loading:
//
//     fs::path folder = argv[2];
//
//     if (!fs::exists(folder) || !fs::is_directory(folder)) {
//         std::cout << "Folder does not exist or is not a directory: " << folder << "\n";
//         return 1;
//     }
//
//     fs::path mapping_file = folder / mapping_file_name;
//
//     if (mode == "-enc") {
//         std::cout << "Start Encrypting folder: " << folder << "\n";
//         start_encrypt_dir(folder , mapping_file);
//         std::cout << "Encryption done.\n";
//     } else if (mode == "-dec") {
//         std::cout << "Start Decrypting folder: " << folder << "\n";
//         start_decrypt_dir(folder , mapping_file);
//         std::cout << "Decryption done.\n";
//     } else {
//         std::cout << "Unknown mode: " << mode << "\n";
//         std::cout << "Usage: " << argv[0] << " <-enc|-dec> <folder>\n";
//         return 1;
//     }
//
//     return 0;
// }



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

        if (!key_loaded && input != "-key") {
            std::cout << "AES key not loaded! Use -loadKey first.\n";
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