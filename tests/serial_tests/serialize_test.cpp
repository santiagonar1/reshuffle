#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <serialize.hpp>

#include "aggregate_data.hpp"
#include "fixed_size_data.hpp"
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

TEST(Serialize, CanBeDoneInPlace) {
    const auto values = std::vector{FixedSizeData{1}, FixedSizeData{2}};
    auto bytes = std::vector<std::byte>(values.size() * sizeof(FixedSizeData));
    serialize(values, bytes);

    const auto deserialized_values = deserialize<FixedSizeData>(bytes);
    EXPECT_THAT(deserialized_values, Eq(values));
}

TEST(Serialize, WorksWithSpans) {
    const auto non_aggregates =
            std::vector{NonAggregateDataSerializableWithArgumentLookup{"one", 1},
                        NonAggregateDataSerializableWithArgumentLookup{"two", 2}};

    const auto bytes = serialize(std::span{non_aggregates});
    const auto deserialized_non_aggregates =
            deserialize<NonAggregateDataSerializableWithArgumentLookup>(bytes);

    EXPECT_THAT(non_aggregates, Eq(deserialized_non_aggregates));
}

TEST(Deserialize, WorksWithSpans) {
    const auto non_aggregates =
            std::vector{NonAggregateDataSerializableWithArgumentLookup{"one", 1},
                        NonAggregateDataSerializableWithArgumentLookup{"two", 2}};

    const auto bytes = serialize(non_aggregates);
    const auto deserialized_non_aggregates =
            deserialize<NonAggregateDataSerializableWithArgumentLookup>(std::span{bytes});

    EXPECT_THAT(non_aggregates, Eq(deserialized_non_aggregates));
}