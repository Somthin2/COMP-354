/**
 * @file wish.cpp
 * @brief A simple shell implementation with basic command execution capabilities
 *
 * This program implements a basic shell ("wish") that can:
 * - Execute commands from standard paths
 * - Handle path management via the "path" command
 * - Support changing directories with "cd"
 * - Support output redirection with ">"
 * - Handle batch script execution from files
 * - Support background processes using "&"
 *
 * @author Not specified
 * @date May 2025
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <fstream>
#include <algorithm>
#include <fcntl.h>
#include <sys/mman.h> 
#include <string>

// Constants for better readability and safety
constexpr const char* DEFAULT_PATHS_FILE = "/workspaces/COMP-354/paths.txt";
constexpr const char* PROMPT = "wish> ";
constexpr const char* EXIT_COMMAND = "exit";
constexpr const char* CD_COMMAND = "cd";
constexpr const char* PATH_COMMAND = "path";
constexpr const char* REDIRECT_OPERATOR = ">";
constexpr const char* BACKGROUND_OPERATOR = "&";
constexpr int COMMAND_NOT_FOUND = -1;
constexpr int EXIT_SUCCESS_CODE = 0;
constexpr int EXIT_FAILURE_CODE = 1;
constexpr mode_t FILE_PERMISSIONS = S_IRUSR | S_IWUSR;

using namespace std;

/**
 * @brief Reads command paths from a configuration file
 * 
 * Reads system paths from a "paths.txt" file, where each line contains one path.
 * These paths are used to locate executable commands.
 *
 * @return A vector of strings containing all the paths read from the file
 */
vector<string> readPaths();

/**
 * @brief Stores path information to a configuration file
 * 
 * Writes the current set of paths to the specified file, one path per line.
 * This maintains path persistence between shell sessions.
 *
 * @param paths The vector of paths to be stored
 * @param fileName The name of the file to write the paths to
 */
void storePaths(const vector<string> &paths, const string &fileName);

/**
 * @brief Searches for a string in a vector and returns its index
 *
 * Checks if a given string exists in a vector and returns its position.
 * Commonly used to find special characters or commands in the arguments list.
 *
 * @param vec The vector to search in
 * @param str The string to search for
 * @return The index of the string if found, -1 otherwise
 */
int isStringInVector(const vector<string> &vec, const string &str);

/**
 * @brief Executes a command with its arguments
 *
 * Takes a vector of arguments, looks for the executable in the defined paths,
 * and executes it if found. Handles output redirection and special shell commands.
 *
 * @param args Vector of strings where the first element is the command and subsequent elements are arguments
 */
void runCommandLineCommand(vector<string> &args);

/**
 * @brief Main function of the shell
 *
 * Initializes the shell, processes user commands, and handles batch script execution.
 * Creates a shared memory space for tracking the parent process.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit status of the program
 */
