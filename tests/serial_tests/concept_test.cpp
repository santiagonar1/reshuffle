#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts.hpp>

#include "aggregate_data.hpp"
#include "fixed_size_data.hpp"
#include "non_aggregate_data.hpp"

using namespace reshuffle::concepts;


TEST(AnAggregate, RepresentsAggregateData) {
    static_assert(Aggregate<AggregateData>, "An Aggregate must represent AggregateData");
}

TEST(AnAggregate, CannotHaveParametrizedConstructor) {
    static_assert(!Aggregate<NonAggregateData>,
                  "An Aggregate must not have a parametrized constructor");
}

TEST(AnAggregate, IsSerializable) {
    static_assert(Serializable<AggregateData>, "An Aggregate must be Serializable");
}

TEST(ANonAggregate, IsNotSerializableByDefault) {
    static_assert(!Serializable<NonAggregateData>,
                  "A NonAggregate must not be Serializable by default");
}

TEST(ANonAggregate, NeedsToImplementSerializeToBeSerializable) {
    static_assert(Serializable<NonAggregateDataSerializable>,
                  "A NonAggregate must be Serializable if it implements serialize");
    static_assert(Serializable<NonAggregateDataSerializableWithArgumentLookup>,
                  "A NonAggregate must be Serializable if it implements serialize with lookup");
}

TEST(AnAggregate, NeedsSerialization) {
    static_assert(NeedsSerialization<AggregateData>, "An Aggregate needs serialization");
}

TEST(ANonAggregate, NeedsSerialization) {
    static_assert(NeedsSerialization<NonAggregateData>, "A NonAggregate needs serialization");
}

TEST(ABasicDatatype, IsAFundamentalType) {
    static_assert(FundamentalType<int>, "An int must be a FundamentalType");
    static_assert(FundamentalType<float>, "A float must be a FundamentalType");
    static_assert(FundamentalType<double>, "A double must be a FundamentalType");
    static_assert(FundamentalType<char>, "A char must be a FundamentalType");
}

TEST(ABasicDataType, DoesNotRequireSerialization) {
    static_assert(!NeedsSerialization<int>, "A basic datatype does not require serialization");
}

TEST(FixedSizedData, CanBeMarkedAsSo) {
    static_assert(FixedSizeSerializable<FixedSizeData>, "A FixedSizeData must be fixed size");
}

TEST(AClassNotMarkedAsFixedZise, IsNotFixedSized) {
    static_assert(!FixedSizeSerializable<AggregateData>,
                  "A class not marked as fixed size is not fixed size");
}