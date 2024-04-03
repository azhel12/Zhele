#include <zhele/clock.h>
#include <zhele/iopins.h>
#include <zhele/spi.h>
#include <zhele/usart.h>

#include <zhele/drivers/sdcard.h>
#include <zhele/drivers/filesystem/fatfs/ff.h>

#include <cstring>

using namespace Zhele;
using namespace Zhele::Clock;
using namespace Zhele::IO;

#if defined (STM32G0)
using SpiInterface = Spi1<>;
#else
using SpiInterface = Spi1;
#endif

using SdCardReader = Drivers::SdCard<SpiInterface, IO::Pa4>;
using UsartConnection = Usart1;

void ConfigureClock();

int main()
{	
    ConfigureClock();

    UsartConnection::Init(9600);
    UsartConnection::SelectTxRxPins<Pa9, Pa10>();
    UsartConnection::EnableInterrupt(UsartConnection::InterruptFlags::RxNotEmptyInt);
    UsartConnection::Write("Hello\r\n", 7);


    SpiInterface::Init(SpiInterface::Fast, SpiInterface::Master);
    SpiInterface::SelectPins<Pa7, Pa6, Pa5, Pa4>();

    for (;;)
    {
    }
}

void ConfigureClock()
{
    PllClock::SelectClockSource(PllClock::ClockSource::External);
    PllClock::SetMultiplier(9);
    Apb1Clock::SetPrescaler(Apb1Clock::Div2);
    SysClock::SelectClockSource(SysClock::Pll);
}

extern "C"
{
    void USART1_IRQHandler()
    {
        char command;

        if(UsartConnection::ReadReady())
        {
            command = UsartConnection::Read();
        }

        static FATFS fs;
        FRESULT res;
        char buffer[32];
        DIR dir;

        switch (command)
        {
            case 'h':
                UsartConnection::Write("Hello\r\n", 7);
                break;

            case 'm':
            {
                auto sdCardType = SdCardReader::Detect();

                if(sdCardType == Drivers::SdCardType::SdCardNone)
                {
                    UsartConnection::Write("Card detect fail\r\n", 18);
                }

                res = f_mount(&fs, "", 1);

                if(res == FR_OK)
                {
                    UsartConnection::Write("Mount success\r\n", 15);
                }
                else
                {
                    UsartConnection::Write("Mount fail\r\n", 12);
                }
                
                break;
            }
            case 'u':
            {
                res = f_unmount("");

                if(res == FR_OK)
                {
                    UsartConnection::Write("Umount success\r\n", 16);
                }
                else
                {
                    UsartConnection::Write("Umount fail\r\n", 13);
                }
                break;
            }
            case 'l':
            {
                res = f_opendir(&dir, "/");

                if(res != FR_OK)
                {
                    UsartConnection::Write("List dir fail\r\n", 15);
                    break;
                }

                FILINFO fileInfo;
                
                for(;;)
                {
                    res = f_readdir(&dir, &fileInfo);
                    if((res != FR_OK) || (fileInfo.fname[0] == '\0'))
                        break;

                    strcpy(buffer, fileInfo.fname);
                    strcat(buffer, "\r\n");

                    UsartConnection::Write(buffer, strlen(buffer));
                }
                f_closedir(&dir);
                break;
            }

            case 'r':
            {
                FIL file;
                UINT bytesReaded;
                res = f_open(&file, "hello.txt", FA_OPEN_EXISTING | FA_READ);

                if(res != FR_OK)
                {
                    UsartConnection::Write("Read hello.txt fail\r\n", 21);
                    break;
                }

                
                f_read(&file, buffer, 32, &bytesReaded);
                f_close(&file);
                strcpy(&buffer[bytesReaded], "\r\n\0");
                UsartConnection::Write(buffer, strlen(buffer));
                break;
            }
            default:
                UsartConnection::Write("Unknown command\r\n", 17);
        }

        UsartConnection::ClearInterruptFlag(UsartConnection::InterruptFlags::RxNotEmptyInt);
    }
}
