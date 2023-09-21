#include "../GetOpts/get_options.cpp"
#include "../haf/haf.cpp"

int main(int argc, const char** argv) {
    options opts;
    opts.Proccess(argc, argv);
    HAF_file archive;
    if (opts.append_file) {
        archive.Append(opts.archive_name, opts.files[0]);
    } else if (opts.create) {
        archive.Create(opts.files_inserted, opts.archive_name, opts.files);
    } else if (opts.concatenate_archives) {
        archive.Concatenate(opts.files, opts.archive_name);
    } else if (opts.delete_file) {
        archive.Delete(opts.files[0], opts.archive_name);
    } else if (opts.extract && opts.files_inserted == 0) {
        archive.Extract(opts.archive_name);
    } else if (opts.extract) {
        archive.ExtractFile(opts.files[0], opts.archive_name);
    } else if (opts.show_list) {
        archive.List(opts.archive_name);
    } else {
        std::cout << "Wrong input\n";
    }
    
    return 0;
}
