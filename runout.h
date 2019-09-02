/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 断料检测监测时间
 */

/**
 * runout.h - Runout sensor support
 */

#ifndef _RUNOUT_H_       
#define _RUNOUT_H_

#include "cardreader.h"
#include "printcounter.h"
#include "stepper.h"
#include "Marlin.h"

#include "MarlinConfig.h"

#define FIL_RUNOUT_THRESHOLD 200
#define  RUNOUT_CHECK_PRERIOD 100

class FilamentRunoutSensor {
  public:
    FilamentRunoutSensor() {}

    static void setup();
	
	FORCE_INLINE static void reset() {
		runout_count = 0; filament_ran_out = false; LAST_RO_TIME = 0;
	}

    FORCE_INLINE static void run() {
	  if(check()){
		  if ((IS_SD_PRINTING || print_job_timer.isRunning()) && !filament_ran_out) {
			filament_ran_out = true;
			enqueue_and_echo_commands_P(PSTR(FILAMENT_RUNOUT_SCRIPT));
			planner.synchronize();
		  }
		  notify_host_filament_runout();
	  }  
    }
  private:
    static bool filament_ran_out;
    static uint8_t runout_count;
	  static long LAST_RO_TIME;
	
	  static long LAST_RO_Notify_TIME;
    
	FORCE_INLINE static void notify_host_filament_runout(){
	  //notify every 2 second
	  if(millis() - LAST_RO_Notify_TIME > 2000){
		LAST_RO_Notify_TIME = millis();
		SERIAL_ERROR_START();
		SERIAL_ERRORLNPGM(MSG_FILAMENTCHANGE);
		SERIAL_EOL();
	  }
	}

    FORCE_INLINE static bool check() {
      #if NUM_RUNOUT_SENSORS < 2
        // A single sensor applying to all extruders
        const bool is_out = READ(FIL_RUNOUT_PIN) == FIL_RUNOUT_INVERTING;
      #else
        // Read the sensor for the active extruder
        bool is_out;
        switch (active_extruder) {
          case 0: is_out = READ(FIL_RUNOUT_PIN) == FIL_RUNOUT_INVERTING; break;
          case 1: is_out = READ(FIL_RUNOUT2_PIN) == FIL_RUNOUT_INVERTING; break;
          #if NUM_RUNOUT_SENSORS > 2
            case 2: is_out = READ(FIL_RUNOUT3_PIN) == FIL_RUNOUT_INVERTING; break;
            #if NUM_RUNOUT_SENSORS > 3
              case 3: is_out = READ(FIL_RUNOUT4_PIN) == FIL_RUNOUT_INVERTING; break;
              #if NUM_RUNOUT_SENSORS > 4
                case 4: is_out = READ(FIL_RUNOUT5_PIN) == FIL_RUNOUT_INVERTING; break;
              #endif
            #endif
          #endif
        }
      #endif

		if (is_out)
		{

			if (LAST_RO_TIME < millis())
			{
				LAST_RO_TIME = millis() + RUNOUT_CHECK_PRERIOD;
				runout_count++;

			}
			if (runout_count > FIL_RUNOUT_THRESHOLD)return true;

		}
		else
		{
			runout_count = 0;
		}
	

    //  return (is_out ? ++runout_count : (runout_count = 0)) > FIL_RUNOUT_THRESHOLD;

		return false;
    }
};

extern FilamentRunoutSensor runout;

#endif // _RUNOUT_H_
