/**
 * @file tmc2209.h
 * 
 * @brief Contains TMC2209 driver code
 * 
 * @author zhel, Peter Polidoro (UART version ported from https://github.com/janelia-arduino/TMC2209 by zhel)
 * 
 * @date 2024
 * 
 * @copyright LLC United Water
*/

#ifndef ZHELE_DRIVERS_TMC2209_H_
#define ZHELE_DRIVERS_TMC2209_H_

#include <limits>
#include <cstdint>

#include <zhele/iopins.h>

namespace Zhele::Drivers {
    /**
     * @brief Implements stepper motor control over TMC2226 (2209) driver by step-dir
     * 
     * @tparam _dir_pin Direction IO
     * @tparam _step_pwm_timer Assigned timer instance for PWM
     * @tparam _pwm_channel Timer's channel number for use as PWM
     * @tparam _step_pwm_pin Step pin (deafult: first avaliable IO pin for given channel)
    */
    template<typename _dir_pin, typename _step_pwm_timer, unsigned _pwm_channel, typename _step_pwm_pin = typename _step_pwm_timer::template PWMGeneration<_pwm_channel>::Pins::template Pin<0>>
    class tmc2209_stepdir
    {
        using step_timer = _step_pwm_timer;
        using step_pwm = step_timer::template PWMGeneration<_pwm_channel>;

        static constexpr uint32_t _pwm_period = 100;
    public:
        static constexpr uint32_t steps_per_rotate = 200;

        /**
         * @brief Initialize module for control by step/dir
         * 
         * @param [in] rpm Initial RPM
         * 
         * @par Returns
         *  Nothing
        */
        static void init(uint16_t rpm = 100) {
            step_timer::Enable();
            set_rpm(rpm);
            step_timer::SetPeriod(_pwm_period - 1);

            step_pwm::template SelectPins<_step_pwm_pin>();
            step_pwm::Enable();
            step_pwm::SetOutputMode(step_pwm::OutputMode::PWM1);
            step_pwm::SetPulse(_pwm_period / 2);
        }

        /**
         * @brief Set motor RPM speed
         * 
         * @param [in] rpm RPM
         * 
         * @par Returns
         *  Nothing
        */
        static void set_rpm(uint16_t rpm) {
            uint32_t prescaler = (step_timer::GetClockFreq() / _pwm_period) * 60 / steps_per_rotate / rpm;
            step_timer::SetPrescaler(prescaler);
        }

        /**
         * @brief Start motor (enable timer and PWM output)
         * 
         * @par Returns
         *  Nothing
        */
        static void start() {
            step_timer::Start();
        }

        /**
         * @brief Stop motor (stop timer and PWM output)
         * 
         * @par Returns
         *  Nothing
        */
        static void stop() {
            step_timer::Stop();
        }
    };

    /**
     * @brief Implements stepper motor control over TMC2226 (2209) driver by UART
    */
    template<typename _uart, typename _tx_pin, uint8_t _serial_address = 0, typename _rx_pin = Zhele::IO::NullPin, typename _en_pin = Zhele::IO::NullPin>
    class tmc2209
    {
        /// @brief StandStill mode
        enum StandstillMode {
            NORMAL = 0,
            FREEWHEELING = 1,
            STRONG_BRAKING = 2,
            BRAKING = 3,
        };

        /// @brief CoolStep current increment
        enum CurrentIncrement {
            CURRENT_INCREMENT_1 = 0,
            CURRENT_INCREMENT_2 = 1,
            CURRENT_INCREMENT_4 = 2,
            CURRENT_INCREMENT_8 = 3,
        };

        /// @brief CoolStep measurement count
        enum MeasurementCount {
            MEASUREMENT_COUNT_32 = 0,
            MEASUREMENT_COUNT_8 = 1,
            MEASUREMENT_COUNT_2 = 2,
            MEASUREMENT_COUNT_1 = 3,
        };

        /// @brief Whole driver settings
        struct Settings {
            bool is_communicating;
            bool is_setup;
            bool software_enabled;
            uint16_t microsteps_per_step;
            bool inverse_motor_direction_enabled;
            bool stealth_chop_enabled;
            uint8_t standstill_mode;
            uint8_t irun_percent;
            uint8_t irun_register_value;
            uint8_t ihold_percent;
            uint8_t ihold_register_value;
            uint8_t iholddelay_percent;
            uint8_t iholddelay_register_value;
            bool automatic_current_scaling_enabled;
            bool automatic_gradient_adaptation_enabled;
            uint8_t pwm_offset;
            uint8_t pwm_gradient;
            bool cool_step_enabled;
            bool analog_current_scaling_enabled;
            bool internal_sense_resistors_enabled;
        };

        /// @brief Whole driver status
        struct Status
        {
            uint32_t over_temperature_warning : 1;
            uint32_t over_temperature_shutdown : 1;
            uint32_t short_to_ground_a : 1;
            uint32_t short_to_ground_b : 1;
            uint32_t low_side_short_a : 1;
            uint32_t low_side_short_b : 1;
            uint32_t open_load_a : 1;
            uint32_t open_load_b : 1;
            uint32_t over_temperature_120c : 1;
            uint32_t over_temperature_143c : 1;
            uint32_t over_temperature_150c : 1;
            uint32_t over_temperature_157c : 1;
            uint32_t reserved0 : 4;
            uint32_t current_scaling : 5;
            uint32_t reserved1 : 9;
            uint32_t stealth_chop_mode : 1;
            uint32_t standstill : 1;
        };

