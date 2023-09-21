#pragma once 

#include <iostream>
#include <string>
#include <vector>

class options {
    public: 
        bool append_file;
        bool concatenate_archives;
        bool create;
        bool delete_file;
        bool extract;
        size_t files_inserted;
        bool show_list;
        std::string archive_name;
        std::vector<std::string> files;

        void Proccess(int args, const char**& console_line);

        options(): append_file(0), concatenate_archives(0), create(0), delete_file(0), extract(0), files_inserted(0), show_list(0){ };
};