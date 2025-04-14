#ifndef PROCESSOR_GRID_HPP
#define PROCESSOR_GRID_HPP

namespace reshuffle::dev {
    class ProcessorGrid {
    public:
        explicit ProcessorGrid(int num_processors);

        [[nodiscard]] auto get_num_processors() const -> int;

        auto operator==(const ProcessorGrid &other) const -> bool;

    private:
        const int _num_processors;
    };
}// namespace reshuffle::dev

#endif//PROCESSOR_GRID_HPP
