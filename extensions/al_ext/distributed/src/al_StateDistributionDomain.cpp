#include "al_StateDistributionDomain.hpp"

using namespace al;

bool StateDistributionDomain::initialize(ComputationDomain *parent) {
  initializeSubdomains(true);
  initializeSubdomains(false);
  return true;
}

bool StateDistributionDomain::tick() {

  tickSubdomains(true);
  tickSubdomains(false);
  return true;
}

bool StateDistributionDomain::cleanup(ComputationDomain *parent) {
  cleanupSubdomains(true);
  cleanupSubdomains(false);
  return true;

}


