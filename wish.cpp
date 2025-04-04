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

using namespace std;

// Function to read paths from paths.txt
vector<string> readPaths();
void storePaths(vector<string> &paths,const string fileName);
int isStringInVector(const vector<string> &vec, const string &str);

int main(int argc, char *argv[])
{
    // Check if no arguments are passed
    if (argc == 1)
    {
        while(true)
        {
            cout << "wish> " ;
            string command;
            int pid ;

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
                        break;
                    }
                }
                args.push_back(arg);
            }

            cout << args[0] << endl;

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
                        cout << chdir(args[1].c_str()) << endl ;
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
                    bool commandExecuted = false;

                    for (const string &path : paths)
                    {
                        if (access((path + "/" + args[0]).c_str(), X_OK) == 0)
                        {
                            int pid = fork();

                            if (pid == 0)
                            {
                                int pos = isStringInVector(args,">");
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

                                        // Prepare arguments for execv
                                        vector<char *> execArgs;
                                        for (int i = 0; i < pos; ++i)
                                        {
                                            execArgs.push_back(const_cast<char *>(args[i].c_str()));
                                        }
                                        execArgs.push_back(nullptr);

                                        execv((path + "/" + args[0]).c_str(), execArgs.data());
                                        cerr << "Failed to execute command: " << args[0] << endl;
                                        exit(1);
                                    }
                                }

                                execv((path + "/" + command).c_str(), 0);
                                exit(0);
                            }
                            else if (pid > 0)
                            {
                                wait(NULL);
                                commandExecuted = true;
                                break;
                            }
                        }
                    }

                    if (!commandExecuted)
                    {
                        cout << "Wrong Input !" << endl;
                    }
                }
                
            }
            else
            {
                cout<<"Goodbye !"<<endl;
                exit(0);
            }

            if(pid == 0)
            {
                exit(1);
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
    ifstream file("paths.txt");
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
        cout << "Unable to open file: " << "paths.txt" << endl;
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