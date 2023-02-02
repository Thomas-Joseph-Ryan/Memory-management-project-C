#!/bin/bash

#Compile
gcc -fprofile-arcs -ftest-coverage -o ymirdb ymirdb.c
#Reading the file names of the tests and writing them to a .txt
for FILE in ./tests/*.in; 
do 
echo $FILE|
cut -f 3 -d '/'|
cut -f 1 -d '.' >> testnames.txt;
done 

#Loop through testnames and use these to run the diff cmd.
cat testnames.txt | while read testname 
do

    #Use contents of .in file to test
    ./ymirdb < ./tests/$testname.in > out.txt

    #Running diff tests
    diff -u ./tests/$testname.out ./out.txt
    es=$?
    if [ $es -ne 0 ]; 
    then
        echo ^ "$testname test" ^
        echo ------------
    else
        echo "$testname test success!"
    fi

    #Cleaning up output for next test.
    rm ./out.txt

done

gcov ./ymirdb.c

##CLEAN UP
rm testnames.txt
rm ymirdb.gcda
rm ymirdb.gcno