#ifndef UTILITIES_H
#define UTILITIES_H

#include<cmath>
#include<iostream>

#include<string>
#include<fstream>
#include<streambuf>

#include<memory>

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

struct name_gen
{
    static std::string gen()
    {
        const std::vector<std::string> names = {
            "flower", "sky", "gear", "shaker", "kitty", "communist", "frog", "sword", "fog", 
            "chocolate", "queen", "intention", "tooth", "knowledge", "virus", "failure", "cookie", "anxiety", 
            "attitude", "feedback", "signifiance", "monk", "pizza", "mojo", "splash", "pineapple", 
            "spleen", "routine", "night", "morning", "edge", "candy", "famine"
        };

        const std::vector<std::string> adjectives = {
            "hardcode", "cute", "damaged", "majestic", "majestic", 
            "dusty", "lonely", "scared", "thirsty", "attractive", 
            "mighty", "cool", "orange", "abrasive", "satisfying", "greedy", "lame", "popular", "fuzzy",
            "rusty", "crispy", "curious",  "sweet", "fancy"
        };


        int iadj = rand() % (adjectives.size());
        int inam = rand() % (names.size());
        std::string w1  = adjectives[iadj];
        std::string w2 = names[inam];

        return std::string(w1 + " " + w2);

    }

    static std::string concat(std::string base)
    {
        return std::string(base + " " + gen());
    }    
};

template<typename Flt>
Flt* contiguous_memory(size_t bloc_size, size_t n_channels, Flt **d_ptr)
{
    Flt *mem = new Flt[n_channels * bloc_size];
    d_ptr = new Flt*[n_channels];
    for(size_t i = 0; i < n_channels; ++i)
        d_ptr[i] = mem + (i * bloc_size);
    return mem;
}

#endif