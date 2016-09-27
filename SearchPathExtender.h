/** @file
    @brief Header

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Razer Inc.
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

#ifndef INCLUDED_SearchPathExtender_h_GUID_8154F6C5_C93B_45BD_6013_9CBF316CC9AF
#define INCLUDED_SearchPathExtender_h_GUID_8154F6C5_C93B_45BD_6013_9CBF316CC9AF

// Internal Includes
// - none

// Library/third-party includes
#include <osvr/Util/PlatformConfig.h>
#include <osvr/Util/WindowsVariantC.h>

// Standard includes
#include <algorithm>
#include <cstdlib>
//#include <iostream>
#include <string>

namespace osvr {
namespace vive {

#if defined(OSVR_WINDOWS)
    static const auto SEARCH_PATH_ENV = "PATH";
    static const auto SEARCH_PATH_SEP = ";";
#else
    static const auto SEARCH_PATH_ENV = "LD_LIBRARY_PATH";
    static const auto SEARCH_PATH_SEP = ":";
#endif

#if defined(OSVR_WINDOWS) && !defined(OSVR_WINDOWS_DESKTOP)
    /// Non-desktop Windows builds can't access the environment like this.
    class SearchPathExtender {
      public:
        explicit SearchPathExtender(std::string const &additionalDir) {}

        ~SearchPathExtender() {}
        SearchPathExtender(SearchPathExtender const &) = delete;
        SearchPathExtender &operator=(SearchPathExtender const &) = delete;
    };
#else

    class SearchPathExtender {
      public:
        explicit SearchPathExtender(std::string const &additionalDir) {
#ifdef _MSC_VER
            char buf[1024 * 64];
            size_t spaceRequired;
            auto ret = getenv_s(&spaceRequired, buf, SEARCH_PATH_ENV);
            if (0 == ret) {
                // OK, we got it.
                before_ = std::string{buf};
            }
#else // not microsoft runtime specific
            auto initialRet = std::getenv(SEARCH_PATH_ENV);
            if (nullptr == initialRet) {
                nullBefore_ = true;
            } else {
                before_ = initialRet;
            }
#endif
            auto newValue = additionalDir +
                            (before_.empty() ? "" : SEARCH_PATH_SEP) + before_;

#if 0
            std::cout << "Extending " << SEARCH_PATH_ENV << "\n";
            std::cout << "Before: " << before_ << "\n\n";
            std::cout << "After:  " << newValue << "\n\n" << std::endl;
#endif
            wrappedPutenv(newValue);
        }

        ~SearchPathExtender() {
            /// Restore previous search path.
            wrappedPutenv(before_);
        }
        SearchPathExtender(SearchPathExtender const &) = delete;
        SearchPathExtender &operator=(SearchPathExtender const &) = delete;

      private:
        void wrappedPutenv(std::string const &val) {
#ifdef _MSC_VER
            _putenv_s(SEARCH_PATH_ENV, val.c_str());
#else // not microsoft runtime specific
            auto newValue = SEARCH_PATH_ENV + std::string("=") + val;
            // Have to allocate new string because it becomes part of the
            // environment.
            char *newString = static_cast<char *>(malloc(newValue.size() + 1));
            std::copy(begin(newValue), end(newValue), newString);
#endif
        }
        bool nullBefore_ = false;
        std::string before_;
    };
#endif

} // namespace vive
} // namespace osvr

#endif // INCLUDED_SearchPathExtender_h_GUID_8154F6C5_C93B_45BD_6013_9CBF316CC9AF
