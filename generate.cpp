#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstdlib>

void generateInputFile(const std::string &filename, int fileSizeInBytes)
{
    std::ofstream outputFile(filename);

    if (!outputFile.is_open())
    {
        std::cerr << "Failed to create input file: " << filename << std::endl;
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(97, 122); // ASCII range for lowercase alphabets

    while (fileSizeInBytes > 0)
    {
        char ch = static_cast<char>(distribution(gen));
        outputFile << ch;
        fileSizeInBytes--;
    }

    std::cout << "Successfully generated input file: " << filename << std::endl;
}

int main()
{
    std::string inputFilePath = "input.txt";
    const int fileSizeInBytes = 10 * 1024 * 1024; // 10MB

    generateInputFile(inputFilePath, fileSizeInBytes);

    return 0;
}
