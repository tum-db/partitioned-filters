#pragma once

#include <array>

static const char counters_arg[] = "--benchmark_counters_tabular=true";

static const std::array<char *, 1> extra_args{const_cast<char *>(counters_arg)};

#define MAIN()                                                          \
int main(int argc, char **argv) {                                       \
    char **new_argv = new char *[argc + extra_args.size()];             \
    for (int i = 0; i < argc; i++)                                      \
        new_argv[i] = argv[i];                                          \
    for (size_t i = 0; i < extra_args.size(); i++)                      \
        new_argv[i + argc] = &extra_args[i][0];                         \
    argc += extra_args.size();                                          \
    ::benchmark::Initialize(&argc, new_argv);                           \
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1; \
    ::benchmark::RunSpecifiedBenchmarks();                              \
    delete[](new_argv);                                                 \
}                                                                       \
int main(int, char**)

