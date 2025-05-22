#ifndef PROFILER_HPP
#define PROFILER_HPP

#ifdef ENABLE_PROFILING_RESHUFFLE
    #include <tracy/Tracy.hpp>
    #define PROFILE_SCOPE ZoneScoped
    #define PROFILE_SCOPE_NAMED(name) ZoneScopedN(name)
#else
    #define PROFILE_SCOPE
    #define PROFILE_SCOPE_NAMED(name)
#endif


#endif //PROFILER_HPP
