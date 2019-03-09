/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "CLI11/CLI11.hpp"
#include "helics-time.hpp"
#include "helicsVersion.hpp"

namespace helics
{
class helicsCLI11App : public CLI::App
{
  public:
    explicit helicsCLI11App (std::string app_description = "", std::string app_name = "")
        : CLI::App (app_description, app_name, nullptr)
    {
        set_help_flag ("-h,-?,--help", "Print this help message and exit");
        set_config ("--config-file", "helics_config.ini", "specify base configuration file");
        add_flag_callback ("--version,-v", [] () { throw (CLI::Success{}); });
        add_option_group ("quiet")->immediate_callback ()->add_flag ("--quiet", quiet,
                                                                     "silence most print output");
    }
    bool quiet{false};

    enum class parse_return
    {
        ok = 0,
        help_return,
        help_all_return,
        version_return,
        error_return,
    };
    template <typename... Args>
    parse_return helics_parse (Args &&... args)
    {
        try
        {
            parse (std::forward<Args> (args...));
            return parse_return::ok;
        }
        catch (const CLI::CallForHelp &ch)
        {
            if (!quiet)
            {
                exit (ch);
            }
            return parse_return::help_return;
        }
        catch (const CLI::CallForAllHelp &ca)
        {
            if (!quiet)
            {
                exit (ch);
            }
            return parse_return::help_all_return;
        }
        catch (const CLI::Success &)
        {
            if (!quiet)
            {
                std::cout << helics::versionString << '\n';
            }
            return parse_return::version_return;
        }
        catch (const CLI::Error &ce)
        {
            exit (ce);
            return parse_return::error_return;
        }
    }
};
}  // namespace helics

// use the CLI lexical cast function overload to convert a string into a time
namespace CLI
{
namespace detail
{
template <>
bool lexical_cast<helics::Time> (std::string input, helics::Time &output)
{
    try
    {
        output = helics::loadTimeFromString (input);
    }
    catch (std::invalid_argument &)
    {
        return false;
    }
    return true;
}
}  // namespace detail
}  // namespace CLI
