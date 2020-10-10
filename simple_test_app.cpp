#include "MarcovChain.h"

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
        MarcovChain::Dictionary<std::string, 3> dict;

        std::ifstream inputFile(argv[1]);
        if (inputFile.is_open())
        {
            std::string line;
            while (std::getline(inputFile, line))
            {
                std::stringstream ss(line);
                std::istream_iterator<std::string> begin(ss);
                std::istream_iterator<std::string> end;
                std::vector<std::string> vstrings(begin, end);
                dict.addDataToDictionary<decltype(vstrings.begin())>(vstrings.begin(), vstrings.end());
            }

            for (int i = 0; i < 100; ++i)
            {
                std::vector<const std::string*> genOutput;
                const std::string* genVal =  nullptr;
                do
                {
                   genVal =  dict.getToken<decltype(genOutput)>(genOutput.begin(), genOutput.end());
                   if (not genVal)
                       break;
                   genOutput.push_back(genVal);
                } while(genVal);

                std::vector<std::string> tokenList;

                for (const auto& token: genOutput)
                    tokenList.push_back(*token);

                std::copy(tokenList.begin(), tokenList.end(), std::ostream_iterator<std::string>(std::cout, " "));
                std::cout << std::endl;
            }
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
            std::cout << "Program Duration: " << duration.count() << "ms" << std::endl;
        }
        else {
           std::cout << "Can't open the input file!" << std::endl;
        }

    }

    return 0;
}
