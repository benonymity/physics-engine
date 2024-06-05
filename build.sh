wget https://raw.githubusercontent.com/benonymity/CMP-201/main/homework/Assignment%205/tigr.c -q -O tigr.c
wget https://raw.githubusercontent.com/benonymity/CMP-201/main/homework/Assignment%205/tigr.h -q -O tigr.h
if [[ "$OSTYPE" == "darwin"* ]]; then
    g++ -std=c++17 main.cpp tigr.c -o physics -framework OpenGL -framework Cocoa
    ./physics
fi