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

TEST(AnMPIType, IsAnyTypeThatCanBeRepresentedByDefaultMPIDatatypes) {
    static_assert(MPIType<int>, "An int must be a MPIType");
    static_assert(MPIType<float>, "An float must be a MPIType");
    static_assert(MPIType<double>, "An double must be a MPIType");
    static_assert(MPIType<std::byte>, "An std::byte must be a MPIType");
    static_assert(MPIType<char>, "An char must be a MPIType");
    static_assert(MPIType<unsigned char>, "An unsigned char must be a MPIType");
    static_assert(MPIType<short>, "A short must be a MPIType");
    static_assert(MPIType<unsigned short>, "An unsigned short must be a MPIType");
    static_assert(MPIType<unsigned int>, "An unsigned int must be a MPIType");
    static_assert(MPIType<long>, "A long must be a MPIType");
    static_assert(MPIType<unsigned long>, "An unsigned long must be a MPIType");
    static_assert(MPIType<long long>, "A long long must be a MPIType");
    static_assert(MPIType<unsigned long long>, "An unsigned long long must be a MPIType");
    static_assert(MPIType<long double>, "A long double must be a MPIType");
    static_assert(MPIType<bool>, "An bool must be a MPIType");
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

TEST(AnInt, IsAnInteger) { static_assert(Int<int>, "An int must be an integer"); }

TEST(AnInt, AnIntReferenceIsAnInt) { static_assert(Int<int &>, "An int& must be an integer"); }

TEST(AnInt, AConstReferenceToIntIsInt) {
    static_assert(Int<const int &>, "A const int& must be an integer");
}