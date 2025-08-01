
#include "contiguous_mpi_datatype.hpp"

namespace reshuffle::mpi {
    ContiguousMPIDatatype::ContiguousMPIDatatype(MPI_Datatype base_datatype,
                                                 const int num_consecutive_elements)
        : _datatype(get_contiguous_datatype(base_datatype, num_consecutive_elements)) {}

    ContiguousMPIDatatype::~ContiguousMPIDatatype() { MPI_Type_free(&_datatype); }

    auto ContiguousMPIDatatype::get_contiguous_datatype(MPI_Datatype base_datatype,
                                                        const int num_consecutive_elements)
            -> MPI_Datatype {
        MPI_Datatype contiguous_datatype;
        MPI_Type_contiguous(num_consecutive_elements, base_datatype, &contiguous_datatype);
        MPI_Type_commit(&contiguous_datatype);

        return contiguous_datatype;
    }
}// namespace reshuffle::mpi