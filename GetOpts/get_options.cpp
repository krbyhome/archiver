#include "get_options.h"

void options::Proccess(int args, const char**& console_line) {
    std::string current_opt;
    for (size_t i = 1; i < args; i++) {
        current_opt = console_line[i];
        if (current_opt == "-c" || current_opt == "--create") {
            create = true;
        } else if (current_opt == "-x" || current_opt == "--extract") {
            extract = true;
        } else if (current_opt == "-a" || current_opt == "--append") {
            append_file = true;
        } else if (current_opt == "-d" || current_opt == "--delete") {
            delete_file = true;
        } else if (current_opt == "-A" || current_opt == "--concatenate") {
            concatenate_archives = true;
        } else if (current_opt == "-l" || current_opt == "--list") {
            show_list = true;
        } else if (current_opt.substr(0, 2) == "-f") {
            archive_name = console_line[i + 1];
            i++;
        } else if (current_opt.substr(0, 3) == "--f") {
            std::string pattern = "--file=";
            archive_name = current_opt.substr(pattern.length());
        } else {
            files_inserted++;
            files.emplace_back(current_opt);
        }
    }
}