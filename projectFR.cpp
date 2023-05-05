#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>
#include <thread>

using namespace std;
using namespace std::chrono;

void IndexDirectory(string directoryPath, ofstream& output, bool includeHidden)
{
    WIN32_FIND_DATA fileData;
    HANDLE hFind;

    string searchPath = directoryPath + "\\*.*";

    hFind = FindFirstFile(searchPath.c_str(), &fileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "Error: Unable to index directory " << directoryPath << endl;
        return;
    }

    do
    {
        if (strcmp(fileData.cFileName, ".") == 0 || strcmp(fileData.cFileName, "..") == 0)
        {
            continue;
        }

        string filePath = directoryPath + "\\" + fileData.cFileName;

        if (!includeHidden && (fileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
            continue;
        }

        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            IndexDirectory(filePath, output, includeHidden);
        }
        else
        {
            output << filePath << endl;
        }
    } while (FindNextFile(hFind, &fileData));

    FindClose(hFind);
}

void TraverseDirectory(string directoryPath, ofstream& output, bool recursive, bool includeHidden, int numThreads)
{
    vector<thread> threads;

    WIN32_FIND_DATA fileData;
    HANDLE hFind;

    string searchPath = directoryPath + "\\*.*";

    hFind = FindFirstFile(searchPath.c_str(), &fileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "Error: Unable to traverse directory " << directoryPath << endl;
        return;
    }

    do
    {
        if (strcmp(fileData.cFileName, ".") == 0 || strcmp(fileData.cFileName, "..") == 0)
        {
            continue;
        }

        string filePath = directoryPath + "\\" + fileData.cFileName;

        if (!includeHidden && (fileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
            continue;
        }

        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (recursive)
            {
                threads.push_back(thread(TraverseDirectory, filePath, ref(output), recursive, includeHidden, numThreads));
            }
        }
        else
        {
            if (threads.size() < numThreads)
            {
                threads.push_back(thread(IndexDirectory, filePath, ref(output), includeHidden));
            }
            else
            {
                for (auto& t : threads)
                {
                    t.join();
                }

                threads.clear();

                threads.push_back(thread(IndexDirectory, filePath, ref(output), includeHidden));
            }
        }
    } while (FindNextFile(hFind, &fileData));

    FindClose(hFind);

    for (auto& t : threads)
    {
        t.join();
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: Indexer <directory path> [<output path>] [-r] [-h] [-t <num threads>]" << endl;
        return 1;
    }

    string directoryPath = argv[1];
    string outputPath;
    bool recursive = false;
    bool includeHidden = false;
    int numThreads = 1;

    for (int i = 2; i < argc; i++)
    {
        string arg = argv[i];

        if (arg == "-r")
        {
            recursive = true;
        }
       
    else if (arg == "-h")
    {
        includeHidden = true;
    }
    else if (arg == "-t")
    {
        if (i + 1 >= argc)
        {
            cout << "Error: Expected number of threads after -t" << endl;
            return 1;
        }

        numThreads = atoi(argv[i + 1]);

        if (numThreads < 1)
        {
            cout << "Error: Number of threads must be at least 1" << endl;
            return 1;
        }

        i++;
    }
    else
    {
        outputPath = arg;
    }
}

if (outputPath.empty())
{
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y%m%d_%H%M%S_index.txt", localTime);
    outputPath = buffer;
}

ofstream output(outputPath);

if (!output.is_open())
{
    cout << "Error: Unable to open output file " << outputPath << endl;
    return 1;
}

auto start = high_resolution_clock::now();

if (recursive)
{
    TraverseDirectory(directoryPath, output, recursive, includeHidden, numThreads);
}
else
{
    IndexDirectory(directoryPath, output, includeHidden);
}

auto end = high_resolution_clock::now();

output.close();

auto duration = duration_cast<milliseconds>(end - start);

cout << "Indexing completed in " << duration.count() << " ms." << endl;
cout << "Output file: " << outputPath << endl;

return 0;
}

/*
In this code, we first include the necessary headers and declare the functions we will use. The TraverseDirectory function recursively traverses a directory and its subdirectories using the Windows API FindFirstFile and FindNextFile functions. For each file found, it checks if it is a directory or a file, and if it is a file, it writes the file path to the output file.

The main function sets the input and output paths and opens the output file for writing. It then calls the TraverseDirectory function to index the specified directory and writes the output to the output file. Finally, it closes the output file and prints a message indicating that the indexing is complete.

Note that this is a simple example and there are many ways to improve this indexing system. For example, you could include additional file information in the output file or use multithreading to improve performance.
we first include the necessary headers and declare the functions we will use. The TraverseDirectory function is the same as before, but we have added error checking to handle the case where the directory cannot be traversed.

In the main function, we check if the user has provided the required command-line arguments (the directory path). If not, we print a usage message and exit. If the user has provided an output path, we use that, otherwise we create an output file with the name "Index.txt" in the directory being indexed.

We then open the output file and perform error checking to ensure that it was opened successfully. We then call the TraverseDirectory function to index the specified directory and write the output to the output file. Finally, we close the output file and print a message indicating that the indexing is complete.

This longer code includes additional error checking and command-line argument handling to make the indexing system more robust and user-friendly.

This updated code includes the following additional features:

Recursive indexing: The user can now specify the -r flag on the command line to enable recursive indexing, which will index all files in all subdirectories of the specified directory.
Hidden file exclusion: The user can now specify the -h flag on the command line to exclude hidden files from the indexing process.
Timing information: The code now uses the C++ chrono library to measure the time taken to index the directory, and prints this information to the console.
The TraverseDirectory function has been updated to include the new recursive and includeHidden parameters, which control whether the function traverses subdirectories and includes hidden files, respectively. The main function has been updated to parse these

*/