        /// @brief Write-read-reply datagram
        union WriteReadReplyDatagram {
            struct {
                uint64_t sync : 4;
                uint64_t reserved : 4;
                uint64_t serial_address : 8;
                uint64_t register_address : 7;
                uint64_t rw : 1;
                uint64_t data : 32;
                uint64_t crc : 8;
            };
            uint64_t bytes;
        };
        static_assert(sizeof(WriteReadReplyDatagram) == 8, "Invalid WriteReadReplyDatagram size!");
        const static uint8_t SYNC = 0b101;
        const static uint8_t RW_READ = 0;
        const static uint8_t RW_WRITE = 1;
        const static uint8_t READ_REPLY_SERIAL_ADDRESS = 0b11111111;


        /// @brief Read request datagram
        union ReadRequestDatagram {
            struct {
                uint32_t sync : 4;
                uint32_t reserved : 4;
                uint32_t serial_address : 8;
                uint32_t register_address : 7;
                uint32_t rw : 1;
                uint32_t crc : 8;
            };
            uint32_t bytes;
        };
        static_assert(sizeof(ReadRequestDatagram) == 4, "Invalid ReadRequestDatagram size!");

        // General Configuration Registers

        /// @brief GCONF
        const static uint8_t ADDRESS_GCONF = 0x00;
        union GlobalConfig {
            struct {
                uint32_t i_scale_analog : 1;
                uint32_t internal_rsense : 1;
                uint32_t enable_spread_cycle : 1;
                uint32_t shaft : 1;
                uint32_t index_otpw : 1;
                uint32_t index_step : 1;
                uint32_t pdn_disable : 1;
                uint32_t mstep_reg_select : 1;
                uint32_t multistep_filt : 1;
                uint32_t test_mode : 1;
                uint32_t reserved : 22;
            };
            uint32_t bytes;
        };
        static GlobalConfig global_config_;

        /// @brief GSTAT
        const static uint8_t ADDRESS_GSTAT = 0x01;
        union GlobalStatus {
            struct {
                uint32_t reset : 1;
                uint32_t drv_err : 1;
                uint32_t uv_cp : 1;
                uint32_t reserved : 29;
            };
            uint32_t bytes;
        };

        /// @brief IFCNT
        const static uint8_t ADDRESS_IFCNT = 0x02;

        /// @brief REPLYDELAY
        const static uint8_t ADDRESS_REPLYDELAY = 0x03;
        union ReplyDelay {
            struct {
                uint32_t reserved_0 : 8;
                uint32_t replydelay : 4;
                uint32_t reserved_1 : 20;
            };
            uint32_t bytes;
        };

        /// @brief IOIN
        const static uint8_t ADDRESS_IOIN = 0x06;
        union Input {
            struct {
                uint32_t enn : 1;
                uint32_t reserved_0 : 1;
                uint32_t ms1 : 1;
                uint32_t ms2 : 1;
                uint32_t diag : 1;
                uint32_t reserved_1 : 1;
                uint32_t pdn_serial : 1;
                uint32_t step : 1;
                uint32_t spread_en : 1;
                uint32_t dir : 1;
                uint32_t reserved_2 : 14;
                uint32_t version : 8;
            };
            uint32_t bytes;
        };

        /// @brief VERSION
        const static uint8_t VERSION = 0x21;


        // Velocity Dependent Driver Feature Control Register Set

        /// @brief IHOLD_IRUN
        const static uint8_t ADDRESS_IHOLD_IRUN = 0x10;
        union DriverCurrent {
            struct {
                uint32_t ihold : 5;
                uint32_t reserved_0 : 3;
                uint32_t irun : 5;
                uint32_t reserved_1 : 3;
                uint32_t iholddelay : 4;
                uint32_t reserved_2 : 12;
            };
            uint32_t bytes;
        };
        static DriverCurrent driver_current_;
        const static uint8_t PERCENT_MIN = 0;
        const static uint8_t PERCENT_MAX = 100;
        const static uint8_t CURRENT_SETTING_MIN = 0;
        const static uint8_t CURRENT_SETTING_MAX = 31;
        const static uint8_t HOLD_DELAY_MIN = 0;
        const static uint8_t HOLD_DELAY_MAX = 15;
        const static uint8_t IHOLD_DEFAULT = 16;
        const static uint8_t IRUN_DEFAULT = 31;
        const static uint8_t IHOLDDELAY_DEFAULT = 1;

        const static uint8_t ADDRESS_TPOWERDOWN = 0x11;
        const static uint8_t TPOWERDOWN_DEFAULT = 20;

        const static uint8_t ADDRESS_TSTEP = 0x12;

        const static uint8_t ADDRESS_TPWMTHRS = 0x13;
        const static uint32_t TPWMTHRS_DEFAULT = 0;

        const static uint8_t ADDRESS_VACTUAL = 0x22;
        const static int32_t VACTUAL_DEFAULT = 0;
        const static int32_t VACTUAL_STEP_DIR_INTERFACE = 0;

        // CoolStep and StallGuard Control Register Set
        const static uint8_t ADDRESS_TCOOLTHRS = 0x14;
        const static uint8_t TCOOLTHRS_DEFAULT = 0;
        const static uint8_t ADDRESS_SGTHRS = 0x40;
        const static uint8_t SGTHRS_DEFAULT = 0;
        const static uint8_t ADDRESS_SG_RESULT = 0x41;

        /// @brief COOLCONF
        const static uint8_t ADDRESS_COOLCONF = 0x42;
        const static uint8_t COOLCONF_DEFAULT = 0;
        union CoolConfig {
                struct {
                uint32_t semin : 4;
                uint32_t reserved_0 : 1;
                uint32_t seup : 2;
                uint32_t reserved_1 : 1;
                uint32_t semax : 4;
                uint32_t reserved_2 : 1;
                uint32_t sedn : 2;
                uint32_t seimin : 1;
                uint32_t reserved_3 : 16;
            };
            uint32_t bytes;
        };
        static CoolConfig cool_config_;
        static bool cool_step_enabled_;

