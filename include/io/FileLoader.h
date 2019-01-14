/**
 * @file FileLoader.h
 * @brief Provides helper functions to load files into buffers.
 */

#pragma once

#if !defined(SP_IO_FileLoader_h__)
#define SP_IO_FileLoader_h__

#include "types.h"

namespace SecretProject {
    namespace io {
        namespace File {
            /**
             * @brief Reads a file at the given filepath into the given buffer.
             *
             * @param filepath The filepath of the file to read.
             * @param buffer The buffer to populate.
             *
             * @return True if read is successful, false otherwise.
             */
            bool read(const char* filepath, char*& buffer);

            /**
             * @brief Reads a file at the given filepath into the given buffer, line-by-line.
             *
             * @param filepath The filepath of the file to read.
             * @param buffer The buffer to populate.
             *
             * @return True if read is successful, false otherwise.
             *
             * @warning This function calls new for each line in the read file - use sparingly.
             */
            bool readByLine(const char* filepath, std::vector<char*>& buffer);
        }
    }
}
namespace spio = SecretProject::io;

#endif // !defined(SP_IO_FileLoader_h__)
