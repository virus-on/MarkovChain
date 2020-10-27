#include "MarkovChain.h"

#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>


int main(int argc, char *argv[])
{
    using namespace std::chrono;

    if (argc == 2)
    {
        auto start = high_resolution_clock::now();
        // Init Dictionary with maximum depth 3
        // This includes sequence start token, which will be the root for all tokens
        MarcovChain::Dictionary<std::string, 3, MarcovChain::StorageStrategy::Hash> dict; // in this test MarcovChain::StorageStrategy::Hash is faster by 25 ms in avarage on my setup
        // MarcovChain::StorageStrategy says what to use as a unique token storage - std::set or std::unordered_set

        // Read file line by line
        std::ifstream inputFile(argv[1]);
        if (inputFile.is_open())
        {
            std::string line;
            while (std::getline(inputFile, line))
            {
                // Split line into words, use it as a token sequence for dictionary
                std::stringstream ss(line);
                std::istream_iterator<std::string> begin(ss);
                std::istream_iterator<std::string> end;
                std::vector<std::string> vstrings(begin, end);
                dict.addDataToDictionary<decltype(vstrings.begin())>(vstrings.begin(), vstrings.end());
            }

            for (int i = 0; i < 100; ++i)
            {
                std::vector<decltype(dict)::output_value_type> genOutput;
                decltype(dict)::output_value_type genVal =  nullptr;
                do
                {
                   // Gen next token using previously generated tokens
                   // Window Size = 2, value will be generated using last 2 elements of genOutput
                   genVal =  dict.getToken<decltype(genOutput), 2>(genOutput.begin(), genOutput.end());
                   if (genVal == nullptr)
                       break;
                   genOutput.push_back(genVal);
                } while(genVal);

                std::vector<std::string> tokenList;
                tokenList.reserve(genOutput.size());
                for (const auto& token: genOutput)
                    tokenList.push_back(*token);

                std::copy(tokenList.begin(), tokenList.end(), std::ostream_iterator<std::string>(std::cout, " "));
                std::cout << std::endl;
            }
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            std::cout << std::endl;
            std::cout << "Program Duration: " << duration.count() << "ms" << std::endl;
        }
        else
        {
           std::cout << "Can't open the input file!" << std::endl;
        }
    }

    return 0;
}
