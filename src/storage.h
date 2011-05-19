// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

// Functions for accessing persistent storage

namespace circa {

std::string get_directory_for_filename(std::string const& filename);
bool is_absolute_path(std::string const& path);
std::string get_absolute_path(std::string const& path);

namespace storage {

// Callback used in read_text_file. If the file is read successfully, then 'contents' will
// contain its full contents. If there is an error, 'contents' will be NULL and 'error' will
// have a human-readable description of the error.
typedef void (*FileReceiveFunc)(void* context, const char* contents, const char* error);

typedef void (*ReadTextFile)(const char* filename, FileReceiveFunc receiveFile, void* context);
typedef void (*WriteTextFile)(const char* filename, const char* contents);
typedef time_t (*GetModifiedTime)(const char* filename);
typedef bool (*FileExists)(const char* filename);

void read_text_file(const char* filename, FileReceiveFunc receiveFile, void* context);
void write_text_file(const char* filename, const char* contents);
time_t get_modified_time(const char* filename);
bool file_exists(const char* filename);

void read_text_file_to_value(const char* filename, Value* contents, Value* error);
std::string read_text_file_as_str(const char* filename);

struct StorageInterface {
    ReadTextFile readTextFile;
    WriteTextFile writeTextFile;
    GetModifiedTime getModifiedTime;
    FileExists fileExists;
};

// Install the provided storage interface
void install_storage_interface(StorageInterface* interface);

// Copy the current storage interface to the given argument
void get_current_storage_interface(StorageInterface* interface);

} // namespace storage
} // namespace circa

// Install a builtin interface that just uses the filesystem in a standard way
export_func void circa_storage_use_filesystem();
