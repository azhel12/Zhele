#define F_CPU 8000000

#include <iopins.h>
#include <drivers/hd44780.h>

using namespace Zhele::IO;
using namespace Zhele::Drivers;

int main()
{
	// Check docs for your board. Choose any free io pins.
	// For example, for my stm32f103c8t6, stm32f401cc, stm32f030f4p6 Pa0...Pa5 set is possible.
	using lcd = Lcd<Pa0, Pa1, Pa2, Pa3, Pa4, Pa5>;
	// But for stm32f072rbt6 demo board Pa0-Pa3, Pa7 are busy.
	//using lcd = Lcd<Pa1, Pa4, Pa5, Pa8, Pa9, Pa10>;

	lcd::Init();
	lcd::Clear();
	lcd::Puts("Welcome to hell!");

	for (;;)
	{
	}
}