        const static uint8_t SEIMIN_UPPER_CURRENT_LIMIT = 20;
        const static uint8_t SEIMIN_LOWER_SETTING = 0;
        const static uint8_t SEIMIN_UPPER_SETTING = 1;
        const static uint8_t SEMIN_OFF = 0;
        const static uint8_t SEMIN_MIN = 1;
        const static uint8_t SEMIN_MAX = 15;
        const static uint8_t SEMAX_MIN = 0;
        const static uint8_t SEMAX_MAX = 15;

        // Microstepping Control Register Set
        const static uint8_t ADDRESS_MSCNT = 0x6A;
        const static uint8_t ADDRESS_MSCURACT = 0x6B;

        // Driver Register Set
        const static uint8_t ADDRESS_CHOPCONF = 0x6C;
        union ChopperConfig {
            struct {
                uint32_t toff : 4;
                uint32_t hstart : 3;
                uint32_t hend : 4;
                uint32_t reserved_0 : 4;
                uint32_t tbl : 2;
                uint32_t vsense : 1;
                uint32_t reserved_1 : 6;
                uint32_t mres : 4;
                uint32_t interpolation : 1;
                uint32_t double_edge : 1;
                uint32_t diss2g : 1;
                uint32_t diss2vs : 1;
            };
            uint32_t bytes;
        };
        static ChopperConfig chopper_config_;

        const static uint32_t CHOPPER_CONFIG_DEFAULT = 0x10000053;
        const static uint8_t TBL_DEFAULT = 0b10;
        const static uint8_t HEND_DEFAULT = 0;
        const static uint8_t HSTART_DEFAULT = 5;
        const static uint8_t TOFF_DEFAULT = 3;
        const static uint8_t TOFF_DISABLE = 0;
        static uint8_t toff_;
        const static uint8_t MRES_001 = 0b1000;

        const static size_t MICROSTEPS_PER_STEP_MIN = 1;
        const static size_t MICROSTEPS_PER_STEP_MAX = 256;

        /// @brief DRV_STATUS
        const static uint8_t ADDRESS_DRV_STATUS = 0x6F;
        union DriveStatus {
            struct
            {
                Status status;
            };
            uint32_t bytes;
        };

        /// @brief PWMCONF
        const static uint8_t ADDRESS_PWMCONF = 0x70;
        union PwmConfig {
            struct {
                uint32_t pwm_offset : 8;
                uint32_t pwm_grad : 8;
                uint32_t pwm_freq : 2;
                uint32_t pwm_autoscale : 1;
                uint32_t pwm_autograd : 1;
                uint32_t freewheel : 2;
                uint32_t reserved : 2;
                uint32_t pwm_reg : 4;
                uint32_t pwm_lim : 4;
            };
            uint32_t bytes;
        };
        static PwmConfig pwm_config_;

        const static uint32_t PWM_CONFIG_DEFAULT = 0xC10D0024;
        const static uint8_t PWM_OFFSET_MIN = 0;
        const static uint8_t PWM_OFFSET_MAX = 255;
        const static uint8_t PWM_OFFSET_DEFAULT = 0x24;
        const static uint8_t PWM_GRAD_MIN = 0;
        const static uint8_t PWM_GRAD_MAX = 255;
        const static uint8_t PWM_GRAD_DEFAULT = 0x14;

        /// @brief PWM_SCALE
        union PwmScale {
            struct {
                uint32_t pwm_scale_sum : 8;
                uint32_t reserved_0 : 8;
                uint32_t pwm_scale_auto : 9;
                uint32_t reserved_1 : 7;
            };
            uint32_t bytes;
        };
        const static uint8_t ADDRESS_PWM_SCALE = 0x71;

        /// @brief PWM_AUTO
        union PwmAuto {
            struct {
                uint32_t pwm_offset_auto : 8;
                uint32_t reserved_0 : 8;
                uint32_t pwm_gradient_auto : 8;
                uint32_t reserved_1 : 8;
            };
            uint32_t bytes;
        };
        const static uint8_t ADDRESS_PWM_AUTO = 0x72;

        const static uint8_t STEPPER_DRIVER_FEATURE_OFF = 0;
        const static uint8_t STEPPER_DRIVER_FEATURE_ON = 1;

    public:
        static_assert(_serial_address <= 3, "Serial address can be only 0..3");

        /**
         * @brief Initialize driver
         * 
         * @param [in] baud Serial speed (115200 by default)
         * 
         * @par Returns
         *  Nothing
        */
        static void init(unsigned baud = 115200) {
            if constexpr(std::is_same_v<_rx_pin, Zhele::IO::NullPin>) {
                _uart::Init(baud, _uart::UsartMode::Default | _uart::UsartMode::HalfDuplex);
            } else {
                _uart::Init(baud);
            }
            _uart::template SelectTxRxPins<_tx_pin, _rx_pin>();

            if constexpr(!std::is_same_v<_en_pin, Zhele::IO::NullPin>) {
                _en_pin::Port::Enable();
                _en_pin::template SetConfiguration<_en_pin::Configuration::Out>();
                _en_pin::template SetDriverType<_en_pin::DriverType::PushPull>();
                _en_pin::Set();
            }

            initialize();
        }

