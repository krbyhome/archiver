#include "haf.h"

bool Calculate(size_t pos, const std::bitset<BITES_IN_CODED>& coded) {
    bool ctrl_bit = false;
    for (int i = BITES_IN_CODED - pos; i >= 0; i -= 2*pos) {
         for (int j = i; j > i - static_cast<int>(pos) && j >= 0; j--) {
            ctrl_bit = ctrl_bit ^ coded[j];
        }
    }

    return ctrl_bit;
}

uint16_t CodeByte(uint8_t data) {
    uint16_t coded_result = 0;
    std::bitset<BITES_IN_BYTE> byte = data; 
    std::bitset<BITES_IN_CODED> coded_byte;
    size_t power_of_two = 1;
    size_t data_point = 1;
    for (size_t i = 1; i <= BITES_IN_CODED; i++) {
        if (i == power_of_two) {
            power_of_two *= 2;
        } else {
            coded_byte[BITES_IN_CODED - i] = byte[BITES_IN_BYTE - data_point];
            data_point++;
        }
    }

    for (size_t i = 1; i <= BITES_IN_BYTE; i*=2) {
        coded_byte[BITES_IN_CODED - i] = Calculate(i, coded_byte);
    }

    power_of_two = 1;
    for (size_t i = 0; i < BITES_IN_CODED; i++) {
        coded_result += (size_t)coded_byte[i] * power_of_two;
        power_of_two *= 2;
    }

    return coded_result;
}

void CodeUint64(uint64_t file_size, std::ofstream& output) {
    for (size_t i = 0; i < sizeof(file_size); i++) {
        uint16_t two_coded_bytes = CodeByte(file_size%(UINT8_MAX + 1));
        output.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(uint16_t));
        file_size /= (UINT8_MAX + 1);
    }
}

void CurruptedNameMessage(size_t file_number) {
    if (file_number == 1) {
        std::cout << "1'st file is corrupted\n";
    } else if (file_number == 2) {
        std::cout << "2'nd file is corrupted\n";
    } else if (file_number== 3) {
        std::cout << "3'rd file is corrupted\n";
    } else {
        std::cout << file_number << "'th file is corrupted\n"; 
    }
}

int Decode(uint16_t two_bytes) {
    uint8_t byte;
    byte = 0;
    std::bitset<BITES_IN_CODED> coded_byte;
    std::bitset<BITES_IN_CODED> coded_byte_mask;
    std::bitset<BITES_IN_BYTE> decoded_byte;
    coded_byte = two_bytes;
    coded_byte_mask = two_bytes;

    size_t control_sum;
    control_sum = 0;
    for (int i = 1; i <= BITES_IN_CODED; i *= 2) {
        coded_byte_mask[BITES_IN_CODED - i] = 0;
        bool current_bit = Calculate(i, coded_byte_mask);
        if (current_bit != coded_byte[BITES_IN_CODED - i]) {
            control_sum += i;
        }
    }

    if (control_sum <= BITES_IN_CODED) {
        if (control_sum != 0) {
            coded_byte[BITES_IN_CODED - control_sum] = (coded_byte[BITES_IN_CODED - control_sum] + 1) % 2;
        }

        size_t data_point;
        data_point = 1;
        size_t power_of_two;
        power_of_two = 1;
        for (size_t i = 1; i <= BITES_IN_CODED; i++) {
            if (i == power_of_two) {
                power_of_two *= 2;
            } else {
                decoded_byte[BITES_IN_BYTE - data_point] = coded_byte[BITES_IN_CODED - i];
                data_point++;
            }
        }

        for (size_t i = 1; i <= BITES_IN_BYTE; i++) {
            byte += decoded_byte[BITES_IN_BYTE - i] << (BITES_IN_BYTE - i);
        }

        return byte;
    } else {
        return -1;
    }
}

uint64_t GetFileSize(std::ifstream& file) {
    uint64_t size;
    file.seekg(0, std::ios::end);
    size = file.tellg();
    file.seekg(0, std::ios::beg);

    return size;
}

std::string GetName(std::ifstream& input) {
    std::string name;
    uint8_t symbol;
    uint16_t coded_byte;
    int decoded_byte = 0;
    while(static_cast<char>(decoded_byte) != '\n') {
        input.read(reinterpret_cast<char*>(&coded_byte), sizeof(coded_byte));
        int decoded_byte;
        decoded_byte = Decode(coded_byte);
        if (decoded_byte == -1) {
            return "currupted";
        } else if (static_cast<char>(decoded_byte) != '\n') {
            name += static_cast<char>(decoded_byte);
        } else {
            break;
        }
    }

    return name;
}

