/** @file
    @brief Header

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
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

#ifndef INCLUDED_ReturnValue_h_GUID_B28A8BE9_541A_42AC_6925_C51C944B089E
#define INCLUDED_ReturnValue_h_GUID_B28A8BE9_541A_42AC_6925_C51C944B089E

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
// - none

namespace osvr {
namespace vive {
    struct ReturnValueBase {
        ReturnValueBase() {}
        ReturnValueBase(bool ok) : valid(ok) {}

        const bool valid = false;

        /// check validity by evaluating as a bool
        explicit operator bool() const { return valid; }
    };

    template <typename ValueType, typename ErrorCodeType>
    struct ReturnValue : ReturnValueBase {
        ReturnValue(ValueType const &val, ErrorCodeType err)
            : ReturnValueBase(), value(val), errorCode(err) {}
        ReturnValue(ValueType const &val, ErrorCodeType err, bool ok)
            : ReturnValueBase(ok), value(val), errorCode(err) {}

        ValueType value = {};
        ErrorCodeType errorCode = {};

        static ReturnValue makeError(ErrorCodeType err) {
            return ReturnValue{{}, err};
        }
        static ReturnValue makeValueFromRvalue(ValueType &&val) {
            return ReturnValue{std::move(val), {}, true};
        }
        static ReturnValue makeValue(ValueType val) {
            return ReturnValue{val, {}, true};
        }
    };

    /// specialization for bool "error codes"
    template <typename ValueType>
    struct ReturnValue<ValueType, bool> : ReturnValueBase {
        ReturnValue(ValueType const &val) : ReturnValueBase(true), value(val) {}
        ReturnValue(ValueType const &val, bool ok)
            : ReturnValueBase(ok), value(val) {}

        ValueType value = {};

        static ReturnValue makeError() { return ReturnValue{{}, false}; }
        static ReturnValue makeValueFromRvalue(ValueType &&val) {
            return ReturnValue{std::move(val), true};
        }
        static ReturnValue makeValue(ValueType val) {
            return ReturnValue{val, true};
        }
    };

    template <typename ValueType, typename ErrorCodeType>
    inline ReturnValue<ValueType, ErrorCodeType>
    makeGenericReturnValue(ValueType const &val, ErrorCodeType err,
                           bool valid) {
        using type = ReturnValue<ValueType, ErrorCodeType>;
        return type(val, err, valid);
    }

    template <typename ValueType, typename ErrorCodeType>
    inline ReturnValue<ValueType, ErrorCodeType> makeError(ValueType const &val,
                                                           ErrorCodeType err) {
        return makeGenericReturnValue(val, err, false);
    }

    template <typename ValueType, typename ErrorCodeType>
    inline ReturnValue<ValueType, ErrorCodeType>
    makeReturnValue(ValueType const &val, ErrorCodeType err) {
        return makeGenericReturnValue(val, err, true);
    }

} // namespace vive
} // namespace osvr
#endif // INCLUDED_ReturnValue_h_GUID_B28A8BE9_541A_42AC_6925_C51C944B089E
