#ifndef UTILITIES_H
#define UTILITIES_H

#include<cmath>
#include<iostream>

#include<string>
#include<fstream>
#include<streambuf>

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif


#include<iostream>

// Windows
inline double hanning(int index, int length) {
    return  0.5 * (1 - cos(2 * M_PI * index / (length - 1 )));
}

inline std::string read_file(std::string path)
{
    std::ifstream ifs;
    ifs.open(path);
    std::string csd_str; 
    ifs.seekg(0, std::ios::end);
    csd_str.reserve(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    csd_str.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return csd_str;
}

#endif