#!/bin/bash
# Accepts the following arguments: the first argument is a full path to a file (including filename)
# on the filesystem, referred to below as writefile;
# the second argument is a text string which will be written within this file,
# referred to below as writestr
# Exits with value 1 error and print statements if any of the arguments above were not specified
# Creates a new file with name and path writefile with content writestr,
# overwriting any existing file and creating the path if it doesn’t exist.
# Exits with value 1 and error print statement if the file could not be created.

red=$(tput setaf 1)
green=$(tput setaf 2)
reset=$(tput sgr0)

writefile=$1
writestr=$2

usage="program to write text to certain file
${green}Usage${reset}:
./$(basename "$0") <write_file> <write_text> 
example:
    ./$(basename "$0") /tmp/aesd/assignment1/sample.txt ios"

if [ ! $# -eq 2 ]; then
    echo "$usage"
    exit 1
fi

if [ ! -e $writefile ]; then
    echo "${green}creating file ${writefile}${reset}"
    mkdir -p "${writefile%/*}" && touch "$writefile"
else
    echo "file exit"
fi

echo $writestr >$writefile
