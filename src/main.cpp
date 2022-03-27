#include "config.h"
#include "io.h"
#include "Map3d.h"

void printWelcome() {
//    printf("\e[?25l");
    auto logo{
            R"(
    #==============================================================#
    #                        __                                    #
    #                   __  |''|                                   #
    #                  |""| |''|  _   /|__                         #
    #                __|""| |''|_| | | |""|/\_                     #
    #               |''|""| |''|'| __| |""|'''|  _____             #
    #          _ _  |''|""|^|''|'||""| |""|'''| |"""""|            #
    #         |"|"| |''|""|||''|'||""| |""|'''| |"""""|            #
    #     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~     #
    #    ____   _   _                ___________________________   #
    #   / ___| /_\ | |_   _   _      __  ____/___  ____/___  __ \  #
    #  | |     |"| | __| | | | |     _  /     __  /_    __  / / /  #
    #  | |___  |"| | |_  | |_| |  4  / /___   _  __/    _  /_/ /   #
    #   \____| |"|  \__|  \__, |     \____/   /_/       /_____/    #
    #                     |___/                                    #
    #                                                              #
    #==============================================================#
)"
    };

    std::cout << logo;
    std::cout << "City4CFD Copyright (C) 2021-2022 3D geoinformation research group, TU Delft\n" << std::endl;
}

void printHelp() {
    auto helpMsg{
R"(
USAGE:
    City4CFD config_file.json OPTIONS

AVAILABLE OPTIONS:
    --help            Prints out this help message
    --output_dir      Sets the directory where output files are stored
    --output_file     Overrides output file(s) name from the configuration file
)"
    };

    std::cout << helpMsg;
}

int main(int argc, char** argv) {
    try {
        printWelcome();

        auto startTime = std::chrono::steady_clock::now();

        std::string config_path;
        //-- Path to config.json file
        if (argc >= 2) {
            config_path = fs::current_path().append(argv[1]).string();
        } else {
            printHelp();
            return EXIT_SUCCESS;
        }

        //-- Input arguments
        for (auto i = 1; i < argc; ++i) {
            if (boost::iequals(argv[i], "--help")) {
                printHelp();
                return EXIT_SUCCESS;
            } else if (boost::iequals(argv[i], "--output_dir")) {
                if (i + 1 == argc) throw std::invalid_argument("Missing argument for --output_dir");

                config::outputDir = fs::absolute(fs::current_path().append(argv[++i]));
                if (!fs::exists(config::outputDir)) throw std::invalid_argument("Output directory does not exist!");
            } else if (boost::iequals(argv[i], "--output_file")) {
                if (i + 1 == argc) throw std::invalid_argument("Missing argument for --output_file");

                config::outputFileName = argv[++i];
            } else {
                if (i > 1) throw std::invalid_argument(std::string("Unknown option " + std::string(argv[i])));
            }
        }

        //-- Read configuration file
        IO::read_config(config_path);

        //-- Create the main class
        Map3d map3d;

        //-- Read point cloud and polygons, and store them in Map3d
        map3d.read_data();

        //-- Calculate elevations and triangulate
        map3d.reconstruct();

        //-- Output data
        map3d.output();

        //-- Output log
        IO::output_log();

        auto endTime = std::chrono::steady_clock::now();
        auto diffTime = endTime - startTime;
        std::cout << "\nProgram executed in " << std::chrono::duration<double>(diffTime).count() << " s" << std::endl;
        std:: cout << "End" << std::endl;

        return EXIT_SUCCESS;
    } catch (std::exception& e) {
        //-- Output log
        IO::output_log();

        std::cerr << "\nProgram failed! Reason: " << e.what() << std::endl;
        std::cout << "End" << std::endl;
        return EXIT_FAILURE;
    }
}