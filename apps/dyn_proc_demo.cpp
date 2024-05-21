#include <iostream>
#include <mpi.h>
#include <string>
#include <vector>

#include <reshuffle.hpp>

reshuffle::rank_id get_rank(const MPI_Comm &comm = MPI_COMM_WORLD);
int get_num_ranks(const MPI_Comm &comm = MPI_COMM_WORLD);
MPI_Comm get_comm_from_pset(const std::string &pset_name, const MPI_Session &session);
bool is_dynamic_process(const MPI_Session &session);
std::string get_grown_main_pset(std::string default_pset, const MPI_Session &session);
std::string get_main_pset(const MPI_Session &session);

void free_string_array(char **array, int size) {
    for (int i = 0; i < size; i++) { free(array[i]); }
    free(array);
}

void request_expansion(const std::string &main_pset, const MPI_Session &session);

int main() {
    MPI_Session session = MPI_SESSION_NULL;

    MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_ARE_FATAL, &session);

    auto main_pset = get_main_pset(session);

    auto comm = get_comm_from_pset(main_pset, session);
    auto rank = get_rank(comm);
    auto num_ranks = get_num_ranks(comm);

    const auto dyn_proc = is_dynamic_process(session);

    /* Original processes will switch to a grown communicator */
    if (not dyn_proc) {
        std::cout << "Origin: Hello from rank " << rank << " of " << num_ranks << std::endl;

        /* One process needs to request the set operation and publish the kickof information */
        if (rank == 0) { request_expansion(main_pset, session); }

        int op = MPI_PSETOP_GROW;
        char **output_psets;
        MPI_Info info = MPI_INFO_NULL;
        int noutput{};
        char *dict_key = strdup("main_pset");
        int flag{};


        /* All processes can query the information about the pending Set operation */
        MPI_Session_dyn_v2a_query_psetop(session, main_pset.data(), main_pset.data(), &op,
                                         &output_psets, &noutput);

        /* Lookup the name of the new main PSet stored on the delta PSet */
        main_pset.reserve(MPI_MAX_PSET_NAME_LEN);
        MPI_Session_get_pset_data(session, main_pset.data(), output_psets[0], (char **) &dict_key,
                                  1, true, &info);
        MPI_Info_get(info, "main_pset", MPI_MAX_PSET_NAME_LEN, main_pset.data(), &flag);
        free_string_array(output_psets, noutput);
        MPI_Info_free(&info);

        /* Disconnect from the old communicator */
        MPI_Comm old_comm = comm;
        comm = get_comm_from_pset(main_pset, session);
        rank = get_rank(comm);
        num_ranks = get_num_ranks(comm);

        std::cout << "Origin: After adaptation " << rank << " of " << num_ranks << std::endl;

        auto data = std::vector<int>(20, 42);
        data = reshuffle::shuffle(data, comm);

        MPI_Comm_disconnect(&old_comm);

        /* Indicate completion of the Pset operation*/
        if (rank == 0) { MPI_Session_dyn_finalize_psetop(session, main_pset.data()); }
    } else {
        auto data = std::vector<int>{};
        data = reshuffle::shuffle(data, comm);
        std::cout << "Dynamic: After adaptation " << rank << " of " << num_ranks << " with "
                  << data.size() << " values\n";
    }

    /* Disconnect from the old communicator */
    MPI_Comm_disconnect(&comm);

    /* Finalize the MPI Session */
    MPI_Session_finalize(&session);

    return 0;
}

void request_expansion(const std::string &main_pset, const MPI_Session &session) {
    int op = MPI_PSETOP_GROW;
    const int num_add_proc = 2;

    MPI_Info info = MPI_INFO_NULL;
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_num_procs_add", std::to_string(num_add_proc).data());

    char **input_psets = (char **) malloc(1 * sizeof(char *));
    char **output_psets;
    input_psets[0] = strdup(main_pset.data());

    int noutput{};

    /* Send the Set Operation request */
    MPI_Session_dyn_v2a_psetop(session, &op, input_psets, 1, &output_psets, &noutput, info);
    MPI_Info_free(&info);

    /* Publish the name of the new main PSet on the delta Pset */
    MPI_Info_create(&info);
    MPI_Info_set(info, "main_pset", output_psets[1]);
    MPI_Session_set_pset_data(session, output_psets[0], info);
    MPI_Info_free(&info);
    free_string_array(input_psets, 1);
    free_string_array(output_psets, noutput);
}


reshuffle::rank_id get_rank(const MPI_Comm &comm) {
    reshuffle::rank_id id{};
    MPI_Comm_rank(comm, &id);

    return id;
}

int get_num_ranks(const MPI_Comm &comm) {
    int num_ranks{};
    MPI_Comm_size(comm, &num_ranks);

    return num_ranks;
}

MPI_Comm get_comm_from_pset(const std::string &pset_name, const MPI_Session &session) {
    MPI_Group group = MPI_GROUP_NULL;
    MPI_Comm comm = MPI_COMM_NULL;

    MPI_Group_from_session_pset(session, pset_name.data(), &group);
    MPI_Comm_create_from_group(group, "mpi.forum.example", MPI_INFO_NULL, MPI_ERRORS_RETURN, &comm);
    MPI_Group_free(&group);

    return comm;
}

bool is_dynamic_process(const MPI_Session &session) {
    MPI_Info info = MPI_INFO_NULL;
    auto boolean_string = std::string{16, ' '};
    const auto pset_name = std::string{"mpi://WORLD"};
    int flag{};

    MPI_Session_get_pset_info(session, pset_name.data(), &info);

    /* get value for the 'mpi_dyn' key -> if true, this process was added dynamically */
    MPI_Info_get(info, "mpi_dyn", 6, boolean_string.data(), &flag);
    MPI_Info_free(&info);

    //TODO: For some reason doing boolean_string == "True" fails.
    return flag and strcmp(boolean_string.data(), "True") == 0;
}

std::string get_grown_main_pset(std::string default_pset, const MPI_Session &session) {
    auto main_pset = std::string(MPI_MAX_PSET_NAME_LEN, ' ');
    char *dict_key = strdup("main_pset");
    int flag{};
    MPI_Info info = MPI_INFO_NULL;

    /* Lookup the value for the "grown_main_pset" key in the PSet Dictionary and use it as our main PSet */
    MPI_Session_get_pset_data(session, default_pset.data(), default_pset.data(),
                              (char **) &dict_key, 1, true, &info);
    MPI_Info_get(info, "main_pset", MPI_MAX_PSET_NAME_LEN, main_pset.data(), &flag);
    MPI_Info_free(&info);

    return main_pset;
}

std::string get_main_pset(const MPI_Session &session) {
    auto default_pset = std::string{"mpi://WORLD"};

    if (is_dynamic_process(session)) { return get_grown_main_pset(default_pset, session); }

    return default_pset;
}