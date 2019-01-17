#include "stdafx.h"
#include "io/DirectorySearcher.h"

void spio::DirectorySearcher::init() {
    // Empty
}

void spio::DirectorySearcher::dispose() {
    Directories().swap(m_directories);
}

bool spio::DirectorySearcher::addDirectory(DirectoryPriority priority, const Directory& directory) {
    // Exceptions suck, just convert to a bool return for now.
    try {
        m_directories.insert(std::make_pair(priority, directory));
    } catch (std::exception& e) {
        return false;
    }
    return true;
}

bool spio::DirectorySearcher::removeDirectory(DirectoryPriority priority, const Directory& directory) {
    // Get the range of elements which have the given priority.
    auto [current, end] = m_directories.equal_range(priority);

    // Iterate over all directories with the given priority and if
    // one of those has the same path as the to-be-removed directory
    // then erase it.
    for (; current != end; ++current) {
        if (current->second == directory) {
            m_directories.erase(current);
            return true;
        }
    }

    return false;
}

bool spio::DirectorySearcher::removeDirectory(const Directory& directory) {
    // Iterate over all directories and if one of those has the same
    // path as the to-be-removed directory then erase it.
    for (auto current = m_directories.begin(); current != m_directories.end(); ++current) {
        if (current->second == directory) {
            m_directories.erase(current);
            return true;
        }
    }

    return false;
}

bool spio::DirectorySearcher::findFile(const std::string& filepath, char*& file) {
    // Iterate over all directories.
    for (auto current = m_directories.begin(); current != m_directories.end(); ++current) {
        Directory& directory = current->second;

        // For the current directory, construct a filesystem path all the way to
        // the searched-for file.
        std::fs::path path(directory + filepath);

        // Determine if this filepath leads to a valid regular file.
        if (std::fs::is_regular_file(path)) {
            // Canonicalise the path (i.e. make it absolute to the hard drive
            // with no "/.." or "/." segments).
            path = std::fs::canonical(path);

            // Get the string of this filepath.
            std::string pathStr = path.string();

            // Copy out the underlying characters into
            // the given buffer.
            file = new char[pathStr.length() + 1];
            std::strcpy(file, pathStr.c_str());

            return true;
        }
    }

    return false;
}

bool spio::DirectorySearcher::findFiles(const std::string& filepath, char** files, ui16 total, ui16& count) {
    // Iterate over all directories.
    count = 0;
    for (auto current = m_directories.begin(); current != m_directories.end(); ++current) {
        Directory& directory = current->second;

        // For the current directory, construct a filesystem path all the way to
        // the searched-for file.
        std::fs::path path(directory + filepath);

        // Determine if this filepath leads to a valid regular file.
        if (std::fs::is_regular_file(path)) {
            // Canonicalise the path (i.e. make it absolute to the hard drive
            // with no "/.." or "/." segments).
            path = std::fs::canonical(path);

            // Get the string of this filepath.
            std::string pathStr = path.string();

            // Copy out the underlying characters into
            // the given buffer.
            files[count] = new char[pathStr.length() + 1];
            std::strcpy(files[count], pathStr.c_str());

            ++count;
        }

        if (count == total) break;
    }

    return count == 0;
}

bool spio::DirectorySearcher::findAllFiles(const std::string& filepath, std::vector<char*>& files, ui16& count) {
    // Reserve enough space in files.
    files.reserve(m_directories.size());

    // Iterate over all directories.
    count = 0;
    for (auto current = m_directories.begin(); current != m_directories.end(); ++current) {
        Directory& directory = current->second;

        // For the current directory, construct a filesystem path all the way to
        // the searched-for file.
        std::fs::path path(directory + filepath);

        // Determine if this filepath leads to a valid regular file.
        if (std::fs::is_regular_file(path)) {
            // Canonicalise the path (i.e. make it absolute to the hard drive
            // with no "/.." or "/." segments).
            path = std::fs::canonical(path);

            // Get the string of this filepath.
            std::string pathStr = path.string();

            // Copy out the underlying characters into
            // the given buffer.
            files.emplace_back(new char[pathStr.length() + 1]);
            std::strcpy(files.back(), pathStr.c_str());

            ++count;
        }
    }

    return count == 0;
}
