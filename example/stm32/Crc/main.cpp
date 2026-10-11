#include <zhele/crc.h>

int main()
{
    Zhele::Crc::Enable();
#if defined (CRC_POL_POL)
    // The CRC unit supports odd polynomials only (CRC-32C here)
    Zhele::Crc::SetPolynom(0x1EDC6F41);
#endif

    uint32_t polynom = Zhele::Crc::GetPolynom();

    uint8_t data[] = "123456789";
    uint32_t crc = Zhele::Crc::CalculateCrc32(data, sizeof(data));

    for(;;)
    {
    }
}