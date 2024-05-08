#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <cstring>
#include <filesystem>
#include <string>
#include <mutex>

const int CHUNK_SIZE = 1024 * 1024; // 1MB

void splitFile(const std::string &inputFilePath, const std::string &outputPrefix)
{
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open input file." << std::endl;
        return;
    }

    std::vector<char> buffer(CHUNK_SIZE);
    int fileIndex = 0;

    while (inputFile.read(buffer.data(), CHUNK_SIZE))
    {
        std::streamsize bytesRead = inputFile.gcount();

        if (bytesRead > 0)
        {
            std::string outputFileName = outputPrefix + "_" + std::to_string(fileIndex++);
            std::ofstream outputFile(outputFileName, std::ios::binary);
            if (!outputFile.is_open())
            {
                std::cerr << "Failed to create output file: " << outputFileName << std::endl;
                continue;
            }
            outputFile.write(buffer.data(), bytesRead);
        }
    }

    inputFile.close();
}

void processFile(const std::string &inputFilePath, const std::string &outputFilePath)
{
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open input file: " << inputFilePath << std::endl;
        return;
    }

    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to create output file: " << outputFilePath << std::endl;
        return;
    }

    std::vector<char> buffer(CHUNK_SIZE);
    while (inputFile.read(buffer.data(), CHUNK_SIZE))
    {
        std::streamsize bytesRead = inputFile.gcount();

        // Process each character in the buffer
        for (std::streamsize i = 0; i < bytesRead; ++i)
        {
            buffer[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(buffer[i])));
        }

        outputFile.write(buffer.data(), bytesRead);
    }

    inputFile.close();
    outputFile.close();
}

void splitAndProcessFiles(const std::string &inputFilePath, const std::string &outputPrefix)
{
    splitFile(inputFilePath, outputPrefix);

    std::vector<std::thread> processThreads;
    int fileIndex = 0;

    while (true)
    {
        std::string inputFileName = outputPrefix + "_" + std::to_string(fileIndex);
        std::string outputFileName = outputPrefix + "_processed_" + std::to_string(fileIndex++);

        if (!std::filesystem::exists(inputFileName))
        {
            break; // No more split files to process
        }

        processThreads.emplace_back(processFile, inputFileName, outputFileName);
    }

    for (std::thread &thread : processThreads)
    {
        thread.join();
    }
}

void mergeFiles(const std::vector<std::string> &inputFilePaths, const std::string &outputFilePath)
{
    std::ofstream outputFile(outputFilePath, std::ios::binary);

    if (!outputFile.is_open())
    {
        std::cerr << "Failed to create output file: " << outputFilePath << std::endl;
        return;
    }

    std::mutex mergeMutex; // 创建互斥锁

    std::vector<std::thread> mergeThreads;
    for (const std::string &inputFilePath : inputFilePaths)
    {
        mergeThreads.emplace_back([&outputFile, &mergeMutex, inputFilePath]()
                                  {
            std::ifstream inputFile(inputFilePath, std::ios::binary);
            if (!inputFile.is_open())
            {
                std::cerr << "Failed to open input file: " << inputFilePath << std::endl;
                return;
            }

            std::vector<char> buffer(CHUNK_SIZE);
            while (inputFile.read(buffer.data(), CHUNK_SIZE))
            {
                std::streamsize bytesRead = inputFile.gcount();

                std::lock_guard<std::mutex> lock(mergeMutex); // 加锁

                outputFile.write(buffer.data(), bytesRead);
            }

            inputFile.close(); });
    }

    for (std::thread &thread : mergeThreads)
    {
        thread.join();
    }

    outputFile.close();
}

int main()
{
    std::string inputFilePath = "input.txt";
    std::string outputPrefix = "output";
    std::string outputMergedFile = "merged_output.txt";

    splitAndProcessFiles(inputFilePath, outputPrefix);

    std::vector<std::string> inputFilePaths;
    int fileIndex = 0;

    while (true)
    {
        std::string outputProcessedFileName = outputPrefix + "_processed_" + std::to_string(fileIndex++);
        if (!std::filesystem::exists(outputProcessedFileName))
        {
            break; // No more processed files to merge
        }
        inputFilePaths.push_back(outputProcessedFileName);
    }

    mergeFiles(inputFilePaths, outputMergedFile);

    return 0;
}