uint64_t GetUint64(std::ifstream& input) {
    uint64_t result = 0;
    uint64_t power_of_256 = 1;
    uint16_t coded_byte;
    uint8_t symbol;
    for (size_t i = 0; i < sizeof(uint64_t); i++) {
        input.read(reinterpret_cast<char*>(&coded_byte), sizeof(coded_byte));
        int decoded_byte;
        decoded_byte = Decode(coded_byte);
        if (decoded_byte == -1) {
            return 0;
        } else {
            result += power_of_256 * static_cast<uint64_t>(decoded_byte);
            power_of_256 *= 256;
        }
    }

    return result;
}

void WrongPath(const std::string& path) {
    std::cout << "Couldn't find a path to " << path << "\n";
}

void HAF_file::Append(const std::string& name, const std::string& file_name) {
    std::ofstream out_arch(name, std::ios::app | std::ios::binary);
    if (!out_arch) {
        WrongPath(name);
    }
    std::ifstream file_input(file_name, std::ios::binary);
    if (!file_input) {
        WrongPath(file_name);
        return;
    }

    uint64_t file_size;
    file_size = GetFileSize(file_input) + file_name.length() + sizeof('\n');
    CodeUint64(file_size, out_arch);

    uint16_t two_coded_bytes;
    for (size_t i = 0; i < file_name.length(); i++) {
        two_coded_bytes = CodeByte(file_name[i]);
        out_arch.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(two_coded_bytes));
    }
    char newline = '\n';
    two_coded_bytes = CodeByte(newline);
    out_arch.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(two_coded_bytes));

    file_size -= file_name.length() + sizeof('\n');
    char symbol;
    for (uint64_t i = 0; i < file_size; i++) {
        file_input.get(symbol);
        two_coded_bytes = CodeByte(symbol);
        out_arch.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(two_coded_bytes));
    }
}

void HAF_file::Concatenate(const std::vector<std::string>& file_list, const std::string& merged_archive) {
    std::ofstream concatenated_archive(merged_archive, std::ios::binary);
    for (size_t i = 0; i < file_list.size(); i++) {
        std::ifstream archive(file_list[i], std::ios::binary);
        char current_byte;
        while (archive.get(current_byte)) {
            concatenated_archive.write(reinterpret_cast<char*> (&current_byte), sizeof(current_byte));
        }
    }
}

void HAF_file::Create(uint32_t amount, const std::string& archive_name, const std::vector<std::string>& file_list) {
    std::ofstream output_archive(archive_name + ".haf", std::ios::binary);
    if (!output_archive) {
        WrongPath(archive_name);
    }

    for (size_t i = 0; i < amount; i++) {
        std::ifstream insert(file_list[i], std::ios::binary);
        if (!insert) {
            WrongPath(file_list[i]);

            continue;
        }

        uint64_t current_file_size;
        current_file_size = GetFileSize(insert) + file_list[i].length() + sizeof('\n');
        CodeUint64(current_file_size, output_archive);

        uint16_t two_coded_bytes;
        for (size_t j = 0; j < file_list[i].length(); j++) {
            char current = file_list[i][j];
            two_coded_bytes = CodeByte(current);
            output_archive.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(uint16_t));
        }
        char newline = '\n';
        two_coded_bytes = CodeByte(newline);
        output_archive.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(uint16_t));

        current_file_size -= file_list[i].length() + sizeof('\n');
        char current_symbol;
        for (uint64_t i = 0; i < current_file_size; i++) {
            insert.get(current_symbol);
            two_coded_bytes = CodeByte(current_symbol);
            output_archive.write(reinterpret_cast<char*>(&two_coded_bytes), sizeof(uint16_t));
        }
    }
}

