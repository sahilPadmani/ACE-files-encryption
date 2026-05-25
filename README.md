# filesEncryption

A C++ command-line tool for encrypting and decrypting folders with OpenSSL AES-256-GCM.

The program encrypts file contents, replaces file and directory names with SHA-256-based hashes, and stores the original-name mapping in an encrypted `mapping.dec.data` file inside the encrypted folder.

## Features

- AES-256-GCM encryption for file contents
- Random IV per encrypted file
- GCM authentication tag verification during decryption
- Encrypted file and directory name mapping
- USB-based key loading through `meta.sec`
- Interactive CLI
- Basic preservation of file permissions

## Project Structure

```text
.
├── CMakeLists.txt
├── main.cpp
├── KeyHandler/
│   ├── defines.h
│   ├── KeyHandler.h
│   └── KeyHandler.cpp
└── encryption/
    ├── files.h
    └── files.cpp
```

## Requirements

- Linux
- CMake 4.1 or newer
- C++23 compiler
- OpenSSL development libraries

On Fedora, the OpenSSL development package is usually:

```bash
sudo dnf install openssl-devel
```

On Ubuntu/Debian:

```bash
sudo apt install libssl-dev
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

The executable will be created at:

```text
build/filesEncryption
```

## Run

```bash
./build/filesEncryption
```

The app starts an interactive prompt:

```text
Interactive AES Encryption CLI
Type -help for commands.
>
```

## Commands

```text
-help              Show command list
-createKey         Create or load meta.sec on a detected USB drive
-loadKey           Load AES key from meta.sec on a detected USB drive
-printKey          Print the currently loaded key
-enc <folder>      Encrypt a folder
-dec <folder>      Decrypt a folder
-enc-com           Encrypt predefined folders from main.cpp
-dec-com           Decrypt predefined folders from main.cpp
-clear             Clear the terminal
-clearKey          Clear the key from memory
-exit              Exit the CLI
```

There is also an implemented `-emergencyKey` command, although it is not shown in the CLI help text. It loads a fallback key from the hardcoded path:

```text
/home/sahil/Vaults/password/meta.sec
```

## Key File

The key file is named:

```text
meta.sec
```

The program searches for USB drives under:

```text
/run/media/<current-user>/
```

When loading a key, it looks for `meta.sec` on the detected USB drive.

## Encrypting a Folder

Inside the CLI:

```text
-loadKey
-enc /path/to/folder
```

During encryption:

- A temporary encrypted folder is created next to the original folder.
- The original folder is renamed to `<folder>.bak`.
- The encrypted folder replaces the original folder path.
- The encrypted name mapping is saved as `mapping.dec.data`.
- The backup folder is removed after successful encryption.

## Decrypting a Folder

Inside the CLI:

```text
-loadKey
-dec /path/to/folder
```

During decryption:

- `mapping.dec.data` is decrypted and loaded.
- A temporary decrypted folder is created.
- The encrypted folder is renamed to `<folder>.bak`.
- The decrypted folder replaces the original folder path.
- The backup folder and mapping file are removed after successful decryption.

## Important Notes

- Keep `meta.sec` safe. Without the correct key, encrypted data cannot be decrypted.
- Do not delete `mapping.dec.data` from an encrypted folder. It is required to restore original file and directory names.
- The project currently has key generation disabled in `dump_key_to()` because the `RAND_bytes()` call is commented out. As written, `-createKey` writes the current in-memory key value instead of generating a fresh random key.
- Test with disposable folders before using this on important data.

