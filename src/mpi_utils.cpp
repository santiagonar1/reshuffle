#include "mpi_utils.hpp"

#include <numeric>

namespace reshuffle::mpi {
    namespace internal {
        auto enable_mpi_errors_return(MPI_Comm comm) -> MPI_Errhandler {
            MPI_Errhandler current_errhandler;
            MPI_Comm_get_errhandler(comm, &current_errhandler);
            MPI_Comm_set_errhandler(comm, MPI_ERRORS_RETURN);

            return current_errhandler;
        }

        auto get_num_elements(const MPI_Status &status, const MPI_Datatype &datatype) -> int {
            int count{};
            MPI_Get_count(&status, datatype, &count);
            return count;
        }

        auto block_until_message_is_received(const MPI_Datatype &datatype, const MPI_Comm &comm)
                -> ReceivedMessageInformation {
            MPI_Status status{};
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
            return {status.MPI_SOURCE, status.MPI_TAG, get_num_elements(status, datatype)};
        }
    }// namespace internal

    auto get_rank_id(const MPI_Comm &comm) -> rank_id {
        int rank{MPI_ERR_RANK};

        if (is_comm_null(comm)) {
            throw std::invalid_argument("Invalid MPI_COMM_NULL communicator");
        }

        MPI_Comm_rank(comm, &rank);
        return rank;
    }

    auto is_root(const MPI_Comm &comm) -> bool {
        return belongs_to_comm(comm) and get_rank_id(comm) == 0;
    }

    auto get_num_ranks(const MPI_Comm &comm) -> int {
        int num_ranks{};

        if (is_comm_null(comm)) {
            throw std::invalid_argument("Invalid MPI_COMM_NULL communicator");
        }

        MPI_Comm_size(comm, &num_ranks);
        return num_ranks;
    }

    auto is_comm_null(const MPI_Comm &comm) -> bool { return comm == MPI_COMM_NULL; }


    auto get_sub_comm(const MPI_Comm &base_comm, const std::vector<rank_id> &ranks) -> MPI_Comm {
        const auto base_group = get_group(base_comm).value();
        const auto sub_group = get_sub_group(base_group, ranks);
        auto sub_comm{MPI_COMM_NULL};

        MPI_Comm_create_group(base_comm, sub_group, 1, &sub_comm);
        return sub_comm;
    }

    auto get_group(const MPI_Comm &comm) -> std::optional<MPI_Group> {
        if (is_comm_null(comm)) { return std::nullopt; }

        MPI_Group group;

        const auto current_errhandler = internal::enable_mpi_errors_return(MPI_COMM_WORLD);
        const int err = MPI_Comm_group(comm, &group);
        MPI_Comm_set_errhandler(MPI_COMM_WORLD, current_errhandler);


        return err == MPI_SUCCESS ? std::optional{group} : std::nullopt;
    }

    auto belongs_to_comm(const MPI_Comm &comm) -> bool {
        int dummy_rank{};

        if (comm == MPI_COMM_NULL) { return false; }

        const MPI_Errhandler current_errhandler =
                internal::enable_mpi_errors_return(MPI_COMM_WORLD);
        const int err = MPI_Comm_rank(comm, &dummy_rank);
        MPI_Comm_set_errhandler(MPI_COMM_WORLD, current_errhandler);

        return err == MPI_SUCCESS;
    }

    auto get_sub_group(const MPI_Group &group, const std::vector<rank_id> &ranks) -> MPI_Group {
        MPI_Group sub_group;
        MPI_Group_incl(group, static_cast<int>(ranks.size()), ranks.data(), &sub_group);
        return sub_group;
    }

    auto is_sub_comm(const MPI_Comm &comm, const MPI_Comm &possible_sub_comm) -> bool {
        auto const group = get_group(comm);
        auto const subgroup = get_group(possible_sub_comm);

        auto local_result{false};
        if (group.has_value() and subgroup.has_value()) {
            MPI_Group intersection;
            MPI_Group_intersection(group.value(), subgroup.value(), &intersection);
            int comp;
            MPI_Group_compare(intersection, subgroup.value(), &comp);

            // check if intersection == sub_comm (meaning that sub_comm is a subset of comm)
            local_result = comp != MPI_UNEQUAL;
        } else {
            local_result = false;
        }

        auto is_sub_comm{false};

        MPI_Allreduce(&local_result, &is_sub_comm, 1, MPI_CXX_BOOL, MPI_LOR, MPI_COMM_WORLD);

        return is_sub_comm;
    }

}// namespace reshuffle::mpi
