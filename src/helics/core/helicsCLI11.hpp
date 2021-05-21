/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#define CLI11_EXPERIMENTAL_OPTIONAL 0
#include "helics/external/CLI11/CLI11.hpp"

#include <string>
#include <utility>
#include <vector>
#undef CLI11_EXPERIMENTAL_OPTIONAL
#include "core-types.hpp"
#include "helics-time.hpp"
#if defined HELICS_SHARED_LIBRARY || !defined HELICS_STATIC_CORE_LIBRARY
#    include "../application_api/timeOperations.hpp"
#    include "../application_api/typeOperations.hpp"

using helics::coreTypeFromString;
using helics::loadTimeFromString;
using helics::to_string;
#else
#    include "../utilities/timeStringOps.hpp"
#    include "coreTypeOperations.hpp"

using helics::core::coreTypeFromString;
using helics::core::to_string;
/** generate a local function that uses the utilities library*/
inline helics::Time loadTimeFromString(const std::string& str, time_units defUnit)
{
    return gmlc::utilities::loadTimeFromString<helics::Time>(str, defUnit);
}

#endif
#include "helicsVersion.hpp"
namespace helics {
class helicsCLI11App: public CLI::App {
  public:
    explicit helicsCLI11App(std::string app_description = "", const std::string& app_name = ""):
        CLI::App(std::move(app_description), app_name, nullptr)
    {
        set_help_flag("-h,-?,--help", "Print this help message and exit");
        set_config("--config-file,--config",
                   "helics_config.toml",
                   "specify base configuration file");
        set_version_flag("--version", helics::versionString);
        add_option_group("quiet")->immediate_callback()->add_flag("--quiet",
                                                                  quiet,
                                                                  "silence most print output");
    }

    enum class parse_output : int {
        ok = 0,
        help_call = 1,
        help_all_call = 2,
        version_call = 4,
        parse_error = -4,
    };
    bool quiet{false};
    bool passConfig{true};
    parse_output last_output{parse_output::ok};

    template<typename... Args>
    parse_output helics_parse(Args&&... args) noexcept
    {
        try {
            parse(std::forward<Args>(args)...);
            last_output = parse_output::ok;
            remArgs = remaining_for_passthrough();
            if (passConfig) {
                auto* opt = get_option_no_throw("--config");
                if (opt != nullptr && opt->count() > 0) {
                    remArgs.push_back(opt->as<std::string>());
                    remArgs.emplace_back("--config");
                }
            }

            return parse_output::ok;
        }
        catch (const CLI::CallForHelp& ch) {
            if (!quiet) {
                exit(ch);
            }
            last_output = parse_output::help_call;
            return parse_output::help_call;
        }
        catch (const CLI::CallForAllHelp& ca) {
            if (!quiet) {
                exit(ca);
            }
            last_output = parse_output::help_all_call;
            return parse_output::help_all_call;
        }
        catch (const CLI::CallForVersion& cv) {
            if (!quiet) {
                exit(cv);
            }
            last_output = parse_output::version_call;
            return parse_output::version_call;
        }
        catch (const CLI::Error& ce) {
            CLI::App::exit(ce);
            last_output = parse_output::parse_error;
            return parse_output::parse_error;
        }
        catch (...) {
            last_output = parse_output::parse_error;
            return parse_output::parse_error;
        }
    }
    std::vector<std::string>& remainArgs() { return remArgs; }
    void remove_helics_specifics()
    {
        set_help_flag();
        set_config();
        try {
            remove_option(get_option_no_throw("-v"));
            remove_subcommand(get_option_group("quiet"));
        }
        catch (const CLI::Error&) {
            // must have been removed earlier
        }
    }
    /** Add a callback function to execute on parsing*/
    void add_callback(std::function<void()> cback)
    {
        if (cbacks.empty()) {
            callback([this]() {
                for (auto& cb : cbacks) {
                    cb();
                }
            });
        }
        cbacks.push_back(std::move(cback));
    }

    void addTypeOption(bool includeEnvironmentVariable = true)
    {
        auto* og = add_option_group("network type")->immediate_callback();
        auto* typeOption =
            og->add_option_function<std::string>(
                  "--coretype,-t,--type,--core",
                  [this](const std::string& val) {
                      coreType = coreTypeFromString(val);
                      if (coreType == core_type::UNRECOGNIZED) {
                          throw CLI::ValidationError(val + " is NOT a recognized core type");
                      }
                  },
                  "type of the core to connect to")
                ->default_str("(" + to_string(coreType) + ")");
        if (includeEnvironmentVariable) {
            typeOption->envname("HELICS_CORE_TYPE");
        }
    }
    core_type getCoreType() const { return coreType; }
    /** set default core type*/
    void setDefaultCoreType(core_type type) { coreType = type; }

  private:
    std::vector<std::function<void()>> cbacks;
    std::vector<std::string> remArgs;
    core_type coreType{core_type::DEFAULT};
};
}  // namespace helics

// use the CLI lexical cast function overload to convert a string into a time
namespace CLI {
namespace detail {
    template<>
    inline bool lexical_cast<helics::Time>(const std::string& input, helics::Time& output)
    {
        try {
            output = loadTimeFromString(input, time_units::ms);
        }
        catch (std::invalid_argument&) {
            return false;
        }
        return true;
    }

    template<>
    constexpr const char* type_name<helics::Time>()
    {
        return "TIME";
    }
}  // namespace detail
}  // namespace CLI
