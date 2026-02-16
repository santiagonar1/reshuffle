#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <iostream>
#include <mpi.h>
#include <string>
#include <vector>

#include <reshuffle.hpp>

struct StringArray {
    char **_data{};
    int _size{};

    StringArray() = default;

    explicit StringArray(const int size) : _size(size) {
        _data = static_cast<char **>(std::malloc(size * sizeof(char *)));
    }

    StringArray(const StringArray &other) : _size(other._size) {
        _data = static_cast<char **>(std::malloc(_size * sizeof(char *)));
        if (_data == nullptr) { throw std::bad_alloc(); }

        for (int i = 0; i < _size; ++i) { _data[i] = strdup(other._data[i]); }
    }

    StringArray &operator=(const StringArray &other) {
        if (this == &other) { return *this; }

        for (int i = 0; i < _size; i++) { free(_data[i]); }
        free(_data);

        _size = other._size;

        _data = static_cast<char **>(std::malloc(_size * sizeof(char *)));
        if (_data == nullptr) { throw std::bad_alloc(); }

        for (int i = 0; i < _size; ++i) { _data[i] = strdup(other._data[i]); }

        return *this;
    }

    ~StringArray() {
        for (int i = 0; i < _size; i++) { free(_data[i]); }
        free(_data);
    }
};

reshuffle::RankId get_rank(const MPI_Comm &comm = MPI_COMM_WORLD);
int get_num_ranks(const MPI_Comm &comm = MPI_COMM_WORLD);
MPI_Comm get_comm_from_pset(const std::string &pset_name, const MPI_Session &session);
bool is_dynamic_process(const MPI_Session &session);
std::string get_grown_main_pset(std::string default_pset, const MPI_Session &session);
std::string get_main_pset(const MPI_Session &session);

void request_expansion(const std::string &main_pset, const MPI_Session &session);
void request_reduction(const std::string &main_pset, const MPI_Session &session);

std::pair<int, StringArray> get_set_operation_info(std::string main_pset,
                                                   const MPI_Session &session);

std::string get_new_main_pset(std::string main_pset, std::string delta_pset,
                              const MPI_Session &session);

bool is_process_leaving(std::string delta_pset, const MPI_Session &session) {
    MPI_Info info = MPI_INFO_NULL;
    auto boolean_string = std::string{16, ' '};
    int flag{};

    /* Is proc included in the delta PSet? If yes, need to terminate */
    MPI_Session_get_pset_info(session, delta_pset.data(), &info);
    MPI_Info_get(info, "mpi_included", 6, boolean_string.data(), &flag);
    MPI_Info_free(&info);

    return strcmp(boolean_string.data(), "True") == 0;
}

ABSL_FLAG(bool, reduction, false, "Execute a reduction instead of an expansion");

int main(int argc, char **argv) {

    absl::ParseCommandLine(argc, argv);

    MPI_Session session = MPI_SESSION_NULL;

    MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_ARE_FATAL, &session);

    auto main_pset = get_main_pset(session);

    auto comm = get_comm_from_pset(main_pset, session);
    auto rank = get_rank(comm);
    auto num_ranks = get_num_ranks(comm);

    const auto dyn_proc = is_dynamic_process(session);
    auto data = dyn_proc ? std::vector<int>{} : std::vector<int>(20, 42);

    constexpr int num_adaptations = 1;
    int starting_iteration{};

    if (dyn_proc) {
        MPI_Bcast(&starting_iteration, 1, MPI_INT, 0, comm);
        data = reshuffle::shuffle(data, comm);
        std::cout << "Dynamic: After adaptation " << rank << " of " << num_ranks << " with "
                  << data.size() << " values\n";
    }

    for (int i = starting_iteration; i < num_adaptations; ++i) {
        std::cout << "Origin: Hello from rank " << rank << " of " << num_ranks << std::endl;

        /* One process needs to request the set operation and publish the kickof information */
        if (rank == 0) {
            if (absl::GetFlag(FLAGS_reduction)) {
                request_reduction(main_pset, session);
            } else {
                request_expansion(main_pset, session);
            }
        }

        /* All processes can query the information about the pending Set operation */
        auto [op, output_psets] = get_set_operation_info(main_pset, session);

        main_pset = get_new_main_pset(main_pset, output_psets._data[0], session);

        MPI_Comm old_comm = comm;

        if (is_process_leaving(output_psets._data[0], session)) {
            data = reshuffle::shuffle(data, old_comm, MPI_COMM_NULL);
            break;
        }

        comm = get_comm_from_pset(main_pset, session);
        rank = get_rank(comm);
        num_ranks = get_num_ranks(comm);

        if (op == MPI_PSETOP_GROW) {
            int next_iter = i + 1;
            MPI_Bcast(&next_iter, 1, MPI_INT, 0, comm);
            data = reshuffle::shuffle(data, comm);
        } else if (op == MPI_PSETOP_SHRINK) {
            data = reshuffle::shuffle(data, old_comm, comm);
        }

        std::cout << "Origin: After adaptation " << rank << " of " << num_ranks << " with "
                  << data.size() << " values\n";

        MPI_Comm_disconnect(&old_comm);

        /* Indicate completion of the Pset operation*/
        if (rank == 0) { MPI_Session_dyn_finalize_psetop(session, main_pset.data()); }
    }

    /* Disconnect from the old communicator */
    MPI_Comm_disconnect(&comm);

    /* Finalize the MPI Session */
    MPI_Session_finalize(&session);

    return 0;
}