        /**
         * @brief Enable driver via EN pin
         * 
         * @par Returns
         *  Nothing
        */
        static void enable() {
            if constexpr(!std::is_same_v<_en_pin, Zhele::IO::NullPin>) {
                _en_pin::Clear();
            }

            chopper_config_.toff = toff_;
            writeStoredChopperConfig();
        }

        /**
         * @brief Disable driver via EN pin
         * 
         * @par Returns
         *  Nothing
        */
        static void disable() {
            if constexpr(!std::is_same_v<_en_pin, Zhele::IO::NullPin>) {
                _en_pin::Set();
            }

            chopper_config_.toff = TOFF_DISABLE;
            writeStoredChopperConfig();
        }

        /**
         * @brief Set microstep setting
         * 
         * @param [in] microsteps_per_step Microstep value (should ne power of 2: 1, 2, 4, 8, ... , 256)
         * 
         * @par Returns
         *  Nothing
        */
        static void setMicrostepsPerStep(uint16_t microsteps_per_step) {
            uint16_t microsteps_per_step_shifted = constrain_(microsteps_per_step, MICROSTEPS_PER_STEP_MIN, MICROSTEPS_PER_STEP_MAX) >> 1;
            uint16_t exponent = 0;
            while (microsteps_per_step_shifted > 0) {
                microsteps_per_step_shifted = microsteps_per_step_shifted >> 1;
                ++exponent;
            }
            setMicrostepsPerStepPowerOfTwo(exponent);
        }

        /**
         * @brief Set driver microstep setting 2^exponent
         * 
         * @param [in] exponent Power of two value (0..8)
         * 
         * @par Returns
         *  Nothing
        */
        static void setMicrostepsPerStepPowerOfTwo(uint8_t exponent) {
            chopper_config_.mres = MRES_001 - exponent;
            writeStoredChopperConfig();
        }

        /**
         * @brief Set run current (IRUN) in percent of max
         * 
         * @param [in] percent Current in percents (0..100)
         * 
         * @par Returns
         *  Nothing
        */
        static void setRunCurrent(uint8_t percent) {
            uint8_t run_current = percentToCurrentSetting(percent);
            driver_current_.irun = run_current;
            writeStoredDriverCurrent();
        }

        /**
         * @brief Set hold current (IHOLD) in percent of max
         * 
         * @param [in] percent Current in percents (0..100)
         * 
         * @par Returns
         *  Nothing
        */
        static void setHoldCurrent(uint8_t percent) {
            uint8_t hold_current = percentToCurrentSetting(percent);

            driver_current_.ihold = hold_current;
            writeStoredDriverCurrent();
        }
        
        /**
         * @brief Set hold delay (IHOLDDELAY) in percent of max
         * 
         * @param [in] percent Delay in percents (0..100)
         * 
         * @par Returns
         *  Nothing
        */
        static void setHoldDelay(uint8_t percent) {
            uint8_t hold_delay = percentToHoldDelaySetting(percent);

            driver_current_.iholddelay = hold_delay;
            writeStoredDriverCurrent();
        }

        /**
         * @brief Set all current config (IRUN/IHOLD/HOLDDELAY) in percent of max
         * 
         * @param [in] run_current_percent Run current in percents (0..100)
         * @param [in] hold_current_percent Hold current in percents (0..100)
         * @param [in] hold_delay_percent Hold delay current in percents (0..100)
         * 
         * @par Returns
         *  Nothing
        */
        static void setAllCurrentValues(uint8_t run_current_percent, uint8_t hold_current_percent, uint8_t hold_delay_percent) {
            uint8_t run_current = percentToCurrentSetting(run_current_percent);
            uint8_t hold_current = percentToCurrentSetting(hold_current_percent);
            uint8_t hold_delay = percentToHoldDelaySetting(hold_delay_percent);

            driver_current_.irun = run_current;
            driver_current_.ihold = hold_current;
            driver_current_.iholddelay = hold_delay;
            writeStoredDriverCurrent();
        }

