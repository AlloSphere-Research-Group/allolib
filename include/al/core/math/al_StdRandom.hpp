#ifndef INCLUDE_AL_STD_RANDOM_HPP
#define INCLUDE_AL_STD_RANDOM_HPP

/*
    Keehong Youn, 2017, younkeehong@gmail.com
*/

#include <random>

unsigned int al_random_seed();

namespace al {

// give: interval [a, b]
// take: an int in the interval
// default range [0:100]
class rand_uniformi {
public:
    std::mt19937 engine;
    std::uniform_int_distribution<int> distribution;

    rand_uniformi();
    rand_uniformi(unsigned int const seed);
    void seed(unsigned int const seed);
    int operator()();
    int operator()(int const b);
    int operator()(int const a, int const b);
};

// give: interval [a, b)
// take: a float in the interval
// default range [0:1)
class rand_uniformf {
public:
    std::mt19937 engine;
    std::uniform_real_distribution<float> distribution;

    rand_uniformf();
    rand_uniformf(unsigned int const seed);
    void seed(unsigned int const seed);
    float operator()();
    float operator()(float const b);
    float operator()(float const a, float const b);
};

// give: mean of number of times an event happening in certain interval
// take: number of times that event happened in same interval
// default mean 100
class rand_poisson {
public:
    std::mt19937 engine;
    std::poisson_distribution<int> distribution;

    rand_poisson();
    rand_poisson(unsigned int const seed);
    void seed(unsigned int const seed);
    int operator()();
    int operator()(float const m);
};

// give: rate of an event happening in unit interval
// take: time/distance till next event happens
// default rate 1
class rand_exponential {
public:
    std::mt19937 engine;
    std::exponential_distribution<float> distribution;

    rand_exponential();
    rand_exponential(unsigned int const seed);
    void seed(unsigned int const seed);
    float operator()();
    float operator()(float const r);
};

// give: mean and standard deviation
// take: a sample from normal distribution
// default mean=0, stdev=1
class rand_normal {
public:
    std::mt19937 engine;
    std::normal_distribution<float> distribution;
   
    rand_normal();
    rand_normal(unsigned int const seed);
    void seed(unsigned int const seed);
    float operator()();
    float operator()(float const mean, float const stdev);
};

namespace rnd {

/// Get global random number generator
rand_uniformf& global();
rand_normal& global_normal();
rand_uniformi& global_ui();

/// Returns uniform random in [0, 1)
inline float uniform(){ return global()(); }

/// Returns uniform random in [0, hi)
template <class T>
inline T uniform(const T& hi){ return global()(0.0f, float(hi)); }

/// Returns uniform random in [lo, hi)
template <class T>
inline T uniform(const T& hi, const T& lo){
    return global()(float(lo), float(hi));
}

/// Returns signed uniform random in (-1, 1)
inline float uniformS(){ return global()(-1.0f, 1.0f); }

/// Returns signed uniform random in (-lim, lim)
template <class T>
inline T uniformS(const T& lim){
    return global()(float(-lim), float(lim));
}

/// Returns point within a unit ball

/// To get a random point on a sphere, simply normalize the result.
/// \tparam    N    dimensions of ball
/// @param[in]  point  an array of size N
template <int N, class T>
inline void ball(T * point){
    T w;
    do{
        w = T(0);
        for(int i=0; i<N; ++i){
            float v = uniformS();
            point[i] = v;
            w += v*v;
        }
    } while(w >= T(1)); // if on or outside unit ball, try again
}

/// Returns point within a unit ball
template <template<int,class> class VecType, int N, class T>
inline void ball(VecType<N,T>& point){ ball<N>(&point[0]); }

/// Returns point within a unit ball
template <class VecType>
inline VecType ball(){ VecType v; ball(v); return v; }

/// Returns standard normal variate
inline float normal(){ return global_normal()(); }
inline float gaussian(){ return normal(); }

/// Returns triangle distribution variate, in (-1,1)
inline float triangle(){
    return 0.5f * (uniformS() + uniformS());
}

/// Returns true with probability 0.5
inline bool prob(){ return (uniformS() < 0); }

/// Returns true with probability p
inline bool prob(float p){ return (uniform() < p); }

/// Returns argument with sign randomly flipped
inline float sign(float x=1.f){
    static float arr[2] = {-1.0f, 1.0f};
    return x * arr[global_ui()(0, 1)];
}


} // rnd::

} // al::

#endif