void request_reduction(const std::string &main_pset, const MPI_Session &session) {
    int op = MPI_PSETOP_SHRINK;
    const int num_shrink_proc = 2;

    MPI_Info info = MPI_INFO_NULL;
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_num_procs_sub", std::to_string(num_shrink_proc).data());

    StringArray input_psets(1);
    StringArray output_psets;
    input_psets._data[0] = strdup(main_pset.data());

    int noutput{};

    /* Send the Set Operation request */
    MPI_Session_dyn_v2a_psetop(session, &op, input_psets._data, 1, &output_psets._data, &noutput,
                               info);
    output_psets._size = noutput;
    MPI_Info_free(&info);

    /* Publish the name of the new main PSet on the delta Pset */
    MPI_Info_create(&info);
    MPI_Info_set(info, "main_pset", output_psets._data[1]);
    MPI_Session_set_pset_data(session, output_psets._data[0], info);
    MPI_Info_free(&info);
}

void request_expansion(const std::string &main_pset, const MPI_Session &session) {
    int op = MPI_PSETOP_GROW;
    const int num_add_proc = 2;

    MPI_Info info = MPI_INFO_NULL;
    MPI_Info_create(&info);
    MPI_Info_set(info, "mpi_num_procs_add", std::to_string(num_add_proc).data());

    StringArray input_psets(1);
    StringArray output_psets;
    input_psets._data[0] = strdup(main_pset.data());

    int noutput{};

    /* Send the Set Operation request */
    MPI_Session_dyn_v2a_psetop(session, &op, input_psets._data, 1, &output_psets._data, &noutput,
                               info);
    output_psets._size = noutput;
    MPI_Info_free(&info);

    /* Publish the name of the new main PSet on the delta Pset */
    MPI_Info_create(&info);
    MPI_Info_set(info, "main_pset", output_psets._data[1]);
    MPI_Session_set_pset_data(session, output_psets._data[0], info);
    MPI_Info_free(&info);
}

std::pair<int, StringArray> get_set_operation_info(std::string main_pset,
                                                   const MPI_Session &session) {
    StringArray output_psets;
    int n_output{}, op{};
    MPI_Session_dyn_v2a_query_psetop(session, main_pset.data(), main_pset.data(), &op,
                                     &output_psets._data, &n_output);
    output_psets._size = n_output;

    return {op, output_psets};
}


reshuffle::RankId get_rank(const MPI_Comm &comm) {
    reshuffle::RankId id{};
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
    StringArray dict_key(1);
    dict_key._data[0] = strdup("main_pset");
    int flag{};
    MPI_Info info = MPI_INFO_NULL;

    /* Lookup the value for the "grown_main_pset" key in the PSet Dictionary and use it as our main PSet */
    MPI_Session_get_pset_data(session, default_pset.data(), default_pset.data(), dict_key._data, 1,
                              true, &info);
    MPI_Info_get(info, "main_pset", MPI_MAX_PSET_NAME_LEN, main_pset.data(), &flag);
    MPI_Info_free(&info);

    return main_pset;
}

std::string get_main_pset(const MPI_Session &session) {
    auto default_pset = std::string{"mpi://WORLD"};

    if (is_dynamic_process(session)) { return get_grown_main_pset(default_pset, session); }

    return default_pset;
}

std::string get_new_main_pset(std::string main_pset, std::string delta_pset,
                              const MPI_Session &session) {
    main_pset.reserve(MPI_MAX_PSET_NAME_LEN);
    MPI_Info info = MPI_INFO_NULL;
    StringArray dict_key(1);
    dict_key._data[0] = strdup("main_pset");
    int flag{};

    MPI_Session_get_pset_data(session, main_pset.data(), delta_pset.data(), dict_key._data, 1, true,
                              &info);
    MPI_Info_get(info, "main_pset", MPI_MAX_PSET_NAME_LEN, main_pset.data(), &flag);
    MPI_Info_free(&info);

    return main_pset;
}