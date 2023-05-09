#!/bin/sh
# Accepts the following runtime arguments: the first argument is a path to a directory on the filesystem,
# referred to below as filesdir; the second argument is a text string which will be searched within these files, referred to below as searchstr
# Exits with return value 1 error and print statements if any of the parameters above were not specified
# Exits with return value 1 error and print statements if filesdir does not represent a directory on the filesystem
# Prints a message "The number of files are X and the number of matching lines are Y" where X is the number of files
#in the directory and all subdirectories and Y is the number of matching lines found in respective files,
#where a matching line refers to a line which contains searchstr (and may also contain additional content).

filesdir=$1
searchstr=$2

usage="program to find a certain text in files inside a folder
Usage:
./$(basename "$0") <search_directory> <search_text> 
example:
    ./$(basename "$0") /tmp/aesd-data/ AELD_IS_FUN"

if [ ! $# -eq 2 ]; then
    echo "$usage"
    exit 1
fi

if [ ! -d $filesdir ]; then
    echo "directory doesn't exit"
    echo "$usage"
    exit 1
fi

numFiles=$(cd $filesdir && grep -l $searchstr * | wc -l)
numLines=$(cd $filesdir && grep -o $searchstr * | wc -l)
echo "The number of files are ${numFiles} and the number of matching lines are ${numLines}"
