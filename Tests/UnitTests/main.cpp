// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <string_view>

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Cards/Cards.hpp>

using namespace RosettaStone;

int main(int argc, char** argv)
{
    doctest::Context context(argc, argv);

    PlayMode::Cards::GetInstance();

    bool hasTestCaseFilter = false;
    bool includesBattlegrounds = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument(argv[i]);
        std::string_view filter;
        if (argument == "--test-case" || argument == "-tc")
        {
            if (i + 1 < argc)
            {
                filter = argv[++i];
            }
        }
        else if (argument.starts_with("--test-case=") ||
                 argument.starts_with("-tc="))
        {
            filter = argument.substr(argument.find('=') + 1);
        }

        if (!filter.empty())
        {
            hasTestCaseFilter = true;
            includesBattlegrounds = includesBattlegrounds ||
                                    filter == "*" ||
                                    filter.find("[Battlegrounds") !=
                                        std::string_view::npos;
        }
    }

    // Focused PlayMode tests should not initialize the large Battlegrounds
    // database. Keep the old all-database behavior for full runs and BG tests.
    if (!hasTestCaseFilter || includesBattlegrounds)
    {
        Battlegrounds::Cards::GetInstance();
    }

    // Run queries, or run tests unless --no-run is specified
    const int res = context.run();

    return res;
}
