#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    // Check if no arguments are passed
    if (argc == 1)
    {
        cout << "Just started normally" << endl;
    }
    // Check if one or more arguments are passed
    else if (argc >= 2)
    {
        cout << "Just started by passing variables" << endl;
    }
    return 0;
}