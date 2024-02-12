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


// Collection of windows: 
// https://github.com/sidneycadot/WindowFunctions/tree/master

// Windows
inline double hanning(int index, int length) {
    return  0.5 * (1.0 - cos(2.0 * M_PI * double(index) / ( double(length) - 1.0 )));
}

constexpr static const double hamming_scaling_constant = 0.08;
constexpr static const double hamming_scaling_factor = 1.0 / (1.0 - hamming_scaling_constant);
inline double hamming(int index, int length) {
    return 0.54 - (0.46 * cos(2 * M_PI * double(index) / ( double(length) - 1.)));
}

inline double blackman(int index, int length) {
    return 0.42 - (0.5 * cos(2 * M_PI * index / (length - 1))) + (0.08 * cos(4 * M_PI * index / (length - 1)));
}

inline double triangle(int index, int length) {
    const unsigned int denominator = (length % 2 != 0) ? (length + 1) : length;
    return 1.0 - fabs(2.0 * double(index) - (double(length)-1.0)) / denominator;
}

inline double bartlett(int index, int length) {
    const unsigned int denominator = (length - 1);
    return 1.0 - fabs(2.0 * double(index) - (double(length) - 1)) / denominator;
}
// Wola windows
inline double root_hann(int index, int length)
{
    return std::sqrt(hanning(index, length));//std::sqrt(0.5 + 0.5 * std::cos(2*M_PI*double(index)/double(length)));
}


/*inline double blackmanharris(int index, int length)
{
    const double coefs[3] = {0.42, -0.5, 0.08};
    const size_t ncoef = 3;
    double res = 0.0;
    for(size_t j = 0; j < ncoef; ++j)
        res += coefs[j] * cos(double(index) * double(j) * 2.0 * M_PI / double(length));
    return res;
}
*/
inline bool is_power_of_two(int n) { return (n > 0 && !(n & (n-1))); }

enum class window_t
{
    hanning, hamming, blackman
};

inline void apply_window(size_t len, window_t type, double *buf)
{
    for(size_t i = 0; i < len; ++i)
    {
        switch(type)
        {
        case window_t::hanning:
        {
            buf[i] *= hanning(i, len);
            break;   
        }
        case window_t::hamming:
        {
            buf[i] *= (hamming(i, len) - hamming_scaling_constant) * hamming_scaling_factor; 
            break;
        }
        case window_t::blackman:
        {
            buf[i] *= blackman(i, len);
            break;   
        }
        }
    }
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
            "spleen", "routine", "night", "morning", "edge", "candy", "famine", "grizzly", "fatigue"
        };

        const std::vector<std::string> adjectives = {
            "hardcode", "cute", "damaged", "majestic", "majestic", 
            "dusty", "lonely", "scared", "thirsty", "attractive", 
            "mighty", "cool", "orange", "abrasive", "satisfying", "greedy", "lame", "popular", "fuzzy",
            "rusty", "crispy", "curious",  "sweet", "fancy", "priceless", "disruptive"
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



#define DEBUG
static void print(std::string s) {
#ifdef DEBUG
    std::cout << s << std::endl;
#endif
}

#endif