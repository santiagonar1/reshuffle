#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <serialize.hpp>

#include "aggregate_data.hpp"
#include "non_aggregate_data.hpp"

using namespace reshuffle::internal;

using ::testing::Eq;

TEST(Serialize, WorksOnAggregateData) {
    const auto aggregates = std::vector{AggregateData{"one", 1}, AggregateData{"two", 2}};

    const auto bytes = serialize(aggregates);
    const auto deserialized_aggregates = deserialize<AggregateData>(bytes);

    EXPECT_THAT(aggregates, Eq(deserialized_aggregates));
}

TEST(Serialize, WorksOnAggregateDataThatHasImplementedSerialize) {
    const auto non_aggregates = std::vector{NonAggregateDataSerializable{"one", 1},
                                            NonAggregateDataSerializable{"two", 2}};

    const auto bytes = serialize(non_aggregates);
    const auto deserialized_non_aggregates = deserialize<NonAggregateDataSerializable>(bytes);

    EXPECT_THAT(non_aggregates, Eq(deserialized_non_aggregates));
}

TEST(Serialize, WorksOnAggregateDataThatHasImplementedSerializeWithLookup) {
    const auto non_aggregates =
            std::vector{NonAggregateDataSerializableWithArgumentLookup{"one", 1},
                        NonAggregateDataSerializableWithArgumentLookup{"two", 2}};

    const auto bytes = serialize(non_aggregates);
    const auto deserialized_non_aggregates =
            deserialize<NonAggregateDataSerializableWithArgumentLookup>(bytes);

    EXPECT_THAT(non_aggregates, Eq(deserialized_non_aggregates));
}