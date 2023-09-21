#pragma once 

#include <bitset>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

const size_t BITES_IN_BYTE = 8;
const size_t BITES_IN_CODED = 12;

bool Calculate(size_t position, const std::bitset<BITES_IN_CODED>& coded);

uint16_t CodeByte(uint8_t data);

void CodeUint64(uint64_t number, std::ofstream& output);

void CurruptedNameMessage(size_t file_number);

int Decode(uint16_t two_bytes);

uint64_t GetFileSize(std::ifstream& file);

std::string GetName(std::ifstream& input);

uint64_t GetUint64(std::ifstream& input);

void WrongPath(const std::string& path);

class HAF_file {
    public: 
        void Append(const std::string& name, const std::string& file_name);
        void Create(uint32_t amount, const std::string& archive_name, const std::vector<std::string>& file_list);
        void Concatenate(const std::vector<std::string>& file_list, const std::string& merged_archive);
        void Delete(const std::string& file_name, const std::string& archive_name);
        void Extract(const std::string& name);
        void ExtractFile(const std::string& file_name, const std::string& archive);
        void ExtractCurrentFile(const std::string& name, uint64_t file_size, std::ifstream& archive);
        void List(const std::string& archive);
};