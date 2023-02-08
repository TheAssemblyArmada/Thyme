#include "bufffileclass.h"
#include "compressionmanager.h"
#include <cxxopts.hpp>

int main(int argc, char **argv)
{
    cxxopts::Options options(
        "Compressor", "A commandline tool to encode/decode from all SAGE formats. By default compress FILES");

    options.allow_unrecognised_options();

    // clang-format off
    options.add_options()
    ("i,input", "input file", cxxopts::value<std::string>())
    ("o,output", "output file", cxxopts::value<std::string>())
    ("d,decompress", "decompress the input")
    ("t,type", "specify the encoding format (encode only)", cxxopts::value<std::string>())
    ("h,help", "print usage")
    ("v,verbose", "verbose output", cxxopts::value<bool>()->default_value("false"))
    ;
    // clang-format on

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if (result.count("input") != 1) {
        std::cerr << "Please specify exactly one input" << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    if (result.count("output") != 1) {
        std::cerr << "Please specify exactly one output" << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    auto input_path = result["input"].as<std::string>();
    BufferedFileClass input_file(input_path.c_str());
    if (!input_file.Open(FM_READ)) {
        std::cerr << "Failed to open input file:" << input_path << std::endl;
        return EXIT_FAILURE;
    }

    auto output_path = result["output"].as<std::string>();
    BufferedFileClass output_file(output_path.c_str());
    if (!output_file.Open(FM_WRITE)) {
        std::cerr << "Failed to open output file:" << output_path << std::endl;
        return EXIT_FAILURE;
    }

    auto input_size = input_file.Size();
    std::unique_ptr<uint8_t[]> input_data(new uint8_t[input_size]);
    if (input_file.Read(input_data.get(), input_size) != input_size) {
        std::cerr << "Failed to read input data completly" << std::endl;
        return EXIT_FAILURE;
    }

    // Decompression
    if (result["decompress"].as<bool>()) {
        if (!CompressionManager::Is_Data_Compressed(input_data.get(), input_size)) {
            std::cerr << "Input data is not compressed with a supported compression" << std::endl;
            return EXIT_FAILURE;
        }

        auto output_size = CompressionManager::Get_Uncompressed_Size(input_data.get(), input_size);
        std::unique_ptr<uint8_t[]> output_data(new uint8_t[output_size]);

        if (CompressionManager::Decompress_Data(input_data.get(), input_size, output_data.get(), output_size)
            != output_size) {
            std::cerr << "Failed to decompress data correctly" << std::endl;
            return EXIT_FAILURE;
        }

        if (output_file.Write(output_data.get(), output_size) != output_size) {
            std::cerr << "Failed to write output data completly" << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        CompressionType type = CompressionManager::Get_Prefered_Compression();
        if (result.count("type") > 0) {
            auto compr_name = result["type"].as<std::string>();
            type = CompressionManager::Get_Compression_Type(compr_name.c_str(), compr_name.size() + 5);

            if (type == COMPRESSION_NONE) {
                std::cerr << "Unknown compression type: " << compr_name << std::endl;
                return EXIT_FAILURE;
            }
        }

        auto output_size = CompressionManager::Get_Max_Compressed_Size(input_size, type);
        std::unique_ptr<uint8_t[]> output_data(new uint8_t[output_size]);

        output_size = CompressionManager::Compress_Data(type, input_data.get(), input_size, output_data.get(), output_size);
        if (output_size == 0) {
            std::cerr << "Failed to compress data correctly" << std::endl;
            return EXIT_FAILURE;
        }

        if (output_file.Write(output_data.get(), output_size) != output_size) {
            std::cerr << "Failed to write output data completly" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
