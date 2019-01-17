/**
 * @file DirectorySearcher.h
 * @brief Provides a search functionality across a prioritised list of directories.
 */

#pragma once

#if !defined(SP_IO_DirectorySearcher_h__)
#define SP_IO_DirectorySearcher_h__

#include "types.h"

namespace SecretProject {
    namespace io {
        using Directory         = std::string;
        using DirectoryPriority = ui16;
        using Directories       = std::multimap<DirectoryPriority, Directory>;

        /**
         * Class that stores a set of ordered directories in which to search for named files.
         */
        class DirectorySearcher {
        public:
            DirectorySearcher()  { /* Empty */ }
            ~DirectorySearcher() { /* Empty */ }

            /**
             * @brief Initialises the searcher.
             */
            void init();
            /**
             * @brief Disposes the searcher.
             */
            void dispose();

            /**
             * @brief Adds a directory to be searched with the given priority.
             *
             * @param priority The priority of the given directory.
             * @param directory The directory to be added.
             *
             * @return True if the directory was added, false otherwise.
             */
            bool addDirectory(DirectoryPriority priority, const Directory& directory);
            /**
             * @brief Tries to remove the given directory from the searcher
             * if it exists with the given priority.
             *
             * Note: only one instance will be removed, even if multiple instances
             * of the directory have been inserted (while nonsensical, this is
             * not guarded against).
             *
             * @param priority The priority of the given directory.
             * @param directory The directory to be removed.
             *
             * @return True if the directory was removed, false otherwise.
             */
            bool removeDirectory(DirectoryPriority priority, const Directory& directory);
            /**
             * @brief Removes the given directory from the searcher.
             *
             * Note: only one instance will be removed, even if multiple instances
             * of the directory have been inserted (while nonsensical, this is
             * not guarded against).
             *
             * @param directory The directory to be removed.
             *
             * @return True if the directory was removed, false otherwise.
             */
            bool removeDirectory(const Directory& directory);

            /**
             * @brief Finds the first file to exist with the given filepath.
             * 
             * Searching is done on a basis of directories with higher priority
             * preceeding directories of a lower priority.
             *
             * @param filepath The filepath of the file to be found. This will be
             * treated as relative to the directories to be searched.
             * @param file This is set to the absolute path to the file.
             *
             * @return True if at least one file was found.
             * 
             * Note: file is guaranteed not to be changed if no file was found.
             */
            bool findFile(const std::string& filepath, char*& file OUT);
            /**
             * @brief Finds the first count number of files to exist with the given
             * filepath.
             * 
             * 
             * 
             * Searching is done on a basis of directories with higher priority
             * preceeding directories of a lower priority.
             *
             * @param filepath The filepath of the file to be found. This will be
             * treated as relative to the directories to be searched.
             * @param files This is set to the absolute path to the files found.
             * @param total The maximum number of files to be found.
             * @param count This is set to the number of files found. It is guaranteed
             * to be less than or equal to total.
             *
             * @return True if at least one file was found.
             * 
             * Note: files is guaranteed not to be changed if no file was found.
             */
            bool findFiles(const std::string& filepath, char** files, ui16 total, ui16& count);
            /**
             * @brief Finds all files to exist with the given filepath.
             * 
             * Searching is done on a basis of directories with higher priority
             * preceeding directories of a lower priority.
             *
             * @param filepath The filepath of the file to be found. This will be
             * treated as relative to the directories to be searched.
             * @param files This is set to the absolute path to the files found.
             * @param count This is set to the number of files found.
             *
             * @return True if at least one file was found.
             */
            bool findAllFiles(const std::string& filepath, std::vector<char*>& files OUT, ui16& count OUT);
        protected:
            Directories m_directories;
        };
    }
}
namespace spio = SecretProject::io;

#endif // !defined(SP_IO_DirectorySearcher_h__)