int main(int argc, char *argv[])
{
    // Create shared memory for the parent variable
    int *parent = static_cast<int*>(mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, 
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    if (parent == MAP_FAILED) {
        std::cerr << "mmap failed" << std::endl;
        return EXIT_FAILURE_CODE;
    }
    *parent = 0;

    while(true)
    {
        string command;
        int pid = 0;

        if (argc == 1)
        {
            // Interactive mode
            cout << PROMPT;

            // Reads a line from stream into our String 
            getline(cin, command);

            if (command.empty())
            {
                continue;
            }
        }        
        else if (argc >= 2)
        {
            // Batch mode - execute commands from a file
            ifstream inputFile(argv[1]);
            if (!inputFile.is_open())
            {
                cerr << "Error: Could not open file " << argv[1] << endl;
                return EXIT_FAILURE_CODE;
            }

            string command;
            while (getline(inputFile, command))
            {
                if (command.empty()) continue;

                // Process the command (same as interactive mode)
                stringstream ss(command);
                vector<string> args;
                string arg, temp;
                while (ss >> arg)
                {
                    temp += arg + " ";
                    args.push_back(arg);
                }

                // Remove all spaces for empty command check
                temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());               
                if (temp.empty()) continue;

                if (!args.empty() && args[0] == EXIT_COMMAND) 
                {
                    cout << "GoodBye !" << endl;
                    exit(EXIT_SUCCESS_CODE);
                }
                
                if (!args.empty()) {
                    runCommandLineCommand(args);
                }
                
                temp.clear();
                args.clear();
            }
        
            inputFile.close();
            return EXIT_SUCCESS_CODE;  // Exit after processing batch file
        }

        // Split the command into arguments
        stringstream ss(command);
        string arg;
        vector<string> args, paths = readPaths();

        while (ss >> arg)
        {
            if (arg == BACKGROUND_OPERATOR)
            {
                // Handle background process
                pid = fork();

                if (pid == 0)
                {
                    // Child process
                    args.clear();
                    continue;
                }
                else if (pid > 0)
                {
                    // Parent process
                    if (*parent == 0) *parent = pid;
                    break;
                }
                else {
                    // Fork failed
                    cerr << "Fork failed!" << endl;
                }
            }
            args.push_back(arg);
        }

        if (args.empty()) {
            continue;  // Skip empty commands
        }

        if(args[0] != EXIT_COMMAND)
        {
            if (args[0] == CD_COMMAND)
            {
                // Handle change directory command
                if (args.size() != 2)
                {
                    cout << "Invalid Command" << endl;
                    continue;
                }
                else
                {
                    if (chdir(args[1].c_str()) != 0) {
                        cerr << "cd: " << args[1] << ": No such file or directory" << endl;
                    }
                }
            }
            else if (args[0] == PATH_COMMAND)
            {
                // Handle path command
                paths.clear();

                for (size_t i = 1; i < args.size(); i++)
                {
                    // Store new paths in the paths vector
                    paths.push_back(args[i]);
                }

                // Save the updated paths to the file
                storePaths(paths, "paths.txt");

                cout << "Paths have been updated" << endl;
            }
            else
            {
                // Execute external command
                runCommandLineCommand(args);
                cout << pid << " ? " << *parent << endl;
                if (*parent != 0 && pid != *parent) exit(EXIT_SUCCESS_CODE); // Exit child process

                // Wait for all child processes to finish
                int status;
                while (waitpid(-1, &status, 0) > 0)
                {
                    // Process child termination status
                    cout << "Child process finished with status: " << status << endl;
                }

                // Reset parent to 0 after all child processes are done
                *parent = 0;
            }
        }
        else
        {
            // Exit shell
            cout << "Goodbye !" << endl;
            // Clean up shared memory
            munmap(parent, sizeof(int));
            exit(EXIT_SUCCESS_CODE);
        }
    }
    
    // Clean up shared memory before returning
    munmap(parent, sizeof(int));
    return EXIT_SUCCESS_CODE;
}

/**
 * @brief Reads paths from the paths.txt configuration file
 *
 * Opens and reads the paths.txt file, loading each line as a separate path
 * into a vector of strings. Each path represents a directory to search for
 * executable commands.
 *
 * @return Vector of paths read from the configuration file
 */
vector<string> readPaths()
{
    vector<string> paths;
    ifstream file(DEFAULT_PATHS_FILE);
    string line;

    if (file.is_open())
    {
        while (getline(file, line))
        {
            if (!line.empty()) {
                paths.push_back(line);
            }
        }
        file.close();
    }
    else
    {
        // For debugging
        cerr << "Unable to open file: " << DEFAULT_PATHS_FILE << endl;
    }

    return paths;
}

/**
 * @brief Stores the current paths to the specified file
 *
 * Writes each path in the provided vector to a file, one path per line.
 * This allows path settings to persist between shell sessions.
 *
 * @param paths Vector of paths to store
 * @param fileName Name of the file to write to
 */
void storePaths(const vector<string> &paths, const string &fileName)
{
    ofstream file(fileName); 

    if (file.is_open())
    {
        for (size_t i = 0; i < paths.size(); i++)
        {
            file << paths[i] << endl;
        }
        file.close();
    }
    else
    {
        // For debugging
        cerr << "Unable to open file for writing: " << fileName << endl;
    }
}

/**
 * @brief Finds a string in a vector and returns its position
 *
 * Searches for a string in a vector and returns its index if found.
 * Used to locate special operators or arguments in the command line.
 *
 * @param vec Vector to search in
 * @param str String to find
 * @return Index of the string if found, -1 otherwise
 */
