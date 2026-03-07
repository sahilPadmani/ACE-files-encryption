#include <filesystem>
#include <iostream>

#include "encryption/files.h"
#include "KeyHandler/KeyHandler.h"

// ===== Main =====
#include <string>

namespace fs = std::filesystem;

// void computerEncryption() {
//     std::vector<std::string> folders {
//         "/home/sahil/Music",
//         ""
//     };
// }

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage:\n"
                  << " --key                    # Generate AES key\n"
                  << " <--enc|--dec> <folder>   # Encrypt or decrypt folder\n";
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "--key") {
        int status = create_key();
        if (status == LOAD_KEY_SUCCESS) {
            std::cout << "Key successfully created.\n";
        } else {
            std::cerr << "Failed to create key.\n";
            return 1;
        }
        return 0;
    }

    if (argc != 3) {
        std::cerr << "Usage: <--enc|--dec> <folder>\n";
        return 1;
    }

    fs::path folder = argv[2];

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        std::cerr << "Folder does not exist or is not a directory: " << folder << "\n";
        return 1;
    }

    int status = load_key();
    if (status != LOAD_KEY_SUCCESS) {
        std::cerr << "Failed to load AES key from USB.\n";
        return 1;
    }

    fs::path mapping_file = folder / mapping_file_name;

    if (mode == "--enc") {
        std::cout << "Start Encrypting folder: " << folder << "\n";
        start_encrypt_dir(folder , mapping_file);
        std::cout << "Encryption done.\n";
    } else if (mode == "--dec") {
        std::cout << "Start Decrypting folder: " << folder << "\n";
        start_decrypt_dir(folder , mapping_file);
        std::cout << "Decryption done.\n";
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        std::cerr << "Usage: " << argv[0] << " <--enc|--dec> <folder>\n";
        return 1;
    }

    return 0;
}