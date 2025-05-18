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
void storePaths(vector<string> &paths, const string fileName);

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
    int *parent = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *parent = 0;

    while(true)
    {
        string command;
        int pid;

        if (argc == 1)
        {
            // Interactive mode
            cout << "wish> " ;

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
                return 1;
            }

            string command;
            while (getline(inputFile, command))
            {
                if (command.empty()) continue;

                // Process the command (same as interactive mode)
                stringstream ss(command);
                vector<string> args;
                string arg,temp="";
                while (ss >> arg)
                {
                    temp += arg + " ";
                    args.push_back(arg);
                }

                // Remove all spaces for empty command check
                temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());               
                if (temp == "") continue;

                if (args[0] == "exit") 
                {
                    cout<<"GoodBye !"<<endl;
                    exit(0);
                }
                runCommandLineCommand(args);
                temp="";
                args.clear();
            }
        
            inputFile.close();
        }

        // Split the command into arguments
        stringstream ss(command);
        string arg;
        vector<string> args,paths = readPaths();

        while (ss >> arg)
        {
            if (arg == "&")
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
                    if (*parent == 0) *parent = pid ;

                    break;
                }
            }
            args.push_back(arg);
        }

        if(args[0] != "exit")
        {
            if (args[0] == "cd")
            {
                // Handle change directory command
                if (args.size() != 2)
                {
                    cout<<"Invalid Command"<<endl;
                    continue;
                }
                else
                {
                    chdir(args[1].c_str());
                }
            }
            else if (args[0] == "path")
            {
                // Handle path command
                paths.clear();

                for (int i = 1 ; i < args.size(); i++)
                {
                    // Store new paths in the paths vector
                    paths.push_back(args[i]);
                }

                // Save the updated paths to the file
                storePaths(paths, "paths.txt");

                cout<<"Paths have been updated"<<endl;
            }
            else
            {
                // Execute external command
                runCommandLineCommand(args);
                cout<<pid<<" ? " << *parent <<endl;
                if (*parent != 0 && pid != *parent) exit(0); // Exit child process

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
            cout<<"Goodbye !"<<endl;
            exit(0);
        }
    }
    return 0;
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
    ifstream file("/workspaces/COMP-354/paths.txt");
    string line;

    if (file.is_open())
    {
        while (getline(file, line))
        {
            paths.push_back(line);
        }
        file.close();
    }
    else
    {
        // For debugging
        cout << "Unable to open file: " << "/workspaces/COMP-354/paths.txt" << endl;
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
void storePaths(vector<string> &paths,const string fileName)
{
    ofstream file(fileName); 

    if (file.is_open())
    {
        for (int i = 0 ; i < paths.size(); i++)
        {
            file << paths[i] << endl;
        }
        file.close();
    }
    else
    {
        // For debugging
        cerr << "Unable to open file: paths.txt" << endl;
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
        return distance(vec.begin(), it);
    }
    else
    {
        return -1; 
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
    vector<string> paths = readPaths();

    if (args.empty()) {
        cerr << "No command provided!" << endl;
        return;
    }
    // Convert vector<string> to char* array for execvp

        for (const string &path : paths)
        {
            if (commandExecuted == true) break;

            else if (access((path + "/" + args[0]).c_str(), X_OK) == 0)
            {
                int pid = fork();

                if (pid == 0)
                {
                    // Check for redirection operator
                    int pos = isStringInVector(args,">");

                    // Convert string arguments to char* for execvp
                    vector<char*> c_args;
                    for (const auto& arg : args) {
                        c_args.push_back(const_cast<char*>(arg.c_str()));
                    }
                    c_args.push_back(nullptr); // Null-terminate the array                
                    
                    if (pos != -1)
                    {
                        // Handle output redirection
                        // Currently assuming only one argument after ">"
                        if (args.size() - pos != 2)
                        {
                            cout<<"Invalid arguments"<<endl;
                            exit(1);
                        }
                        else
                        {
                            // Open the file for writing
                            int fd = open(args[pos + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                            if (fd < 0)
                            {
                                cerr << "Failed to open file: " << args[pos + 1] << endl;
                                exit(1);
                            }

                            // Redirect stdout to the file
                            dup2(fd, STDOUT_FILENO);
                            close(fd);

                            if (execvp(c_args[0], c_args.data()) == -1) 
                            {
                                perror("execvp failed");
                                exit(EXIT_FAILURE); // Exit if execvp fails
                            }                            
                        }
                        exit(0);
                    }
                    
                    // Execute the command
                    if (execvp(c_args[0], c_args.data()) == -1) 
                    {
                        perror("execvp failed");
                        exit(EXIT_FAILURE); // Exit if execvp fails
                    }                    
                    exit(0);
                }
                else if (pid > 0)
                {
                    // Parent process waits for child
                    wait(NULL);
                    commandExecuted = true;
                    continue;
                }
            }
        }

        if (!commandExecuted)
        {
            if(args[0] != "exit")
            {
                if (args[0] == "cd")
                {
                    // Handle built-in cd command
                    if (args.size() != 2)
                    {
                        cout<<"Invalid Command"<<endl;
                
                    }
                    else
                    {
                        chdir(args[1].c_str());
                    }
                }
                else if (args[0] == "path")
                {
                    // Handle built-in path command
                    paths.clear();

                    for (int i = 1 ; i < args.size(); i++)
                    {
                        // Store new paths
                        paths.push_back(args[i]);
                    }

                    // Save paths to file
                    storePaths(paths, "paths.txt");

                    cout<<"Paths have been updated"<<endl;
                }
                else
                {
                    // Command not found
                    cout << "Wrong Input !" << endl;
                }
            }
        }
}