#ifndef CONTIGUOUSMPIDATATYPE_HPP
#define CONTIGUOUSMPIDATATYPE_HPP

#include <mpi.h>

namespace reshuffle::mpi {


    class ContiguousMPIDatatype {


    public:
        ContiguousMPIDatatype(MPI_Datatype base_datatype, int num_consecutive_elements);
        ~ContiguousMPIDatatype();
        [[nodiscard]] MPI_Datatype get_datatype() const { return _datatype; }

    private:
        MPI_Datatype _datatype{};
        static auto get_contiguous_datatype(MPI_Datatype base_datatype,
                                            int num_consecutive_elements) -> MPI_Datatype;
    };


}// namespace reshuffle::mpi

#endif//CONTIGUOUSMPIDATATYPE_HPP
