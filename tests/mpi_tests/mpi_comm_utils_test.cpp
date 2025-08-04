#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi_comm_utils.hpp>
#include <mpi_utils.hpp>

#include "../serial_tests/include/non_aggregate_data.hpp"
#include "aggregate_data.hpp"

using namespace reshuffle::internal;
using namespace reshuffle::mpi;

using ::testing::Eq;
