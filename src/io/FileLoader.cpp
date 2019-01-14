#include "stdafx.h"
#include "io/FileLoader.h"

bool spio::File::read(const char* filepath, char*& buffer) {
    // Open file.
    std::ifstream file(filepath, std::ios::binary);
    if (file.fail()) return false;

    // Find position of end of file.
    file.seekg(0, std::ios::end);
    std::streamoff size = file.tellg();
    // Determine size of contents of file by subtracting end of file header.
    file.seekg(0, std::ios::beg);
    size -= file.tellg();

    // Construct buffer of appropriate size.
    buffer = new char[static_cast<size_t>(size) + 1];

    // Read in file to buffer, null-terminating the string.
    file.read(buffer, size);
    buffer[size] = '\0';

    // Close file and return.
    file.close();
    return true;
}

bool spio::File::readByLine(const char* filepath, std::vector<char*>& buffer) {
    // Open file.
    std::ifstream file(filepath, std::ios::binary);
    if (file.fail()) return false;

    // Clear buffer.
    std::vector<char*>().swap(buffer);

    // Copy line-by-line into buffer.
    std::string line;
    while (std::getline(file, line)) {
        line += '\n';

        char* linebuffer = new char[line.size() + 1];
        memcpy(linebuffer, reinterpret_cast<void*>(const_cast<char*>(line.c_str())), line.size());
        linebuffer[line.size()] = '\0';

        buffer.push_back(linebuffer);
    }

    // Close file.
    file.close();
    return true;
}
