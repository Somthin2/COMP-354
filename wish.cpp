#include <iostream>
#include <unistd.h>

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

            if(command != "exit")
            {
                //If the first check comes true it wont need to do the second, but if its -1 then it will go check the second one too
                if (access(("/bin/" + command).c_str(), X_OK) == 0 || access(("/usr/bin/" + command).c_str(), X_OK)  == 0)
                {
                    cout<<"Command Exists !"<<endl;
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