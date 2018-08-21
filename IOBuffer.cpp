/** @file
    @brief Implementation

    @date 2018

    @author
    Georgiy Frolov
    <http://github.com/mars979>
*/

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
#include "IOBuffer.h"

// Library/third-party includes
// - none

// Standard includes
// - none

using namespace vr;

IOBuffer::IOBuffer() : m_logger(osvr::util::log::make_logger("IOBuffer")) {}

vr::EIOBufferError IOBuffer::Open(const char *pchPath, vr::EIOBufferMode mode,
                                  uint32_t unElementSize, uint32_t unElements,
                                  vr::IOBufferHandle_t * /*pulBuffer*/) {

    m_logger->info("Open(") << pchPath << ", " << mode << ", " << unElementSize
                            << ", " << unElements << ")";

    return IOBuffer_OperationFailed;
}

vr::EIOBufferError IOBuffer::Close(vr::IOBufferHandle_t ulBuffer) {
    m_logger->info("Close(") << ulBuffer << ")";
    return IOBuffer_OperationFailed;
}

vr::EIOBufferError IOBuffer::Read(vr::IOBufferHandle_t ulBuffer,
                                  void * /*pDst*/, uint32_t unBytes,
                                  uint32_t * /*punRead*/) {
    m_logger->info("Read(") << ulBuffer << "," << unBytes << ")";
    return IOBuffer_OperationFailed;
}

vr::EIOBufferError IOBuffer::Write(vr::IOBufferHandle_t ulBuffer,
                                   void * /*pSrc*/, uint32_t unBytes) {
    m_logger->info("Write(") << ulBuffer << "," << unBytes << ")";
    return IOBuffer_OperationFailed;
}

vr::PropertyContainerHandle_t
IOBuffer::PropertyContainer(vr::IOBufferHandle_t ulBuffer) {
    m_logger->info("PropertyContainer(") << ulBuffer << ")";
    return 0;
}
