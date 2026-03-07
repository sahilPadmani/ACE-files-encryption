#include "files.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "../KeyHandler/KeyHandler.h"

namespace fs = std::filesystem;

std::string encrypt_name(std::string_view name) {
    unsigned char hash[16];
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, name.data(), name.size());
    EVP_DigestFinal_ex(ctx, hash, nullptr);
    EVP_MD_CTX_free(ctx);

    char out[33] = {};
    for(int i=0;i<16;i++)
        sprintf(out + i*2,"%02x", hash[i]);
    return std::string(out);
}

bool encrypt_file(const fs::path &in, const fs::path &out) {
    FILE *fin = fopen(in.c_str(), "rb");
    if (!fin) return false;

    FILE *fout = fopen(out.c_str(), "wb");
    if (!fout) { fclose(fin); return false; }

    unsigned char iv[IV_SIZE];
    RAND_bytes(iv, IV_SIZE);
    fwrite(iv, 1, IV_SIZE, fout);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv);

    unsigned char inbuf[BUF_SIZE];
    unsigned char outbuf[BUF_SIZE];
    int outlen = 0, inlen = 0;

    while((inlen = fread(inbuf,1,BUF_SIZE,fin))>0) {
        EVP_EncryptUpdate(ctx, outbuf, &outlen, inbuf, inlen);
        fwrite(outbuf,1,outlen,fout);
    }

    EVP_EncryptFinal_ex(ctx, outbuf, &outlen);
    fwrite(outbuf, 1, outlen, fout);

    unsigned char tag[TAG_SIZE];
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag);
    fwrite(tag, 1, TAG_SIZE, fout);

    EVP_CIPHER_CTX_free(ctx);
    fclose(fin);
    fclose(fout);
    return true;
}

bool decrypt_file(const fs::path &in, const fs::path &out) {
    FILE *fin = fopen(in.c_str(), "rb");
    if (!fin) return false;

    fseek(fin, 0, SEEK_END);
    long filesize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if(filesize < IV_SIZE + TAG_SIZE) { fclose(fin); return false; }

    std::vector<unsigned char> filebuf(filesize);
    fread(filebuf.data(), 1, filesize, fin);
    fclose(fin);

    unsigned char iv[IV_SIZE];
    memcpy(iv, filebuf.data(), IV_SIZE);

    unsigned char tag[TAG_SIZE];
    memcpy(tag, filebuf.data() + filesize - TAG_SIZE, TAG_SIZE);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv);

    std::vector<unsigned char> plaintext(filesize - IV_SIZE - TAG_SIZE);
    int outlen1=0, outlen2=0;

    EVP_DecryptUpdate(ctx, plaintext.data(), &outlen1,
                      filebuf.data() + IV_SIZE, filesize - IV_SIZE - TAG_SIZE);

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag);

    if(EVP_DecryptFinal_ex(ctx, plaintext.data() + outlen1, &outlen2) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        std::cerr << "Integrity check failed: " << in << "\n";
        return false;
    }

    EVP_CIPHER_CTX_free(ctx);

    FILE *fout = fopen(out.c_str(), "wb");
    if(!fout) return false;

    fwrite(plaintext.data(), 1, outlen1 + outlen2, fout);
    fclose(fout);
    return true;
}

// ===== Directory processing =====
void process_dir_encrypt(const fs::path &in, const fs::path &out) {
    fs::create_directories(out);
    for(auto &entry : fs::directory_iterator(in)) {
        std::string encname = encrypt_name(entry.path().filename().string());
        fs::path outpath = out / encname;
        name_mapping[encname] = entry.path().filename().string();

        if(entry.is_directory())
            process_dir_encrypt(entry.path(), outpath);
        else
            encrypt_file(entry.path(), outpath);
    }
}

void process_dir_decrypt(const fs::path &in, const fs::path &out) {
    fs::create_directories(out);
    for(auto &entry : fs::directory_iterator(in)) {

        if (entry.path().filename().compare("mapping.dec.data") == 0)
            continue;

        std::string orig = name_mapping.count(entry.path().filename().string()) ?
                           name_mapping[entry.path().filename().string()] :
                           entry.path().filename().string();
        fs::path outpath = out / orig;
        if(entry.is_directory())
            process_dir_decrypt(entry.path(), outpath);
        else
            decrypt_file(entry.path(), outpath);
    }
}

// ===== Mapping file =====
void save_mapping(const fs::path &root) {
    fs::path mapfile = root / mapping_file_name;
    fs::path tmpfile = root / mapping_file_tmp_name;  // <-- temp file path

    std::ofstream tmp(tmpfile);
    for(auto &[enc, orig] : name_mapping)
        tmp << std::format("{}={}\n", enc, orig);
    tmp.close();

    encrypt_file(tmpfile, mapfile);

    // Remove temporary plaintext
    fs::remove(tmpfile);
}

void load_mapping(const fs::path &mapfile) {
    name_mapping.clear();

    fs::path tmp = mapfile.string() + ".tmp";
    if(!decrypt_file(mapfile, tmp)) {
        std::cerr << "Failed to decrypt mapping file: " << mapfile << "\n";
        return;
    }

    // Read decrypted mapping
    std::ifstream f(tmp);
    std::string line;
    while(std::getline(f,line)) {
        auto pos = line.find('=');
        if(pos != std::string::npos)
            name_mapping[line.substr(0,pos)] = line.substr(pos+1);
    }
    f.close();

    fs::remove(tmp);
}

void start_encrypt_dir(const fs::path &in , const fs::path& mapping_file) {
    if (fs::exists(mapping_file)) {
        std::cout << "File already exists: " << mapping_file << "\n";
        return;
    }

    name_mapping.clear();
    const fs::path tmpFolder = make_temp_dir(in, "_enc_tmp");

    try {
        process_dir_encrypt(in, tmpFolder);

        fs::path backup = in;
        backup += ".bak";

        fs::rename(in, backup);
        fs::rename(tmpFolder, in);

        save_mapping(in);

        fs::remove_all(backup);
        protect_file(mapping_file.c_str());
    }
    catch (...) {
        if (fs::exists(tmpFolder))
            fs::remove_all(tmpFolder);
        throw;
    }
}

void start_decrypt_dir(const fs::path &in , const fs::path& mapping_file) {

    if (!fs::exists(mapping_file)) {
        std::cout << "File not exists: " << mapping_file << "\n";
        return;
    }

    const fs::path tmpFolder = make_temp_dir(in, "_dec_tmp");

    try {
        load_mapping(mapping_file);

        process_dir_decrypt(in, tmpFolder);

        fs::path backup = in;
        backup += ".bak";

        fs::rename(in, backup);
        fs::rename(tmpFolder, in);

        fs::remove_all(backup);
        fs::remove(mapping_file);
    }
    catch (...) {
        if (fs::exists(tmpFolder))
            fs::remove_all(tmpFolder);
        throw;
    }
}