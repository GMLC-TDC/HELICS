/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#define CLI11_EXPERIMENTAL_OPTIONAL 0
#include "helics/external/CLI11/CLI11.hpp"
#undef CLI11_EXPERIMENTAL_OPTIONAL
#include "core-types.hpp"
#include "helics-time.hpp"
#include "helicsVersion.hpp"

namespace helics
{
class helicsCLI11App : public CLI::App
{
  public:
    explicit helicsCLI11App (std::string app_description = "", const std::string &app_name = "")
        : CLI::App (std::move (app_description), app_name, nullptr)
    {
        set_help_flag ("-h,-?,--help", "Print this help message and exit");
        set_config ("--config-file", "helics_config.ini", "specify base configuration file");
        add_flag_callback ("--version,-v", [] () { throw (CLI::Success{}); });
        add_option_group ("quiet")->immediate_callback ()->add_flag ("--quiet", quiet,
                                                                     "silence most print output");
    }

    enum class parse_return : int
    {
        ok = 0,
        help_return,
        help_all_return,
        version_return,
        error_return,
    };
    bool quiet{false};
    parse_return last_return{parse_return::ok};

    template <typename... Args>
    parse_return helics_parse (Args &&... args)
    {
        try
        {
            parse (std::forward<Args> (args)...);
            last_return = parse_return::ok;
            remArgs = remaining_for_passthrough ();
            return parse_return::ok;
        }
        catch (const CLI::CallForHelp &ch)
        {
            if (!quiet)
            {
                exit (ch);
            }
            last_return = parse_return::help_return;
            return parse_return::help_return;
        }
        catch (const CLI::CallForAllHelp &ca)
        {
            if (!quiet)
            {
                exit (ca);
            }
            last_return = parse_return::help_all_return;
            return parse_return::help_all_return;
        }
        catch (const CLI::Success &)
        {
            if (!quiet)
            {
                std::cout << helics::versionString << '\n';
            }
            last_return = parse_return::version_return;
            return parse_return::version_return;
        }
        catch (const CLI::Error &ce)
        {
            exit (ce);
            last_return = parse_return::error_return;
            return parse_return::error_return;
        }
    }
    std::vector<std::string> &remainArgs () { return remArgs; }
    void remove_helics_specifics ()
    {
        set_help_flag ();
        set_config ();
        try
        {
            remove_option (get_option_no_throw ("-v"));
            remove_subcommand (get_option_group ("quiet"));
        }
        catch (const CLI::Error &)
        {
            // must have been removed earlier
        }
    }
    /** Add a callback function to execute on parsing*/
    void add_callback (std::function<void ()> cback)
    {
        if (cbacks.empty ())
        {
            callback ([this] () {
                for (auto &cb : cbacks)
                {
                    cb ();
                }
            });
        }
        cbacks.push_back (std::move (cback));
    }

    void addTypeOption ()
    {
        auto og = add_option_group ("network type")->immediate_callback ();
        og->add_option_function<std::string> (
            "--coretype,-t,--type,--core",
            [this] (const std::string &val) {
                coreType = helics::coreTypeFromString (val);
                if (coreType == core_type::UNRECOGNIZED)
                    throw CLI::ValidationError (val + " is NOT a recognized core type");
            },
            "type of the core to connect to")
          ->default_str ("(" + helics::to_string (coreType) + ")");
    }
    core_type getCoreType () const { return coreType; }
    /** set default core core type*/
    void setDefaultCoreType (core_type type) { coreType = type; }

  private:
    std::vector<std::function<void ()>> cbacks;
    std::vector<std::string> remArgs;
    core_type coreType{core_type::DEFAULT};
};
}  // namespace helics

// use the CLI lexical cast function overload to convert a string into a time
namespace CLI
{
namespace detail
{
template <>
inline bool lexical_cast<helics::Time> (std::string input, helics::Time &output)
{
    try
    {
        output = helics::loadTimeFromString (input, time_units::ms);
    }
    catch (std::invalid_argument &)
    {
        return false;
    }
    return true;
}

template <>
constexpr const char *type_name<helics::Time> ()
{
    return "TIME";
}
}  // namespace detail
}  // namespace CLI
