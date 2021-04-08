/**
 * @file
 * Driver for ds1307 RTC
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_DRIVERS_DS1307_H
#define ZHELE_DRIVERS_DS1307_H

namespace Zhele::Drivers
{
	/**
	 * @brief Class for DS1307 RTC
	 * 
	 * @tparam _I2CBus Target I2C type (See i2c.h)
	 */
	template <typename _I2CBus>
	class Ds1307
	{
		const static uint8_t  Ds1307Address = (0xD0 >> 1); ///< I2C address in 7-bit mode

		// Registers location
		enum Registers
		{
			Seconds = 0x00,
			Minutes = 0x01,
			Hours = 0x02,
			Weekday = 0x03,
			Day = 0x04,
			Month = 0x05,
			Year = 0x06,
			Control = 0x07
		}
		;

		// Bits in control register
		enum ControlRegisterBits
		{
			Rs0 = 0x00,
			Rs1 = 0x01,
			Sqwe = 0x04,
			Out = 0x07
		};

	public:
		struct Time
		{
			uint8_t Seconds; ///< Seconds parameter, from 00 to 59
			uint8_t Minutes; ///< Minutes parameter, from 00 to 59
			uint8_t Hours; ///< Hours parameter, 24Hour mode, 00 to 23
			uint8_t Weekday; ///< Day in a week, from 1 to 7
			uint8_t Day; ///< Day in a month, 1 to 31
			uint8_t Month; ///< Month in a year, 1 to 12
			uint8_t Year; ///< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099
		};

		/**
		 * @brief Initialize MCU to work with RTC
		 * 
		 * @details
		 * This method init I2C bus
		 */
		static void Init()
		{
			_I2CBus::Init();
		}

		/**
		 * @brief Returns Ds1307::Time struct object.
		 */
		static Time GetDateTime()
		{
			uint8_t data[7];
	
			/* Read multi bytes */
			_I2CBus::Read(Ds1307Address, 0x00, data, 7);

			Time& time = reinterpret_cast<Time&>(data);

			/* Fill data */
			time.Seconds = ConvertFromBcd(data[Registers::Seconds]);
			time.Minutes = ConvertFromBcd(data[Registers::Minutes]);
			time.Hours = ConvertFromBcd(data[Registers::Hours]);
			time.Weekday = ConvertFromBcd(data[Registers::Weekday]);
			time.Day = ConvertFromBcd(data[Registers::Day]);
			time.Month = ConvertFromBcd(data[Registers::Month]);
			time.Year = ConvertFromBcd(data[Registers::Year]);

			return time;
		}

		static void SetDateTime(const Time& time)
		{
			uint8_t data[7];
	
			data[Registers::Seconds] = ConvertToBcd(time.Seconds);
			data[Registers::Minutes] = ConvertToBcd(time.Minutes);
			data[Registers::Hours] = ConvertToBcd(time.Hours);
			data[Registers::Weekday] = ConvertToBcd(time.Weekday);
			data[Registers::Day] = ConvertToBcd(time.Day);
			data[Registers::Month] = ConvertToBcd(time.Month);
			data[Registers::Year] = ConvertToBcd(time.Year);

			_I2CBus::Write(Ds1307Address, 0x00, data, 7);
		}

		/**
		 * @brief Returns seconds
		 * 
		 * @returns Seconds value
		 */
		static uint8_t GetSeconds()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Seconds).Value);
		}

		/**
		 * @brief Set seconds for ds1307 unit
		 * 
		 * @param [in] seconds Seconds to set
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetSeconds(uint8_t seconds)
		{
			_I2CBus::WriteU8(Ds1307Address, Seconds, ConvertToBcd(seconds));
		}

		/**
		 * @brief Returns minutes
		 * 
		 * @returns Minutes value
		 */
		static uint8_t GetMinutes()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Minutes).Value);
		}

		/**
		 * @brief Set minutes for ds1307 unit
		 * 
		 * @param [in] minutes Minutes to set
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetMinutes(uint8_t minutes)
		{
			_I2CBus::WriteU8(Ds1307Address, Minutes, ConvertToBcd(minutes));
		}

		/**
		 * @brief Returns hours
		 * 
		 * @returns Hours value
		 */
		static uint8_t GetHours()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Hours).Value);
		}

		/**
		 * @brief Set hours for ds1307 unit
		 * 
		 * @param [in] hours Hours to set
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetHours(uint8_t hours)
		{
			_I2CBus::WriteU8(Ds1307Address, Hours, ConvertToBcd(hours));
		}

		/**
		 * @brief Returns weekday number
		 * 
		 * @returns Weekday number value
		 */
		static uint8_t GetWeekday()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Weekday).Value);
		}

		/**
		 * @brief Set weekday number for ds1307 unit
		 * 
		 * @param [in] weekday Weekday number to set (1 to 7)
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetWeekday(uint8_t weekday)
		{
			_I2CBus::WriteU8(Ds1307Address, Weekday, ConvertToBcd(weekday));
		}

		/**
		 * @brief Returns day in month
		 * 
		 * @returns Day in month value
		 */
		static uint8_t GetDay()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Day).Value);
		}

		/**
		 * @brief Set month day for ds1307 unit
		 * 
		 * @param [in] day Day to set
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetDay(uint8_t day)
		{
			_I2CBus::WriteU8(Ds1307Address, Day, ConvertToBcd(day));
		}

		/**
		 * @brief Returns month number
		 * 
		 * @returns Month number value
		 */
		static uint8_t GetMonth()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Month).Value);
		}

		/**
		 * @brief Set minutes for ds1307 unit
		 * 
		 * @param [in] month Month to set (1 - 12)
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetMonth(uint8_t month)
		{
			_I2CBus::WriteU8(Ds1307Address, Month, ConvertToBcd(month));
		}

		/**
		 * @brief Returns seconds
		 * 
		 * @returns Seconds value
		 */
		static uint8_t GetYear()
		{
			return ConvertFromBcd(_I2CBus::ReadU8(Ds1307Address, Year).Value);
		}

		/**
		 * @brief Set year for ds1307 unit
		 * 
		 * @param [in] year Year to set (0 - 99)
		 * 
		 * @par Returns
		 *	Nothing
		 */
		static void SetYear(uint8_t year)
		{
			_I2CBus::WriteU8(Ds1307Address, Year, ConvertToBcd(year));
		}

	private:
		/**
		 * @brief Convert from bcd value
		 * 
		 * @details
		 * ds1307 store values in bcd format, so after read
		 * value it should be decoded to normal.
		 * 
		 * @param [in] bcd Encoded by bcd value
		 * 
		 * @returns Decoded (converted) from bcd value
		 */
		static uint8_t ConvertFromBcd(uint8_t bcd)
		{
			return 10 * (bcd >> 4) + (bcd & 0x0f);
		}

		/**
		 * @brief Convert to bcd value
		 * 
		 * @details
		 * ds1307 store values in bcd format, so before
		 * writing to ds1307 unit value should be encoded to bcd format.
		 * 
		 * @param [in] bin Value to encode
		 * 
		 * @returns Encoded (converted) to bcd value
		 */
		static uint8_t ConvertToBcd(uint8_t bin)
		{	
			return (bin / 10) << 4 | (bin % 10);
		}
	};
}
#endif // !ZHELE_DRIVERS_DS1307_H