void HAF_file::Delete(const std::string& file_name, const std::string& archive_name) {
    std::ifstream archive(archive_name, std::ios::binary);
    if (!archive) {
        WrongPath(archive_name);
    }
    const std::string temporary_archive_name = "temporary_for_deleting.haf";
    std::ofstream temporary_archive(temporary_archive_name, std::ios::binary);

    uint64_t archive_size;
    archive_size = GetFileSize(archive);

    size_t file_counter = 0;
    uint64_t current_file_pos = 0;
    uint64_t current_file_size;
    std::string current_file_name;
    while (current_file_pos != archive_size) {
        file_counter++;
        archive.seekg(current_file_pos);
        current_file_size = GetUint64(archive);
        if (current_file_size == 0) {
            std::cout << "archive is corrupted\n";
            return;
        }
        current_file_name = GetName(archive);
        if (current_file_name == "currupted") {
            CurruptedNameMessage(file_counter);
        } else if (current_file_name == file_name) {
            current_file_pos += 2 * (current_file_size + sizeof(current_file_size));
            continue;
        } else {
            archive.seekg(current_file_pos);
            current_file_pos += 2 * (current_file_size + sizeof(current_file_size));
            char current_byte;
            for (uint64_t i = 0; i < 2 * (current_file_size + sizeof(current_file_size)); i++) {
                archive.get(current_byte);
                temporary_archive.write(reinterpret_cast<char*> (&current_byte), sizeof(current_byte));
            }
        }
    }
    archive.close();
    temporary_archive.close();
    remove(archive_name.c_str());
    rename(temporary_archive_name.c_str(), archive_name.c_str());
}

void HAF_file::Extract(const std::string& name) {
    std::ifstream get_arch(name, std::ios::binary);
    if (!get_arch) {
        WrongPath(name);
        return;
    }

    uint64_t archive_size;
    archive_size = GetFileSize(get_arch);
    
    std::string file_name;
    size_t file_counter = 0;
    uint64_t current_file_size;
    while (static_cast<uint64_t>(get_arch.tellg()) != archive_size) {
        file_counter++;
        current_file_size = GetUint64(get_arch);
        if (current_file_size == 0) {
            std::cout << "archive is corrupted\n";
            return;
        }
        file_name = GetName(get_arch);
        std::cout << "Extracting " << file_name << "...\n";
        current_file_size -= file_name.length() + sizeof('\n');
        if (file_name == "currupted") {
            CurruptedNameMessage(file_counter);
        } else {
            ExtractCurrentFile(file_name, current_file_size, get_arch);
        }
    }
}

void HAF_file::ExtractFile(const std::string& file_name, const std::string& archive) {
    std::ifstream get_arch(archive, std::ios::binary);
    if (!get_arch) {
        WrongPath(archive);
    }

    uint64_t archive_size;
    archive_size = GetFileSize(get_arch);

    std::string current_file_name;
    size_t file_counter = 0;
    uint64_t current_file_size;
    while (get_arch.tellg() != archive_size) {
        file_counter++;
        current_file_size = GetUint64(get_arch);
        if (current_file_size == 0) {
            std::cout << "archive is corrupted\n";
            return;
        }
        current_file_name = GetName(get_arch);
        current_file_size -= file_name.length() + sizeof('\n');
        if (current_file_name == "currupted") {
            CurruptedNameMessage(file_counter);
        } else if (current_file_name == file_name) {
            ExtractCurrentFile(file_name, current_file_size, get_arch);
            return;
        }
        get_arch.seekg(static_cast<uint64_t>(get_arch.tellg()) + 2 * current_file_size);
    }
}

void HAF_file::ExtractCurrentFile(const std::string& name, uint64_t file_size, std::ifstream& archive) {
    std::ofstream extract(name, std::ios::binary);
    uint16_t coded_byte;
    char symbol;
    for (size_t i = 0; i < file_size; i++) {
        archive.read(reinterpret_cast<char*>(&coded_byte), sizeof(coded_byte));
        int decoded_byte;
        decoded_byte = Decode(coded_byte);
        if (decoded_byte == -1) {
            std::cout << name << " is corrupted\n";

            return;
        } else {
            extract.write(reinterpret_cast<char*>(&decoded_byte), sizeof(char));
        }
    }
}

void HAF_file::List(const std::string& archive) {
    std::ifstream get_arch(archive, std::ios::binary);
    if (!get_arch) {
        WrongPath(archive);
    }

    uint64_t archive_size;
    archive_size = GetFileSize(get_arch);

    std::string file_name;
    size_t file_counter = 0;
    uint64_t current_file_size;
    while (get_arch.tellg() != archive_size) {
        file_counter++;
        current_file_size = GetUint64(get_arch);
        if (current_file_size == 0) {
            std::cout << "archive is corrupted\n";
            return;
        }
        file_name = GetName(get_arch);
        current_file_size -= file_name.length() + sizeof('\n');
        if (file_name == "currupted") {
            CurruptedNameMessage(file_counter);
        } else {
            std::cout << file_counter << ". " << file_name << "\n";
        }
        get_arch.seekg(static_cast<uint64_t>(get_arch.tellg()) + 2 * current_file_size);
    }
}
