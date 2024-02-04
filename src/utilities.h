#ifndef UTILITIES_H
#define UTILITIES_H

#include<cmath>
#include<iostream>

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif


#include<iostream>

// Windows
inline double hanning(int index, int length) {
    return  0.5 * (1 - cos(2 * M_PI * index / (length - 1 )));
}


#endif