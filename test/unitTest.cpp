/*
The OpenTRV project licenses this file to you
under the Apache Licence, Version 2.0 (the "Licence");
you may not use this file except in compliance
with the Licence. You may obtain a copy of the Licence at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing,
software distributed under the Licence is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the Licence for the
specific language governing permissions and limitations
under the Licence.
Author(s) / Copyright (s): Deniz Erbilgin 2015
*/

#include "unitTest.h"

/**Report an error from a unit test on Serial, and repeat so that it is not missed. */
void error(int expected, int actual, int line)
  {
  for( ; ; )
    {
    Serial.print(F("***Test FAILED*** expected=\t0x"));
    Serial.print(expected, HEX);
    Serial.print(F("actual=\t0x"));
    Serial.print(actual, HEX);
    if(0 != line)
      {
      Serial.print(F(" at line "));
      Serial.print(line);
      }
    Serial.println();
//    LED_HEATCALL_ON();
//    tinyPause();
//    LED_HEATCALL_OFF();
//    sleepLowPowerMs(1000);
    delay(1000);
    }
  }
  
/*
void testLibVersion()
  {
  Serial.println("LibVersion");
  AssertIsEqual(0, ARDUINO_LIB_AESGCM_VERSION_MAJOR);
  AssertIsEqual(2, ARDUINO_LIB_AESGCM_VERSION_MINOR);
  }
*/
