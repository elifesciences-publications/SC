#!/bin/bash

cd ../../scOOP/

rm -f CMakeCache.txt
rm -rf CMakeFiles/

echo "Building..."

cmake . > /dev/null

make -j4 > /dev/null
mv SC ../Tests/

cd ../Tests/

mv SC Long_tests/
cp ./Long_tests/SC normal/new
cp ./Long_tests/SC normal/new_switch
cp ./Long_tests/SC normal/Grand_single

echo ""
echo "Building comparison version..."

gcc sc35.c -lm -o SC

cp SC Long_tests/normal/old/
mv SC Long_tests/normal/old_switch/


cd Long_tests/normal/

for str in `ls`; do 
    cd $str
    rm -rf test/
    mkdir test
    cp * test/
    cd test
    psubmit long SC -y
    cd ../..
done
