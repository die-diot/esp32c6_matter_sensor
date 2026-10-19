#include "diot_commissioning.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>
#include <setup_payload/SetupPayload.h>

namespace {

constexpr uint16_t kSetupDiscriminator = 3840;
#ifdef CONFIG_DIOT_MATTER_SETUP_PIN_CODE
constexpr uint32_t kSetupPinCode = CONFIG_DIOT_MATTER_SETUP_PIN_CODE;
#else
constexpr uint32_t kSetupPinCode = 20202021;
#endif
#ifdef CONFIG_DIOT_MATTER_BLE_DEVICE_NAME
constexpr char kBleDeviceName[] = CONFIG_DIOT_MATTER_BLE_DEVICE_NAME;
#else
constexpr char kBleDeviceName[] = "DIOT-C6";
#endif

static_assert(sizeof(kBleDeviceName) - 1 <= 31, "BLE device name must fit CHIP max length");

class fixed_commissionable_data_provider : public chip::DeviceLayer::CommissionableDataProvider {
public:
    CHIP_ERROR GetSetupDiscriminator(uint16_t &setupDiscriminator) override
    {
        setupDiscriminator = kSetupDiscriminator;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override
    {
        (void)setupDiscriminator;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetSpake2pIterationCount(uint32_t &iterationCount) override
    {
        iterationCount = kIterationCount;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pSalt(chip::MutableByteSpan &saltBuf) override
    {
        VerifyOrReturnError(saltBuf.size() >= sizeof(kSalt), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(saltBuf.data(), kSalt, sizeof(kSalt));
        saltBuf.reduce_size(sizeof(kSalt));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pVerifier(chip::MutableByteSpan &verifierBuf, size_t &outVerifierLen) override
    {
        uint32_t setupPasscode = 0;
        uint32_t iterationCount = 0;
        uint8_t saltStorage[sizeof(kSalt)] = {0};
        chip::MutableByteSpan saltSpan(saltStorage);

        ReturnErrorOnFailure(GetSetupPasscode(setupPasscode));
        ReturnErrorOnFailure(GetSpake2pIterationCount(iterationCount));
        ReturnErrorOnFailure(GetSpake2pSalt(saltSpan));

        chip::Crypto::Spake2pVerifier verifier;
        ReturnErrorOnFailure(verifier.Generate(iterationCount, saltSpan, setupPasscode));
        ReturnErrorOnFailure(verifier.Serialize(verifierBuf));

        outVerifierLen = verifierBuf.size();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSetupPasscode(uint32_t &setupPasscode) override
    {
        VerifyOrReturnError(chip::SetupPayload::IsValidSetupPIN(kSetupPinCode), CHIP_ERROR_INVALID_ARGUMENT);
        setupPasscode = kSetupPinCode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override
    {
        (void)setupPasscode;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    static constexpr uint32_t kIterationCount = 10000;
    static constexpr uint8_t kSalt[16] = {
        0x73, 0x61, 0x6c, 0x74, 0x2d, 0x64, 0x69, 0x6f,
        0x74, 0x2d, 0x63, 0x36, 0x2d, 0x30, 0x31, 0x00,
    };
};

fixed_commissionable_data_provider sProvider;

} // namespace

namespace diot::commissioning {

CHIP_ERROR configure_commissioning()
{
    uint32_t setupPin = 0;
    ReturnErrorOnFailure(sProvider.GetSetupPasscode(setupPin));
    chip::DeviceLayer::SetCommissionableDataProvider(&sProvider);
    return CHIP_NO_ERROR;
}

CHIP_ERROR configure_ble_name()
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    return chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(kBleDeviceName);
#else
    (void)kBleDeviceName;
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

} // namespace diot::commissioning
