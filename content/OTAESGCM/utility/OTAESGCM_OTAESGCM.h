/*
The OpenTRV project licenses this file to you
under the Apache Licence, Version 2.0 (the "Licence");
you may not use this file except in compliance
with the Licence. You may obtain a copy of the Licence at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the Licence is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the Licence for the
specific language governing permissions and limitations
under the Licence.

Author(s) / Copyright (s): Deniz Erbilgin 2015
                           Damon Hart-Davis 2015
*/

/* OpenTRV OTAESGCM microcontroller-/IoT- friendly AES(128)-GCM implementation. */

#ifndef ARDUINO_LIB_OTAESGCM_OTAESGCM_H
#define ARDUINO_LIB_OTAESGCM_OTAESGCM_H

#include <stddef.h>
#include <stdint.h>

// Get available AES API and cipher implementations.
#include "OTAESGCM_OTAES128.h"
#include "OTAESGCM_OTAES128Impls.h"


// Use namespaces to help avoid collisions.
namespace OTAESGCM
    {


static constexpr uint8_t AES128GCM_BLOCK_SIZE = 16; // GCM block size in bytes. This must be the same as the AES block size.
static constexpr uint8_t AES128GCM_IV_SIZE    = 12; // GCM initialisation size in bytes.
static constexpr uint8_t AES128GCM_TAG_SIZE   = 16; // GCM authentication tag size in bytes.


    // Base class / interface for AES128-GCM encryption/decryption.
    // Neither re-entrant nor ISR-safe except where stated.
    class OTAES128GCM
        {
        protected:
            // Only derived classes can construct an instance.
            constexpr OTAES128GCM() { }

        public:
            /**
             * @brief   performs AES-GCM encryption.
             * 			If ADATA unused, set ADATA to NULL and ADATALength to 0.
             * 			If PDATA unused (This is GMAC), set PDATA and CDATA to NULL and PDATALength to 0.
             * @todo	Make GMAC helper function.
             * @param   key		pointer to 16 byte (128 bit) key; never NULL
             * @param   IV             	pointer to 12 byte (96 bit) IV; never NULL
             * @param   PDATA          	pointer to plaintext array, this is internally padded up to a multiple of the blocksize; NULL if length 0.
             * @param   PDATALength	length of plaintext array in bytes, can be zero
             * @param   ADATA           pointer to additional data array; NULL if length 0.
             * @param   ADATALength    	length of additional data in bytes, can be zero
             * @param   CDATA           buffer to output ciphertext to, size MUST BE PADDED/EXPANDED TO FULL BLOCKSIZE MULTIPLE at/above PDATAlength;
             *                          (nominally set to NULL if PDATA is NULL but seems to cause a crash)
             * @param   tag             pointer to 16 byte buffer to output tag to; never NULL
             * @retval	true if encryption is successful, else false
             */
            virtual bool gcmEncrypt(
                const uint8_t* key, const uint8_t* IV,
                const uint8_t* PDATA, uint8_t PDATALength,
                const uint8_t* ADATA, uint8_t ADATALength,
                uint8_t* CDATA, uint8_t *tag) const = 0;

            /**
             * @brief   performs AES-GCM decryption and authentication
             * @param    key             pointer to 16 byte (128 bit) key
             * @param    IV              pointer to 12 byte (96 bit) IV
             * @param    CDATA           pointer to ciphertext array
             * @param    CDATALength     length of ciphertext array
             * @param    ADATA           pointer to additional data array
             * @param    ADATALength     length of additional data
             * @param    PDATA           buffer to output plaintext to; must be same length as CDATA
             * @retval   true if decryption and authentication successful, else false
             */
            virtual bool gcmDecrypt(
                 const uint8_t* key, const uint8_t* IV,
                 const uint8_t* CDATA, uint8_t CDATALength,
                 const uint8_t* ADATA, uint8_t ADATALength,
                 const uint8_t* messageTag, uint8_t *PDATA) const = 0;

#if 0 // Defining the virtual destructor uses ~800+ bytes of Flash by forcing use of malloc()/free().
            // Ensure safe instance destruction when derived from.
            // by default attempts to shut down the sensor and otherwise free resources when done.
            // This uses ~800+ bytes of Flash by forcing use of malloc()/free().
            virtual ~OTAES128GCM() { }
#else
#define OTAES128GCM_NO_VIRT_DEST // Beware, no virtual destructor so be careful of use via base pointers.
#endif
        };

    // Generic implementation, parameterised with type of underlying AES implementation.
    // The default AES implementation for the architecture is used unless otherwise specified.
    // This implementation is not specialised for a particular CPU/MCU for example.
    // This implementation carries no state beyond that of the AES128 implementation.
    class OTAES128GCMGenericBase : public OTAES128GCM
        {
        private:
            // Pointer to an AES block encryption implementation instance; never NULL.
            OTAES128E * const ap;
        public:
            // Create an instance pointing at a suitable AES block enc/dec implementation.
            // The AES impl should not carry logical state between operations,
            // but may hold temporary workspace or non-key/data-dependent state.
            constexpr OTAES128GCMGenericBase(OTAES128E *aptr) : ap(aptr) { }
            // Encrypt; true iff successful.
            virtual bool gcmEncrypt(
                const uint8_t* key, const uint8_t* IV,
                const uint8_t* PDATA, uint8_t PDATALength,
                const uint8_t* ADATA, uint8_t ADATALength,
                uint8_t* CDATA, uint8_t *tag) const override;
            // Decrypt; true iff successful.
            virtual bool gcmDecrypt(
                 const uint8_t* key, const uint8_t* IV,
                 const uint8_t* CDATA, uint8_t CDATALength,
                 const uint8_t* ADATA, uint8_t ADATALength,
                 const uint8_t* messageTag, uint8_t *PDATA) const override;
        };

    // Generic implementation, parameterised with type of underlying AES implementation.
    // Carries the AES working state with it.
    // The OTAESImpl should clear up private state before returning from its methods.
    template<class OTAESImpl = OTAESGCM::OTAES128E_default_t>
    class OTAES128GCMGeneric final : OTAESImpl, public OTAES128GCMGenericBase
        {
        private:
            // Minimum size of workspace required.
            constexpr static uint8_t workspaceRequired = OTAESImpl::workspaceRequired;
            uint8_t workspace[workspaceRequired];
        public:
            // Construct an instance.
            constexpr OTAES128GCMGeneric() : OTAESImpl(workspace, workspaceRequired), OTAES128GCMGenericBase(this) { }
        };

    // Generic implementation, parameterised with type of underlying AES implementation.
    // Carries the AES working state with it.
    // The OTAESImpl should clear up private state before returning from its methods.
    template<class OTAESImpl = OTAESGCM::OTAES128E_default_t>
    class OTAES128GCMGenericWithWorkspace final : OTAESImpl, public OTAES128GCMGenericBase
        {
        public:
            // Minimum size of workspace required.
            constexpr static uint8_t workspaceRequired = OTAESImpl::workspaceRequired;
            // Construct an instance, supplied with workspace.
            constexpr OTAES128GCMGenericWithWorkspace(uint8_t *const workspace, const uint8_t workspaceSize)
                : OTAESImpl(workspace, workspaceSize), OTAES128GCMGenericBase(this)
                { }
            // Verify that the workspace would be adequate before constructing an instance.
            static constexpr bool isWorkspaceSufficient(uint8_t *const workspace, const uint8_t workspaceSize)
                { return((NULL != workspace) && (workspaceSize >= workspaceRequired)); }
        };


    // AES-GCM 128-bit-key fixed-size text (256-bit/32-byte) encryption/authentication function.
    // This is an adaptor/bridge function to ease outside use in simple cases
    // without explicit type/library dependencies, but use with care.
    // Stateless implementation: creates state on stack each time at cost of stack space
    // (which may be considerable and difficult to manage in an embedded system)
    // and at cost of time.
    // The state parameter is not used (is ignored) and should be NULL.
    // Other than the authtext, all sizes are fixed:
    //   * textSize is 32 (or zero if plaintext is NULL)
    //   * keySize is 16
    //   * nonceSize is 12
    //   * tagSize is 16
    // The plain-text (and identical cipher-text) size is picked to be
    // a multiple of the cipher's block size, or zero,
    // which implies likely requirement for padding of the plain text.
    // Note that the authenticated text size is not fixed, ie is zero or more bytes.
    // Returns true on success, false on failure.
    bool fixed32BTextSize12BNonce16BTagSimpleEnc_DEFAULT_STATELESS(void *,
            const uint8_t *key, const uint8_t *iv,
            const uint8_t *authtext, uint8_t authtextSize,
            const uint8_t *plaintext,
            uint8_t *ciphertextOut, uint8_t *tagOut);

    // AES-GCM 128-bit-key fixed-size text (256-bit/32-byte) decryption/authentication function.
    // This is an adaptor/bridge function to ease outside use in simple cases
    // without explicit type/library dependencies, but use with care.
    // Stateless implementation: creates state on stack each time at cost of stack space
    // (which may be considerable and difficult to manage in an embedded system)
    // and at cost of time.
    // The state parameter is not used (is ignored) and should be NULL.
    // Other than the authtext, all sizes are fixed:
    //   * textSize is 32 (or zero if ciphertext is NULL)
    //   * keySize is 16
    //   * nonceSize is 12
    //   * tagSize is 16
    // The plain-text (and identical cipher-text) size is picked to be
    // a multiple of the cipher's block size, or zero,
    // which implies likely requirement for padding of the plain text.
    // Note that the authenticated text size is not fixed, ie is zero or more bytes.
    // Decrypts/authenticates the output of fixed32BTextSize12BNonce16BTagSimpleEnc_DEFAULT_STATELESS.)
    // Returns true on success, false on failure.
    bool fixed32BTextSize12BNonce16BTagSimpleDec_DEFAULT_STATELESS(void *state,
            const uint8_t *key, const uint8_t *iv,
            const uint8_t *authtext, uint8_t authtextSize,
            const uint8_t *ciphertext, const uint8_t *tag,
            uint8_t *plaintextOut);


    // AES-GCM 128-bit-key fixed-size text (256-bit/32-byte) encryption/authentication function using work space passed in.
    // This is an adaptor/bridge function to ease outside use in simple cases
    // without explicit type/library dependencies, but use with care.
    // A workspace is passed in (and cleared on exit);
    // this routine will fail (safely, returning false) if the workspace is NULL or too small.
    // The workspace requirement depends on the implementation used.
    // Other than the authtext, all sizes are fixed:
    //   * textSize is 32 (or zero if plaintext is NULL)
    //   * keySize is 16
    //   * nonceSize is 12
    //   * tagSize is 16
    // The plain-text (and identical cipher-text) size is picked to be
    // a multiple of the cipher's block size, or zero,
    // which implies likely requirement for padding of the plain text.
    // Note that the authenticated text size is not fixed, ie is zero or more bytes.
    // Returns true on success, false on failure.
    bool fixed32BTextSize12BNonce16BTagSimpleEnc_DEFAULT_WITH_WORKSPACE(
            uint8_t *workspace, uint8_t workspaceSize,
            const uint8_t *key, const uint8_t *iv,
            const uint8_t *authtext, uint8_t authtextSize,
            const uint8_t *plaintext,
            uint8_t *ciphertextOut, uint8_t *tagOut);

    // AES-GCM 128-bit-key fixed-size text (256-bit/32-byte) decryption/authentication function using work space passed in.
    // This is an adaptor/bridge function to ease outside use in simple cases
    // without explicit type/library dependencies, but use with care.
    // A workspace is passed in (and cleared on exit);
    // this routine will fail (safely, returning false) if the workspace is NULL or too small.
    // The workspace requirement depends on the implementation used.
    // Other than the authtext, all sizes are fixed:
    //   * textSize is 32 (or zero if ciphertext is NULL)
    //   * keySize is 16
    //   * nonceSize is 12
    //   * tagSize is 16
    // The plain-text (and identical cipher-text) size is picked to be
    // a multiple of the cipher's block size, or zero,
    // which implies likely requirement for padding of the plain text.
    // Note that the authenticated text size is not fixed, ie is zero or more bytes.
    // Decrypts/authenticates the output of fixed32BTextSize12BNonce16BTagSimpleEnc_DEFAULT_STATELESS.)
    // Returns true on success, false on failure.
    bool fixed32BTextSize12BNonce16BTagSimpleDec_DEFAULT_WITH_WORKSPACE(
            uint8_t *workspace, uint8_t workspaceSize,
            const uint8_t *key, const uint8_t *iv,
            const uint8_t *authtext, uint8_t authtextSize,
            const uint8_t *ciphertext, const uint8_t *tag,
            uint8_t *plaintextOut);
    }


#endif
