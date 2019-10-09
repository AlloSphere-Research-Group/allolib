#include "al/math/al_StdRandom.hpp"

using namespace std;

unsigned int al_random_seed() {
  static std::random_device d;
  return d();
}

using namespace al;

rand_uniformi::rand_uniformi() : distribution(0, 100) {
  seed_seq s{al_random_seed(), al_random_seed(), al_random_seed(),
             al_random_seed()};
  engine.seed(s);
}
rand_uniformi::rand_uniformi(unsigned int seed)
    : engine(seed), distribution(0, 100) {}
void rand_uniformi::seed(unsigned int seed) { engine.seed(seed); }
int rand_uniformi::operator()() { return distribution(engine); }
int rand_uniformi::operator()(int b) { return (*this)(0, b); }
int rand_uniformi::operator()(int a, int b) {
  return distribution(engine,
                      std::uniform_int_distribution<int>::param_type(a, b));
}

rand_uniformf::rand_uniformf() : distribution(0, 1) {
  seed_seq s{al_random_seed(), al_random_seed(), al_random_seed(),
             al_random_seed()};
  engine.seed(s);
}
rand_uniformf::rand_uniformf(unsigned int seed)
    : engine(seed), distribution(0, 1) {}
void rand_uniformf::seed(unsigned int seed) { engine.seed(seed); }
float rand_uniformf::operator()() { return distribution(engine); }
float rand_uniformf::operator()(float b) { return (*this)(0, b); }
float rand_uniformf::operator()(float a, float b) {
  return distribution(engine,
                      std::uniform_real_distribution<float>::param_type(a, b));
}

rand_poisson::rand_poisson() : distribution(100) {
  seed_seq s{al_random_seed(), al_random_seed(), al_random_seed(),
             al_random_seed()};
  engine.seed(s);
}
rand_poisson::rand_poisson(unsigned int seed)
    : engine(seed), distribution(100) {}
void rand_poisson::seed(unsigned int seed) { engine.seed(seed); }
int rand_poisson::operator()() { return distribution(engine); }
int rand_poisson::operator()(float m) {
  return distribution(engine, std::poisson_distribution<int>::param_type(m));
}

rand_exponential::rand_exponential() : distribution(1) {
  seed_seq s{al_random_seed(), al_random_seed(), al_random_seed(),
             al_random_seed()};
  engine.seed(s);
}
rand_exponential::rand_exponential(unsigned int seed)
    : engine(seed), distribution(1) {}
void rand_exponential::seed(unsigned int seed) { engine.seed(seed); }
float rand_exponential::operator()() { return distribution(engine); }
float rand_exponential::operator()(float r) {
  return distribution(engine,
                      std::exponential_distribution<float>::param_type(r));
}

rand_normal::rand_normal() : distribution(0, 1) {
  seed_seq s{al_random_seed(), al_random_seed(), al_random_seed(),
             al_random_seed()};
  engine.seed(s);
}
rand_normal::rand_normal(unsigned int seed)
    : engine(seed), distribution(0, 1) {}
void rand_normal::seed(unsigned int seed) { engine.seed(seed); }
float rand_normal::operator()() { return distribution(engine); }
float rand_normal::operator()(float mean, float stdev) {
  return distribution(engine,
                      std::normal_distribution<float>::param_type(mean, stdev));
}

rand_uniformf& rnd::global() {
  static rand_uniformf r;
  return r;
}
rand_normal& rnd::global_normal() {
  static rand_normal n;
  return n;
}
rand_uniformi& rnd::global_ui() {
  static rand_uniformi i;
  return i;
}