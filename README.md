компиляция:

g++ -std=c++20 main.cpp -o main `pkg-config --libs --cflags fmt` `wx-config --cxxflags --libs`