#! /bin/bash

if ! [[ -x wish ]]; then
    echo "wish executable does not exist"
    exit 1
fi

# Run the test script with all passed arguments
./wish < /workspaces/COMP-354/tests/20.in