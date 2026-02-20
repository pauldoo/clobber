#include "random.hpp"

#include <random>

std::random_device::result_type make_random_seed() {
    return (std::random_device())();
}
