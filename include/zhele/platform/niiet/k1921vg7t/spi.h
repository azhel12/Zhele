/**
 * @file
 * K1921VG7T: instantiate SPI instances Spi0..Spi2 (PL022 with a fused CR).
 *
 * Pin options (РП таблица 3.2; the bracketed number is the ALTFUNCNUM value):
 *   SPI0  CLK: Pa4  (AF1), Pb0  (AF2)   FSS: Pa5  (AF1), Pb1  (AF2)
 *         RX:  Pa6  (AF1), Pb2  (AF2)   TX:  Pa7  (AF1), Pb3  (AF2)
 *   SPI1  CLK: Pa12 (AF1), Pb4  (AF2)   FSS: Pa13 (AF1), Pb5  (AF2)
 *         RX:  Pa14 (AF1), Pb6  (AF2)   TX:  Pa15 (AF1), Pb7  (AF2)
 *   SPI2  CLK: Pa0  (AF1), Pb8  (AF2)   FSS: Pa1  (AF1), Pb9  (AF2)
 *         RX:  Pa2  (AF1), Pb10 (AF2)   TX:  Pa3  (AF1), Pb11 (AF2)
 *
 * Zhele's portable naming maps onto the PL022 signals as
 * MOSI = TX, MISO = RX, SCK = CLK, SS = FSS.
 * Note Pa12..Pa15 are the JTAG pins (TMS/TCK/TDI/TDO): SPI1 on port A costs the
 * debug port, so prefer the port B set on a board that keeps JTAG.
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG7T_SPI_H
#define ZHELE_PLATFORM_NIIET_K1921VG7T_SPI_H

#include <K1921VG7T.h>

#include "../common/spi.h"

#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <array>
#include <cstdint>

namespace Zhele
{
    namespace Private
    {
        struct Spi0MosiPins { using io_pins = IO::PinList<IO::Pa7,  IO::Pb3>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi0MisoPins { using io_pins = IO::PinList<IO::Pa6,  IO::Pb2>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi0ClkPins  { using io_pins = IO::PinList<IO::Pa4,  IO::Pb0>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi0SsPins   { using io_pins = IO::PinList<IO::Pa5,  IO::Pb1>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Spi1MosiPins { using io_pins = IO::PinList<IO::Pa15, IO::Pb7>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi1MisoPins { using io_pins = IO::PinList<IO::Pa14, IO::Pb6>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi1ClkPins  { using io_pins = IO::PinList<IO::Pa12, IO::Pb4>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi1SsPins   { using io_pins = IO::PinList<IO::Pa13, IO::Pb5>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Spi2MosiPins { using io_pins = IO::PinList<IO::Pa3,  IO::Pb11>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi2MisoPins { using io_pins = IO::PinList<IO::Pa2,  IO::Pb10>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi2ClkPins  { using io_pins = IO::PinList<IO::Pa0,  IO::Pb8>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Spi2SsPins   { using io_pins = IO::PinList<IO::Pa1,  IO::Pb9>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        IO_STRUCT_WRAPPER(SPI0, Spi0Regs, SPI_TypeDef);
        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
        IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
    } // namespace Private

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi0 = Private::Spi<Private::Spi0Regs, Clock::Spi0Clock, _DmaTx, _DmaRx,
                              Private::Spi0MosiPins, Private::Spi0MisoPins, Private::Spi0ClkPins, Private::Spi0SsPins>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi1 = Private::Spi<Private::Spi1Regs, Clock::Spi1Clock, _DmaTx, _DmaRx,
                              Private::Spi1MosiPins, Private::Spi1MisoPins, Private::Spi1ClkPins, Private::Spi1SsPins>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi2 = Private::Spi<Private::Spi2Regs, Clock::Spi2Clock, _DmaTx, _DmaRx,
                              Private::Spi2MosiPins, Private::Spi2MisoPins, Private::Spi2ClkPins, Private::Spi2SsPins>;
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG7T_SPI_H
