/** @file
    @brief Header

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

#ifndef INCLUDED_IOBuffer_h_GUID_23A85C49_7FFC_4BB8_237B_5C0727114C43
#define INCLUDED_IOBuffer_h_GUID_23A85C49_7FFC_4BB8_237B_5C0727114C43

// Internal Includes
#include <osvr/Util/Logger.h>

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes

namespace vr {

class IOBuffer : public vr::IVRIOBuffer {
  public:
    IOBuffer();

    /** opens an existing or creates a new IOBuffer of unSize bytes */
    virtual vr::EIOBufferError Open(const char *pchPath, vr::EIOBufferMode mode,
                                    uint32_t unElementSize, uint32_t unElements,
                                    vr::IOBufferHandle_t *pulBuffer);

    /** closes a previously opened or created buffer */
    virtual vr::EIOBufferError Close(vr::IOBufferHandle_t ulBuffer);

    /** reads up to unBytes from buffer into *pDst, returning number of bytes
     * read in *punRead */
    virtual vr::EIOBufferError Read(vr::IOBufferHandle_t ulBuffer, void *pDst,
                                    uint32_t unBytes, uint32_t *punRead);

    /** writes unBytes of data from *pSrc into a buffer. */
    virtual vr::EIOBufferError Write(vr::IOBufferHandle_t ulBuffer, void *pSrc,
                                     uint32_t unBytes);

    /** retrieves the property container of an buffer. */
    virtual vr::PropertyContainerHandle_t
    PropertyContainer(vr::IOBufferHandle_t ulBuffer);

  private:
    osvr::util::log::LoggerPtr m_logger;
};

} // namespace vr

#endif // INCLUDED_IOBuffer_h_GUID_23A85C49_7FFC_4BB8_237B_5C0727114C43
