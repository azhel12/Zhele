/**
 * @file
 * K1921VG5T: instantiate SPI instances Spi0..Spi1 (PL022 with a fused CR).
 *
 * Pin options (РП таблица 3.2; the bracketed number is the ALTFUNCNUM value):
 *   SPI0  CLK: Pb0  (AF1), Pb4  (AF2)   FSS: Pb1  (AF1), Pb5  (AF2)
 *         RX:  Pb2  (AF1), Pb6  (AF2)   TX:  Pb3  (AF1), Pb7  (AF2)
 *   SPI1  CLK: Pa12 (AF3), Pb10 (AF3)   FSS: Pa13 (AF3), Pb11 (AF3)
 *         RX:  Pa14 (AF3), Pb12 (AF3)   TX:  Pa15 (AF3), Pb13 (AF3)
 *
 * Zhele's portable naming maps onto the PL022 signals as
 * MOSI = TX, MISO = RX, SCK = CLK, SS = FSS.
 * Pb0..Pb3 are also ADC inputs (permanently connected, not an alternate function),
 * so SPI0 on that set and the ADC cannot be used together.
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG5T_SPI_H
#define ZHELE_PLATFORM_NIIET_K1921VG5T_SPI_H

#include <K1921VG5T.h>

#include "../common/spi.h"

#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <array>
#include <cstdint>

namespace Zhele
{
    namespace Private
    {
        struct Spi0MosiPins { using io_pins = IO::PinList<IO::Pb3,  IO::Pb7>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi0MisoPins { using io_pins = IO::PinList<IO::Pb2,  IO::Pb6>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi0ClkPins  { using io_pins = IO::PinList<IO::Pb0,  IO::Pb4>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi0SsPins   { using io_pins = IO::PinList<IO::Pb1,  IO::Pb5>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Spi1MosiPins { using io_pins = IO::PinList<IO::Pa15, IO::Pb13>; static constexpr std::array<uint8_t, 2> alt_functions{3, 3}; };
        struct Spi1MisoPins { using io_pins = IO::PinList<IO::Pa14, IO::Pb12>; static constexpr std::array<uint8_t, 2> alt_functions{3, 3}; };
        struct Spi1ClkPins  { using io_pins = IO::PinList<IO::Pa12, IO::Pb10>; static constexpr std::array<uint8_t, 2> alt_functions{3, 3}; };
        struct Spi1SsPins   { using io_pins = IO::PinList<IO::Pa13, IO::Pb11>; static constexpr std::array<uint8_t, 2> alt_functions{3, 3}; };

        IO_STRUCT_WRAPPER(SPI0, Spi0Regs, SPI_TypeDef);
        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
    } // namespace Private

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi0 = Private::Spi<Private::Spi0Regs, Clock::Spi0Clock, _DmaTx, _DmaRx,
                              Private::Spi0MosiPins, Private::Spi0MisoPins, Private::Spi0ClkPins, Private::Spi0SsPins>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi1 = Private::Spi<Private::Spi1Regs, Clock::Spi1Clock, _DmaTx, _DmaRx,
                              Private::Spi1MosiPins, Private::Spi1MisoPins, Private::Spi1ClkPins, Private::Spi1SsPins>;
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG5T_SPI_H
