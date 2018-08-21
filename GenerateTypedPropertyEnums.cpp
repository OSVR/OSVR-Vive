/** @file
    @brief Implementation

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Razer, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
// - none

// Library/third-party includes
#include <boost/range/adaptor/reversed.hpp>
#include <json/reader.h>
#include <json/value.h>
#include <osvr/Util/UniqueContainer.h>

// Standard includes
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

static const auto indent = "    ";

static const auto outFilename = "PropertyTraits.h";
static const auto startOfOutput = R"(/** @file
    @brief Header - partially generated from parsing openvr_api.json

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

/*
Copyright 2016 Razer Inc.

SPDX-License-Identifier: BSD-3-Clause

OpenVR input data:
Copyright (c) 2015, Valve Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INCLUDED_PropertyTraits_h_GUID_6CC473E5_C8B9_46B7_237B_1E0C08E91076
#define INCLUDED_PropertyTraits_h_GUID_6CC473E5_C8B9_46B7_237B_1E0C08E91076

#ifndef _INCLUDE_VRTYPES_H
#error "Please include exactly one of openvr.h or openvr_driver.h before including this file"
#endif

#include <cstddef>
#include <string>

namespace osvr {
namespace vive {
)";

static const auto endOfOutput = R"(

} // namespace vive
} // namespace osvr

#endif // INCLUDED_PropertyTraits_h_GUID_6CC473E5_C8B9_46B7_237B_1E0C08E91076
)";

inline bool shouldIgnoreType(std::string const &t) {
    return t == "Start" || t == "End" ||
           t == "Max"; // ignore the sentinels that look like types.
}

std::vector<std::pair<std::string, std::string>> g_fullNameToTypeSuffix;
std::vector<std::pair<std::string, std::string>> g_cleanNameToFullName;

std::map<std::string, std::string> g_typeSuffixToTypename = {
    {"String", "std::string"},
    {"Bool", "bool"},
    {"Float", "float"},
    {"Matrix34", "vr::HmdMatrix34_t"},
    {"Matrix34_Array", "vr::HmdMatrix34_t *"},
    {"Vector3", "vr::HmdVector3d_t"},
    {"Uint64", "uint64_t"},
    {"Int32", "int32_t"},
    {"Binary", "void *"}};

std::set<std::string> g_ambiguousNames;

osvr::util::UniqueContainer<std::vector<std::string>> g_typeSuffixes;
osvr::util::UniqueContainer<std::vector<std::string>> g_cleanNames;

inline std::string getTypenameForTypeSuffix(std::string const &suffix) {
    auto it = g_typeSuffixToTypename.find(suffix);
    if (end(g_typeSuffixToTypename) == it) {
        throw std::logic_error("Missing mapping for suffix " + suffix);
    }
    return it->second;
}

/// Structure that decomposes a full name of an enum value into a clean name and
/// a type suffix.
struct NameDecomp {
    explicit NameDecomp(std::string enumName) : name(std::move(enumName)) {

        // Before the regex, search for known type suffixes.
        for (auto const &pair : g_typeSuffixToTypename) {
            auto len = pair.first.length();
            if (len >= name.length()) {
                continue;
            }
            auto pos = name.length() - len;
            if (name.substr(pos) == pair.first) {
                // OK, we found this type
                typeSuffix = pair.first;
                static const auto partDecomposeRegex =
                    std::regex{"^Prop_(.*)_$"};
                std::smatch m;
                auto typelessName = name.substr(0, pos);
                std::regex_search(typelessName, m, partDecomposeRegex);
                cleanName = m[1];
                return;
            }
        }

        // In the absence of a known type suffix, do the best we can with
        // decomposing by regex. Will just result in a warning anyway.
        static const auto fullDecomposeRegex =
            std::regex{"^Prop_(.*)_([^_]*)$"};
        std::smatch m;
        std::regex_search(name, m, fullDecomposeRegex);
        cleanName = m[1];
        typeSuffix = m[2];
    }
    const std::string name;
    std::string cleanName;
    std::string typeSuffix;
};

bool processEnumValues(Json::Value const &values, std::ostream &output) {
    std::vector<NameDecomp> names;
    for (auto &enumVal : values) {
        auto name = enumVal["name"].asString();

        auto d = NameDecomp{name};
        if (shouldIgnoreType(d.typeSuffix) || (d.typeSuffix.length() == 0)) {
            continue;
        }
        names.push_back(d);
#if 0
        std::cout << "Name: " << name << " Clean name: " << d.cleanName << " Value type: " << d.typeSuffix << std::endl;
#endif
        if (g_cleanNames.contains(d.cleanName)) {
            std::cerr << "Ambiguous clean name found! " << d.cleanName
                      << " (second type was " << d.typeSuffix << ")"
                      << std::endl;
            g_ambiguousNames.insert(d.cleanName);
        } else {
            g_cleanNames.insert(d.cleanName);
        }
        g_typeSuffixes.insert(d.typeSuffix);
        g_fullNameToTypeSuffix.emplace_back(name, d.typeSuffix);
    }

    bool success = true;

    /// OK, so that was the first pass through the list. Quick safety check.
    for (auto &suffix : g_typeSuffixes.container()) {
        if (end(g_typeSuffixToTypename) ==
            g_typeSuffixToTypename.find(suffix)) {
            std::cerr << "Type suffix found in the data file that's not "
                         "accounted for in the application: "
                      << suffix << std::endl;
            std::cerr << "Tool must be updated to add a mapping to "
                         "g_typeSuffixToTypename for this type suffix!"
                      << std::endl;
            success = false;
        }
    }
    if (!success) {
        return success;
    }

    /// Second pass: output the shortcut enum class for everything, qualifying
    /// the ambiguous names.
    {

        std::vector<std::string> lines;
        for (auto &d : names) {
            std::ostringstream os;
            auto isAmbiguous =
                (end(g_ambiguousNames) != g_ambiguousNames.find(d.cleanName));
            if (isAmbiguous) {
                // Qualify ambiguous enums with a type suffix
                os << d.cleanName << "_" << d.typeSuffix << " = vr::" << d.name
                   << ",";
            } else {
                os << d.cleanName << " = vr::" << d.name << ",";
            }
            lines.emplace_back(os.str());
        }

        output << "enum class Props {" << std::endl;
        for (auto &line : lines) {
            output << indent << line << std::endl;
        }
        output << "};" << std::endl;
    }

    /// Third pass: output the traits class specializations to associate types
    /// with each property enum.
    output << "namespace detail {" << std::endl;
    output << indent
           << "template<std::size_t EnumVal> struct PropertyTypeTrait;"
           << std::endl;
    output << indent
           << "template<std::size_t EnumVal> using PropertyType = "
              "typename PropertyTypeTrait<EnumVal>::type;"
           << std::endl;
    for (auto &d : names) {
        auto enumTypename = getTypenameForTypeSuffix(d.typeSuffix);
        output << indent << "template<> struct PropertyTypeTrait<vr::" << d.name
               << "> { using type = " << enumTypename << "; };" << std::endl;
    }
    output << "} // namespace detail" << std::endl;

    return success;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Must pass path to openvr_api.json as first argument!"
                  << std::endl;
        return -1;
    }
    Json::Value root;
    {
        std::ifstream is(argv[1]);
        if (!is) {
            std::cerr << "Could not open expected 'openvr_api.json' file at "
                         "provided path: "
                      << argv[1] << std::endl;
            return -1;
        }
        Json::Reader reader;
        if (!reader.parse(is, root)) {
            std::cerr << "Could not parse " << argv[1]
                      << " as JSON: " << reader.getFormattedErrorMessages()
                      << std::endl;
            return -1;
        }
    }

    std::ostringstream os;
    auto &enums = root["enums"];
    bool success = false;
    for (auto const &enumObj : enums) {
        if (enumObj["enumname"] == "vr::ETrackedDeviceProperty") {
            success = processEnumValues(enumObj["values"], os);
            break;
        }
    }

    if (success) {
        std::cout << "Succeeded in processing JSON: will now write file "
                  << outFilename << std::endl;
        std::ofstream of(outFilename);
        if (!of) {
            std::cerr << "Could not open file " << outFilename << std::endl;
            return -1;
        }
        of << startOfOutput;
        of << os.str();
        of << endOfOutput;
        of.close();

        std::cout << "Done writing file!" << std::endl;
    }

    return success ? 0 : 1;
}
