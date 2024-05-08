#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>

const int CHUNK_SIZE = 1024 * 1024; // 1MB

std::mutex queueMutex;
std::queue<std::vector<char>> taskQueue;

void readFile(const std::string &inputFilePath)
{
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open input file." << std::endl;
        return;
    }

    while (true)
    {
        std::vector<char> buffer(CHUNK_SIZE);
        inputFile.read(buffer.data(), CHUNK_SIZE);
        std::streamsize bytesRead = inputFile.gcount();

        if (bytesRead > 0)
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push(std::vector<char>(buffer.begin(), buffer.begin() + bytesRead));
        }

        if (bytesRead < CHUNK_SIZE)
        {
            break; // Reached end of file
        }
    }

    inputFile.close();
}

void processTask()
{
    while (true)
    {
        std::vector<char> task;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (taskQueue.empty())
            {
                break; // No more tasks to process
            }
            task = taskQueue.front();
            taskQueue.pop();
        }

        // Process the task here
        // Example: Perform some operations on the task data
        for (char &c : task)
        {
            // Do something with each character
            // Example: Convert character to uppercase
            c = std::toupper(static_cast<unsigned char>(c));
        }

        // Do something with the processed task here
    }
}

int main()
{
    std::string inputFilePath = "input.txt";
    int numThreads = 4;

    std::thread readerThread(readFile, inputFilePath);

    std::vector<std::thread> workerThreads;
    for (int i = 0; i < numThreads; ++i)
    {
        workerThreads.emplace_back(processTask);
    }

    readerThread.join();
    for (std::thread &thread : workerThreads)
    {
        thread.join();
    }

    // All tasks have been processed

    return 0;
}
