#include "cache_factory.hpp"
#include "command_line_interface.hpp"
#include "opt.hpp"
#include "statuses.hpp"
#include "subsidiary.hpp"

#include "report.hpp"
#include "experiment.hpp"

#include <cstddef>
#include <iostream>

namespace {

constexpr int kReportedSeed = 1;

int ExitCode(Status status) {
    return static_cast<int>(status);
}

} // namespace

int main(int argc, char** argv) {
    CLI::Config config;
    Status status = CLI::ParseArgs(argc, argv, config);
    if (status != Status::kOk) {
        std::cerr << "--help will work later";
        return ExitCode(status);
    }

    if (config.show_help) {
        // потом допишу как-нибудь
        return ExitCode(Status::kOk);
    }

    std::vector<Result> results;
    status = RunExperiment(config, results);
    if (status != Status::kOk) {
        return ExitCode(status);
    }

    std::cout << "pattern hot, trace length: " << config.trace_length
              << ", seeds: " << config.seeds << " (table for seed " << kReportedSeed << ")\n\n";
    PrintTable(results, kReportedSeed);

    status = WriteCsv(config.output_path, results);
    if (status != Status::kOk) {
        return ExitCode(status);
    }

    std::cout << "\nfull data: " << config.output_path << "\n";
    return ExitCode(Status::kOk);
}
