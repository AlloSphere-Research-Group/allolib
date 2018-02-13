#include "al/core/math/al_StdRandom.hpp"

unsigned int al_random_seed() {
    static std::random_device device;
    return device();
}

using namespace al;

rand_uniformi::rand_uniformi():
    engine(al_random_seed()), distribution(0, 100) {}
rand_uniformi::rand_uniformi(unsigned int const seed):
    engine(seed), distribution(0, 100) {}
void rand_uniformi::seed(unsigned int const seed) { engine.seed(seed); }
int rand_uniformi::operator()() { return distribution(engine); }
int rand_uniformi::operator()(int const b) {
    return (*this)(0, b);
}
int rand_uniformi::operator()(int const a, int const b) {
    return distribution(
        engine,
        std::uniform_int_distribution<int>::param_type(a, b)
    );
}

rand_uniformf::rand_uniformf() :
    engine(al_random_seed()), distribution(0, 1) {}
rand_uniformf::rand_uniformf(unsigned int const seed) :
    engine(seed), distribution(0, 1) {}
void rand_uniformf::seed(unsigned int const seed) { engine.seed(seed); }
float rand_uniformf::operator()() { return distribution(engine); }
float rand_uniformf::operator()(float const b) {
    return (*this)(0, b);
}
float rand_uniformf::operator()(float const a, float const b) {
    return distribution(
        engine,
        std::uniform_real_distribution<float>::param_type(a, b)
    );
}

rand_poisson::rand_poisson():
    engine(al_random_seed()), distribution(100) {}
rand_poisson::rand_poisson(unsigned int const seed):
    engine(seed), distribution(100) {}
void rand_poisson::seed(unsigned int const seed) { engine.seed(seed); }
int rand_poisson::operator()() {
    return distribution(engine);
}
int rand_poisson::operator()(float const m) {
    return distribution(
        engine,
        std::poisson_distribution<int>::param_type(m)
    );
}

rand_exponential::rand_exponential() :
    engine(al_random_seed()), distribution(1) {}
rand_exponential::rand_exponential(unsigned int const seed) :
    engine(seed), distribution(1) {}
void rand_exponential::seed(unsigned int const seed) { engine.seed(seed); }
float rand_exponential::operator()() {
    return distribution(engine);
}
float rand_exponential::operator()(float const r) {
    return distribution(
        engine,
        std::exponential_distribution<float>::param_type(r)
    );
}

rand_normal::rand_normal() :
    engine(al_random_seed()), distribution(0, 1) {}
rand_normal::rand_normal(unsigned int const seed) :
    engine(seed), distribution(0, 1) {}
void rand_normal::seed(unsigned int const seed) { engine.seed(seed); }
float rand_normal::operator()() {
    return distribution(engine);
}
float rand_normal::operator()(float const mean, float const stdev) {
    return distribution(
        engine,
        std::normal_distribution<float>::param_type(mean, stdev)
    );
}

rand_uniformf& rnd::global() { static rand_uniformf r; return r; }
rand_normal& rnd::global_normal() { static rand_normal n; return n; }
rand_uniformi& rnd::global_ui() { static rand_uniformi i; return i; }