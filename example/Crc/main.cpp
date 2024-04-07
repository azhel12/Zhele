#include <zhele/crc.h>

int main()
{
#if defined (CRC_POL_POL)
    Zhele::Crc::SetPolynom(0x12345678);
#endif

    uint32_t polynom = Zhele::Crc::GetPolynom();

    uint8_t data[] = {0xde, 0xad, 0xbe, 0xef};
    uint32_t crc = Zhele::Crc::CalculateCrc32(data, sizeof(data));

    for(;;)
    {
    }
}