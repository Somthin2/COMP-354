#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

// Function to read paths from paths.txt
vector<string> readPaths();
vector<string> storePaths(vector<string> &paths);

int main(int argc, char *argv[])
{
    // Check if no arguments are passed
    if (argc == 1)
    {
        while(true)
        {
            cout << "wish> " ;
            string command;



            // Reads a line from stream into our String 
            getline(cin, command);
            
            // Split the command into arguments
            stringstream ss(command);
            string arg;
            vector<string> args,paths = readPaths();

            while (ss >> arg)
            {
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
                        cout << chdir(args[1].c_str()) << endl ;
                    }
                }
                else if (args[0] == "path")
                {
                    
                }
                else
                {
                    bool commandExecuted = false;

                    for (const string &path : paths)
                    {
                        if (access((path + "/" + command).c_str(), X_OK) == 0)
                        {
                            int pid = fork();

                            if (pid == 0)
                            {
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

vector<string> storePaths(vector<string> &paths)
{
    ofstream file("paths.txt"); 

    if (file.is_open())
    {
        for (int i = 1 ; i < paths.size(); i++)
        {
            file << paths[i] << endl;
        }
        file.close();
    }
    else
    {
        cerr << "Unable to open file: paths.txt" << endl;
    }

    return paths;
}