int isStringInVector(const vector<string> &vec, const string &str)
{
    auto it = find(vec.begin(), vec.end(), str);
    if (it != vec.end())
    {
        return static_cast<int>(distance(vec.begin(), it));
    }
    else
    {
        return COMMAND_NOT_FOUND; 
    }
}

/**
 * @brief Executes a command with arguments
 *
 * Takes a command and its arguments, searches for the executable in the paths,
 * and executes it if found. Handles output redirection with ">" operator.
 * For built-in commands like "cd" and "path", the function delegates to the main loop.
 *
 * @param args Vector containing the command and its arguments
 */
void runCommandLineCommand(vector<string> &args)
{
    bool commandExecuted = false;
    
    if (args.empty()) {
        cerr << "No command provided!" << endl;
        return;
    }
    
    const vector<string> paths = readPaths();
    
    // Search for the executable in all paths
    for (const string &path : paths)
    {
        if (commandExecuted) break;

        const string executablePath = path + "/" + args[0];
        if (access(executablePath.c_str(), X_OK) == 0)
        {
            int pid = fork();

            if (pid == 0) // Child process
            {
                // Check for redirection operator
                const int redirectPos = isStringInVector(args, REDIRECT_OPERATOR);

                // Convert string arguments to char* for execvp
                vector<char*> c_args;
                for (const auto& arg : args) {
                    c_args.push_back(const_cast<char*>(arg.c_str()));
                }
                c_args.push_back(nullptr); // Null-terminate the array                
                
                if (redirectPos != COMMAND_NOT_FOUND)
                {
                    // Handle output redirection
                    // Currently assuming only one argument after ">"
                    if (args.size() - redirectPos != 2)
                    {
                        cerr << "Invalid redirection syntax" << endl;
                        exit(EXIT_FAILURE_CODE);
                    }
                    else
                    {
                        // Open the file for writing
                        const int fd = open(args[redirectPos + 1].c_str(), 
                                      O_WRONLY | O_CREAT | O_TRUNC, 
                                      FILE_PERMISSIONS);
                        if (fd < 0)
                        {
                            cerr << "Failed to open file: " << args[redirectPos + 1] << endl;
                            exit(EXIT_FAILURE_CODE);
                        }

                        // Redirect stdout to the file
                        if (dup2(fd, STDOUT_FILENO) < 0) {
                            cerr << "Failed to redirect output" << endl;
                            close(fd);
                            exit(EXIT_FAILURE_CODE);
                        }
                        close(fd);

                        // Remove redirection operator and filename from args
                        c_args[redirectPos] = nullptr;
                        
                        if (execvp(executablePath.c_str(), c_args.data()) == -1) 
                        {
                            perror("execvp failed");
                            exit(EXIT_FAILURE_CODE);
                        }                            
                    }
                    exit(EXIT_SUCCESS_CODE);
                }
                
                // Execute the command
                if (execvp(executablePath.c_str(), c_args.data()) == -1) 
                {
                    perror("execvp failed");
                    exit(EXIT_FAILURE_CODE);
                }                    
                exit(EXIT_SUCCESS_CODE);
            }
            else if (pid > 0) // Parent process
            {
                // Parent process waits for child
                int status;
                waitpid(pid, &status, 0);
                commandExecuted = true;
            }
            else // Fork failed
            {
                cerr << "Fork failed" << endl;
                return;
            }
        }
    }

    if (!commandExecuted)
    {
        if (!args.empty() && args[0] != EXIT_COMMAND)
        {
            if (args[0] == CD_COMMAND)
            {
                // Handle built-in cd command
                if (args.size() != 2)
                {
                    cout << "Invalid Command" << endl;
                }
                else
                {
                    if (chdir(args[1].c_str()) != 0) {
                        cerr << "cd: " << args[1] << ": No such file or directory" << endl;
                    }
                }
            }
            else if (args[0] == PATH_COMMAND)
            {
                // Handle built-in path command
                vector<string> newPaths;

                for (size_t i = 1; i < args.size(); i++)
                {
                    // Store new paths
                    newPaths.push_back(args[i]);
                }

                // Save paths to file
                storePaths(newPaths, "paths.txt");

                cout << "Paths have been updated" << endl;
            }
            else
            {
                // Command not found
                cerr << "Command not found: " << args[0] << endl;
            }
        }
    }
}