        /**
         * @brief Enable motor direction inverse
         * 
         * @par Returns
         *  Nothing
        */
        static void enableInverseMotorDirection() {
            global_config_.shaft = 1;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Disable motor direction inverse
         * 
         * @par Returns
         *  Nothing
        */
        static void disableInverseMotorDirection() {
            global_config_.shaft = 0;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Set standstill mode
         * 
         * @param [in] mode Standstill mode
        */
        static void setStandstillMode(StandstillMode mode) {
            pwm_config_.freewheel = mode;
            writeStoredPwmConfig();
        }

        /**
         * @brief Enable automatic current scaling
         * 
         * @par Returns
         *  Nothing
        */
        static void enableAutomaticCurrentScaling() {
            pwm_config_.pwm_autoscale = STEPPER_DRIVER_FEATURE_ON;
            writeStoredPwmConfig();
        }

        /**
         * @brief Disable automatic current scaling
         * 
         * @par Returns
         *  Nothing
        */
        static void disableAutomaticCurrentScaling() {
            pwm_config_.pwm_autoscale = STEPPER_DRIVER_FEATURE_OFF;
            writeStoredPwmConfig();
        }

        /**
         * @brief Enable automatic gradient adaptation
         * 
         * @par Returns
         *  Nothing
        */
        static void enableAutomaticGradientAdaptation() {
            pwm_config_.pwm_autograd = STEPPER_DRIVER_FEATURE_ON;
         writeStoredPwmConfig();
        }

        /**
         * @brief Disable automatic gradient adaptation
         * 
         * @par Returns
         *  Nothing
        */
        static void disableAutomaticGradientAdaptation() {
            pwm_config_.pwm_autograd = STEPPER_DRIVER_FEATURE_OFF;
            writeStoredPwmConfig();
        }

        /**
         * @brief Set PWM offset
         * 
         * @param [in] pwm_amplitude Offset
         * 
         * @par Returns
         *  Nothing
        */
        static void setPwmOffset(uint8_t pwm_amplitude) {
            pwm_config_.pwm_offset = pwm_amplitude;
            writeStoredPwmConfig();
        }

        /**
         * @brief Set PWM gradient
         * 
         * @param [in] pwm_amplitude Gradient
         * 
         * @par Returns
         *  Nothing
        */
        static void setPwmGradient(uint8_t pwm_amplitude) {
            pwm_config_.pwm_grad = pwm_amplitude;
            writeStoredPwmConfig();
        }

        /**
         * @brief Set power-down delay
         * 
         * @param [in] power_down_delay Delay
         * 
         * @par Returns
         *  Nothing
        */
        static void setPowerDownDelay(uint8_t power_down_delay) {
            write(ADDRESS_TPOWERDOWN, power_down_delay);
        }

        /**
         * @brief Set reply delay
         * 
         * @param [in] reply_delay Delay
         * 
         * @par Returns
         *  Nothing
        */
        static void setReplyDelay(uint8_t reply_delay) {
            const static uint8_t REPLY_DELAY_MAX = 15;

            if (reply_delay > REPLY_DELAY_MAX) {
                reply_delay = REPLY_DELAY_MAX;
            }

            ReplyDelay reply_delay_data;
            reply_delay_data.bytes = 0;
            reply_delay_data.replydelay = reply_delay;
            write(ADDRESS_REPLYDELAY, reply_delay_data.bytes);
        }

        /**
         * @brief Move motor with given velocity
         * 
         * @param [in] microsteps_per_period Speed in microsteps
         * 
         * @par Returns
         *  Nothing
        */
        static void moveAtVelocity(int32_t microsteps_per_period) {
            write(ADDRESS_VACTUAL, microsteps_per_period);
        }

        /**
         * @brief Move with step-dir interface
         * 
         * @par Returns
         *  Nothing
        */
        static void moveUsingStepDirInterface() {
            write(ADDRESS_VACTUAL, VACTUAL_STEP_DIR_INTERFACE);
        }

        /**
         * @brief Enable StealthChop feauture (disable SpreadCycle)
         * 
         * @par Returns
         *  Nothing
        */
        static void enableStealthChop()
        {
            global_config_.enable_spread_cycle = 0;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Disable StealthChop feauture (enable SpreadCycle)
         * 
         * @par Returns
         *  Nothing
        */
        static void disableStealthChop()
        {
            global_config_.enable_spread_cycle = 1;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Set CoolStep duration threshold
         * 
         * @param [in] duration_threshold Threshold
         * 
         * @par Returns
         *  Nothing
        */
        static void setCoolStepDurationThreshold(uint32_t duration_threshold) {
            write(ADDRESS_TCOOLTHRS, duration_threshold);
        }

        /**
         * @brief Set StealthChop duration threshold
         * 
         * @param [in] duration_threshold Threshold
         * 
         * @par Returns
         *  Nothing
        */
        static void setStealthChopDurationThreshold(uint32_t duration_threshold)
        {
            write(ADDRESS_TPWMTHRS, duration_threshold);
        }

        /**
         * @brief Set StallGuard threshold
         * 
         * @param [in] stall_guard_threshold Threshold
         * 
         * @par Returns
         *  Nothing
        */
        static void setStallGuardThreshold(uint8_t stall_guard_threshold)
        {
            write(ADDRESS_SGTHRS, stall_guard_threshold);
        }

        /**
         * @brief Enable CoolStep feature
         * 
         * @param [in] lower_threshold Lower threshold (1 by default)
         * @param [in] upper_threshold Upper threshold (0 by default)
         * 
         * @par Returns
         *  Nothing
        */
        static void enableCoolStep(uint8_t lower_threshold = 1, uint8_t upper_threshold = 0)
        {
            lower_threshold = constrain_(lower_threshold, SEMIN_MIN, SEMIN_MAX);
            cool_config_.semin = lower_threshold;
            upper_threshold = constrain_(upper_threshold, SEMAX_MIN, SEMAX_MAX);
            cool_config_.semax = upper_threshold;
            write(ADDRESS_COOLCONF, cool_config_.bytes);
            cool_step_enabled_ = true;
        }

        /**
         * @brief Disable CoolStep
         * 
         * @par Returns
         *  Nothing
        */
        static void disableCoolStep() {
            cool_config_.semin = SEMIN_OFF;
            write(ADDRESS_COOLCONF, cool_config_.bytes);
            cool_step_enabled_ = false;
        }

        /**
         * @brief Set CoolStep current increment
         * 
         * @param [in] current_increment Increment value
         * 
         * @par Returns
         *  Nothing
        */
        static void setCoolStepCurrentIncrement(CurrentIncrement current_increment) {
            cool_config_.seup = current_increment;
            write(ADDRESS_COOLCONF, cool_config_.bytes);
        }

        /**
         * @brief Set CoolStep measurement count
         * 
         * @param [in] measurement_count Measurement count
         * 
         * @par Returns
         *  Nothing
         */
        static void setCoolStepMeasurementCount(MeasurementCount measurement_count) {
            cool_config_.sedn = measurement_count;
            write(ADDRESS_COOLCONF, cool_config_.bytes);
        }

        /**
         * @brief Enable analog cuurent scaling
         * 
         * @par Returns
         *  Nothing
        */
        static void enableAnalogCurrentScaling() {
            global_config_.i_scale_analog = 1;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Disable analog cuurent scaling
         * 
         * @par Returns
         *  Nothing
        */
        static void disableAnalogCurrentScaling() {
            global_config_.i_scale_analog = 0;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Use external Rsense
         * 
         * @par Returns
         *  Nothing
        */
        static void useExternalSenseResistors() {
            global_config_.internal_rsense = 0;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Use internal Rsense
         * 
         * @par Returns
         *  Nothing
        */
        static void useInternalSenseResistors() {
            global_config_.internal_rsense = 1;
            writeStoredGlobalConfig();
        }

        /**
         * @brief Returns driver version
         * 
         * @returns Driver version
        */
        static uint8_t getVersion() {
            Input input;
            input.bytes = read(ADDRESS_IOIN);

            return input.version;
        }

        /**
         * @brief Returns communicating is OK
         * 
         * @retval true Communicating OK
         * @retval false Communicating fail
        */
        static bool isCommunicating() {
            return (getVersion() == VERSION);
        }

        /**
         * @brief Returns communicating and setup are OK
         * 
         * @retval true Communicating and setup are OK
         * @retval false Communicating and setup are fail
        */
        static bool isSetupAndCommunicating() {
            return serialOperationMode();
        }

        /**
         * @brief Returns communicating OK but no setup
         * 
         * @retval true Communicating OK but no setup
         * @retval false Communicating Fail or setup OK
        */
        static bool isCommunicatingButNotSetup() {
            return (isCommunicating() && (!isSetupAndCommunicating()));
        }

        /**
         * @brief Check driver disabled
         * 
         * @retval true Disabled
         * @retval false Enabled
        */
        static bool hardwareDisabled() {
            Input input;
            input.bytes = read(ADDRESS_IOIN);

            return input.enn;
        }

        /**
         * @brief Returns current microsteps setting
         * 
         * @returns Microsteps setting
        */
        static uint16_t getMicrostepsPerStep() {
            uint16_t microsteps_per_step_exponent = MRES_001 - chopper_config_.mres;
            return 1 << microsteps_per_step_exponent;
        }

        /**
         * @brief Returns current whole settings
         * 
         * @returns Current settings
        */
        static Settings getSettings() {
            Settings settings;
            settings.is_communicating = isCommunicating();

            if (settings.is_communicating)
            {
                readAndStoreRegisters();

                settings.is_setup = global_config_.pdn_disable;
                settings.software_enabled = (chopper_config_.toff > TOFF_DISABLE);
                settings.microsteps_per_step = getMicrostepsPerStep();
                settings.inverse_motor_direction_enabled = global_config_.shaft;
                settings.stealth_chop_enabled = not global_config_.enable_spread_cycle;
                settings.standstill_mode = pwm_config_.freewheel;
                settings.irun_percent = currentSettingToPercent(driver_current_.irun);
                settings.irun_register_value = driver_current_.irun;
                settings.ihold_percent = currentSettingToPercent(driver_current_.ihold);
                settings.ihold_register_value = driver_current_.ihold;
                settings.iholddelay_percent = holdDelaySettingToPercent(driver_current_.iholddelay);
                settings.iholddelay_register_value = driver_current_.iholddelay;
                settings.automatic_current_scaling_enabled = pwm_config_.pwm_autoscale;
                settings.automatic_gradient_adaptation_enabled = pwm_config_.pwm_autograd;
                settings.pwm_offset = pwm_config_.pwm_offset;
                settings.pwm_gradient = pwm_config_.pwm_grad;
                settings.cool_step_enabled = cool_step_enabled_;
                settings.analog_current_scaling_enabled = global_config_.i_scale_analog;
                settings.internal_sense_resistors_enabled = global_config_.internal_rsense;
            }
            else
            {
                settings.is_setup = false;
                settings.software_enabled = false;
                settings.microsteps_per_step = 0;
                settings.inverse_motor_direction_enabled = false;
                settings.stealth_chop_enabled = false;
                settings.standstill_mode = pwm_config_.freewheel;
                settings.irun_percent = 0;
                settings.irun_register_value = 0;
                settings.ihold_percent = 0;
                settings.ihold_register_value = 0;
                settings.iholddelay_percent = 0;
                settings.iholddelay_register_value = 0;
                settings.automatic_current_scaling_enabled = false;
                settings.automatic_gradient_adaptation_enabled = false;
                settings.pwm_offset = 0;
                settings.pwm_gradient = 0;
                settings.cool_step_enabled = false;
                settings.analog_current_scaling_enabled = false;
                settings.internal_sense_resistors_enabled = false;
            }

            return settings;
        }

        /**
         * @brief Returns current driver status
         * 
         * @returns Status
        */
        static Status getStatus() {
            DriveStatus drive_status;
            drive_status.bytes = 0;
            drive_status.bytes = read(ADDRESS_DRV_STATUS);
            return drive_status.status;
        }

        /**
         * @brief Returns transmission counter
         * 
         * @returns Transmission counter
        */
        static uint8_t getInterfaceTransmissionCounter() {
            return read(ADDRESS_IFCNT);
        }

        /**
         * @brief Returns interstep duration
         * 
         * @returns Interstep duration
        */
        static uint32_t getInterstepDuration() {
            return read(ADDRESS_TSTEP);
        }

        /**
         * @brief Returns StallGuard result
         * 
         * @returns StallGuard result
        */
        static uint16_t getStallGuardResult() {
            return read(ADDRESS_SG_RESULT);
        }

        /**
         * @brief Returns PWM scale sum
         * 
         * @returns PWM scale sum
        */
        static uint8_t getPwmScaleSum() {
            PwmScale pwm_scale;
            pwm_scale.bytes = read(ADDRESS_PWM_SCALE);

            return pwm_scale.pwm_scale_sum;
        }

        /**
         * @brief Returns PWM scale auto value
         * 
         * @returns PWM scale auto
        */
        static int16_t getPwmScaleAuto() {
            PwmScale pwm_scale;
            pwm_scale.bytes = read(ADDRESS_PWM_SCALE);

            return pwm_scale.pwm_scale_auto;
        }

        /**
         * @brief Returns PWM offset auto value
         * 
         * @returns PWM offset auto
        */
        static uint8_t getPwmOffsetAuto() {
            PwmAuto pwm_auto;
            pwm_auto.bytes = read(ADDRESS_PWM_AUTO);

            return pwm_auto.pwm_offset_auto;
        }

        /**
         * @brief Returns PWM gradient auto value
         * 
         * @returns PWM gradient auto
        */
        static uint8_t getPwmGradientAuto() {
            PwmAuto pwm_auto;
            pwm_auto.bytes = read(ADDRESS_PWM_AUTO);

            return pwm_auto.pwm_gradient_auto;
        }

        /**
         * @brief Returns microstep counter
         * 
         * @returns Microstep counter
        */
        static uint16_t getMicrostepCounter() {
            return read(ADDRESS_MSCNT);
        }

    private:
        static void initialize()
        {
            setOperationModeToSerial();
            setRegistersToDefaults();

            minimizeMotorCurrent();
            disable();
            disableAutomaticCurrentScaling();
            disableAutomaticGradientAdaptation();
        }

        static bool serialAvailable() {
            return _uart::ReadReady();
        }

        static void serialWrite(uint8_t c) {
            _uart::Write(c);
        }

        static uint8_t serialRead() {
            return _uart::Read();
        }

        static void setOperationModeToSerial() {
            global_config_.bytes = 0;
            global_config_.i_scale_analog = 0;
            global_config_.pdn_disable = 1;
            global_config_.mstep_reg_select = 1;
            global_config_.multistep_filt = 1;

            writeStoredGlobalConfig();
        }

        static void setRegistersToDefaults() {
            driver_current_.bytes = 0;
            driver_current_.ihold = IHOLD_DEFAULT;
            driver_current_.irun = IRUN_DEFAULT;
            driver_current_.iholddelay = IHOLDDELAY_DEFAULT;
            write(ADDRESS_IHOLD_IRUN, driver_current_.bytes);

            chopper_config_.bytes = CHOPPER_CONFIG_DEFAULT;
            chopper_config_.tbl = TBL_DEFAULT;
            chopper_config_.hend = HEND_DEFAULT;
            chopper_config_.hstart = HSTART_DEFAULT;
            chopper_config_.toff = TOFF_DEFAULT;
            write(ADDRESS_CHOPCONF, chopper_config_.bytes);

            pwm_config_.bytes = PWM_CONFIG_DEFAULT;
            write(ADDRESS_PWMCONF, pwm_config_.bytes);

            cool_config_.bytes = COOLCONF_DEFAULT;
            write(ADDRESS_COOLCONF, cool_config_.bytes);

            write(ADDRESS_TPOWERDOWN, TPOWERDOWN_DEFAULT);
            write(ADDRESS_TPWMTHRS, TPWMTHRS_DEFAULT);
            write(ADDRESS_VACTUAL, VACTUAL_DEFAULT);
            write(ADDRESS_TCOOLTHRS, TCOOLTHRS_DEFAULT);
            write(ADDRESS_SGTHRS, SGTHRS_DEFAULT);
            write(ADDRESS_COOLCONF, COOLCONF_DEFAULT);
        }

        static void readAndStoreRegisters() {
            global_config_.bytes = readGlobalConfigBytes();
            chopper_config_.bytes = readChopperConfigBytes();
            pwm_config_.bytes = readPwmConfigBytes();
        }

        static bool serialOperationMode() {
            GlobalConfig global_config;
            global_config.bytes = readGlobalConfigBytes();

            return global_config.pdn_disable;
        }

        static void minimizeMotorCurrent() {
            driver_current_.irun = CURRENT_SETTING_MIN;
            driver_current_.ihold = CURRENT_SETTING_MIN;
            writeStoredDriverCurrent();
        }

        static uint32_t reverseData(uint32_t data) {
            uint32_t reversed_data = 0;
            uint8_t right_shift;
            uint8_t left_shift;
            for (int i = 0; i < sizeof(data); ++i)
            {
                right_shift = (sizeof(data) - i - 1) * std::numeric_limits<uint8_t>::digits;
                left_shift = i * std::numeric_limits<uint8_t>::digits;
                reversed_data |= ((data >> right_shift) & std::numeric_limits<uint8_t>::max()) << left_shift;
            }
            return reversed_data;
        }

        static uint8_t calculateCrc(auto&& datagram) {
            uint8_t crc = 0;
            uint8_t byte;
            for (int i = 0; i < (sizeof(datagram) - 1); ++i)
            {
                byte = (datagram.bytes >> (i * std::numeric_limits<uint8_t>::digits)) & std::numeric_limits<uint8_t>::max();
                for (int j = 0; j < std::numeric_limits<uint8_t>::digits; ++j)
                {
                    if ((crc >> 7) ^ (byte & 0x01)) {
                        crc = (crc << 1) ^ 0x07;
                    }
                    else {
                        crc = crc << 1;
                    }
                    byte = byte >> 1;
                }
            }
            return crc;
        }

        static void sendDatagramUnidirectional(auto&& datagram) {
            for (int i = 0; i < sizeof(datagram); ++i) {
                uint8_t byte = (datagram.bytes >> (i * std::numeric_limits<uint8_t>::digits)) & std::numeric_limits<uint8_t>::max();
                serialWrite(byte);
            }
        }

        static void sendDatagramBidirectional(auto&& datagram) {
            // clear the serial receive buffer if necessary
            while (serialAvailable())
                serialRead();

            // write datagram
            for (int i = 0; i < sizeof(datagram); ++i) {
                uint8_t byte = (datagram.bytes >> (i * std::numeric_limits<uint8_t>::digits)) & std::numeric_limits<uint8_t>::max();
                serialWrite(byte);
            }

            while (serialAvailable())
                serialRead();
        }

        static void write(uint8_t register_address, uint32_t data) {
            WriteReadReplyDatagram write_datagram;
            write_datagram.bytes = 0;
            write_datagram.sync = SYNC;
            write_datagram.serial_address = _serial_address;
            write_datagram.register_address = register_address;
            write_datagram.rw = RW_WRITE;
            write_datagram.data = reverseData(data);
            write_datagram.crc = calculateCrc(write_datagram);

            sendDatagramUnidirectional(write_datagram);
        }

        static uint32_t read(uint8_t register_address) {
            ReadRequestDatagram read_request_datagram;
            read_request_datagram.bytes = 0;
            read_request_datagram.sync = SYNC;
            read_request_datagram.serial_address = _serial_address;
            read_request_datagram.register_address = register_address;
            read_request_datagram.rw = RW_READ;
            read_request_datagram.crc = calculateCrc(read_request_datagram);

            sendDatagramBidirectional(read_request_datagram);

            uint64_t byte;
            uint8_t byte_count = 0;
            WriteReadReplyDatagram read_reply_datagram;
            read_reply_datagram.bytes = 0;
            for (uint8_t i = 0; i < sizeof(read_reply_datagram); ++i) {
                if (!serialAvailable())
                    return 0;

                byte = serialRead();
                read_reply_datagram.bytes |= (byte << (byte_count++ * std::numeric_limits<uint8_t>::digits));
            }

            return reverseData(read_reply_datagram.data);
        }

        static uint8_t percentToCurrentSetting(uint8_t percent) {
            uint8_t constrained_percent = constrain_(percent,
                PERCENT_MIN,
                PERCENT_MAX);
            uint8_t current_setting = map(constrained_percent,
                PERCENT_MIN,
                PERCENT_MAX,
                CURRENT_SETTING_MIN,
                CURRENT_SETTING_MAX);
            return current_setting;
        }

        static uint8_t currentSettingToPercent(uint8_t current_setting) {
            uint8_t percent = map(current_setting,
                CURRENT_SETTING_MIN,
                CURRENT_SETTING_MAX,
                PERCENT_MIN,
                PERCENT_MAX);
            return percent;
        }

        static uint8_t percentToHoldDelaySetting(uint8_t percent) {
            uint8_t constrained_percent = constrain_(percent,
                PERCENT_MIN,
                PERCENT_MAX);
            uint8_t hold_delay_setting = map(constrained_percent,
                PERCENT_MIN,
                PERCENT_MAX,
                HOLD_DELAY_MIN,
                HOLD_DELAY_MAX);
            return hold_delay_setting;
        }

        static uint8_t holdDelaySettingToPercent(uint8_t hold_delay_setting) {
            uint8_t percent = map(hold_delay_setting,
                HOLD_DELAY_MIN,
                HOLD_DELAY_MAX,
                PERCENT_MIN,
                PERCENT_MAX);
            return percent;
        }

        static void writeStoredGlobalConfig() {
            write(ADDRESS_GCONF, global_config_.bytes);
        }

        static uint32_t readGlobalConfigBytes() {
            return read(ADDRESS_GCONF);
        }

        static void writeStoredDriverCurrent() {
            write(ADDRESS_IHOLD_IRUN, driver_current_.bytes);

            if (driver_current_.irun >= SEIMIN_UPPER_CURRENT_LIMIT) {
                cool_config_.seimin = SEIMIN_UPPER_SETTING;
            }
            else {
                cool_config_.seimin = SEIMIN_LOWER_SETTING;
            }

            if (cool_step_enabled_) {
                write(ADDRESS_COOLCONF, cool_config_.bytes);
            }
        }

        static void writeStoredChopperConfig() {
            write(ADDRESS_CHOPCONF, chopper_config_.bytes);
        }

        static uint32_t readChopperConfigBytes() {
            return read(ADDRESS_CHOPCONF);
        }

        static void writeStoredPwmConfig() {
            write(ADDRESS_PWMCONF, pwm_config_.bytes);
        }

        static uint32_t readPwmConfigBytes() {
            return read(ADDRESS_PWMCONF);
        }

        static uint32_t constrain_(uint32_t value, uint32_t low, uint32_t high) {
            return ((value)<(low)?(low):((value)>(high)?(high):(value)));
        }

        template<typename T>
        static T map(T x, T in_min, T in_max, T out_min, T out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }
    };

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::GlobalConfig tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::global_config_;

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::DriverCurrent tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::driver_current_;

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::CoolConfig tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::cool_config_;

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline bool tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::cool_step_enabled_;

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::ChopperConfig tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::chopper_config_;

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline uint8_t tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::toff_ = tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::TOFF_DEFAULT;

    template<typename _uart, typename _tx_pin, uint8_t _serial_address, typename _rx_pin, typename _en_pin>
    inline tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::PwmConfig tmc2209<_uart, _tx_pin, _serial_address, _rx_pin, _en_pin>::pwm_config_;
}
#endif //! ZHELE_DRIVERS_TMC2209_H_
