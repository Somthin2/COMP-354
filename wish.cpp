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

using namespace std;

// Function to read paths from paths.txt
vector<string> readPaths();
void storePaths(vector<string> &paths,const string fileName);
int isStringInVector(const vector<string> &vec, const string &str);
void runCommandLineCommand(vector<string> &args);


int main(int argc, char *argv[])
{
    // Create shared memory for the parent variable
    int *parent = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *parent = 0;

    // Check if no arguments are passed
    if (argc == 1)
    {
        while(true)
        {
            cout << "wish> " ;
            string command;
            int pid;

            // Reads a line from stream into our String 
            getline(cin, command);
            
            // Split the command into arguments
            stringstream ss(command);
            string arg;
            vector<string> args,paths = readPaths();

            while (ss >> arg)
            {
                if (arg == "&")
                {
                    pid = fork();

                    if (pid == 0)
                    {
                        args.clear();
                        continue;
                    }
                    else if (pid > 0)
                    {
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
                    paths.clear();

                    for (int i = 1 ; i < args.size(); i++)
                    {
                        // This will store them for now in the paths vector but this is only temporary
                        paths.push_back(args[i]);
                    }

                    // maybe can split the task here ask so you know

                    storePaths(paths, "paths.txt");

                    cout<<"Paths have been updated"<<endl;
                }
                else
                {
                    // Command line commands ?
                    runCommandLineCommand(args);
                    cout<<pid<<" ? " << *parent <<endl;
                    if (*parent != 0 && pid != *parent) exit(0); //Time to go xD

                    // Wait for all child processes to finish
                    int status;
                    while (waitpid(-1, &status, 0) > 0)
                    {
                        // Optionally, you can handle the status here if needed
                        cout << "Child process finished with status: " << status << endl;
                    }

                    // Reset parent to 0 after all child processes are done
                    *parent = 0;
                }
                
                
            }
            else
            {
                cout<<"Goodbye !"<<endl;
                exit(0);
            }
        }
        
    }
    // Check if one or more arguments are passed
    else if (argc >= 2)
    {
        cout << "Just started by passing variables" << endl;
    }
    return 0;
}

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
                    int pos = isStringInVector(args,">");

                    vector<char*> c_args;
                    for (const auto& arg : args) {
                        c_args.push_back(const_cast<char*>(arg.c_str()));
                    }
                    c_args.push_back(nullptr); // Null-terminate the array                
                    
                    if (pos != -1)
                    {
                        // Currenlty im assuming after > only one argument can be placed
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
                    
                    
                    if (execvp(c_args[0], c_args.data()) == -1) 
                    {
                        perror("execvp failed");
                        exit(EXIT_FAILURE); // Exit if execvp fails
                    }                    
                    exit(0);
                }
                else if (pid > 0)
                {
                    wait(NULL);
                    commandExecuted = true;
                    continue;
                }
            }
        }

        if (!commandExecuted)
        {
            cout << "Wrong Input !" << endl;
        }
    

}