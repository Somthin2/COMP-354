#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>

using namespace std;

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
            vector<string> args;
            while (ss >> arg)
            {
                args.push_back(arg);
            }

            if(args[0] != "exit")
            {
                //If the first check comes true it wont need to do the second, but if its -1 then it will go check the second one too
                if (access(("/bin/" + args[0]).c_str(), X_OK) == 0 || access(("/usr/bin/" + args[0]).c_str(), X_OK)  == 0)
                {
                    int pid = fork();

                    if (pid == 0)
                    {
                        execv(("/bin/" +  args[0]).c_str(), 0);
                    }
                    else
                    { 
                        wait(NULL);
                        continue ;
                    }
                    
                }
                else if (args[0] == "cd")
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
                else
                {
                    cout<<"Wrong Input !"<<endl;
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