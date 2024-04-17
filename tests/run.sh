#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

for filename in $(find . -name '*.nox' ); do
    file_count=$((file_count + 1))
    ./nothing $filename > /dev/null 2>&1
    if [ $? -eq 0 ]
    then
        echo -e "[${GREEN}SUCCESS${NC}]:\t$filename"
        success_count=$((success_count + 1))
    else
        echo -e "[${RED}FAILURE${NC}]:\t$filename"
        error_count=$((error_count + 1))
    fi
done;

if (($success_count < $file_count)) 
then
    echo -e "${RED}$success_count/$file_count tests succeded${NC}"
else    
    echo -e "${GREEN}$success_count/$file_count tests succeded${NC}"
fi
