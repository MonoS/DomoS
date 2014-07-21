#include "DomoS.h"
#include <Serial.h>
#include <EEPROM.h>

void setup()
{
  DomoS DomoS;

  while(DomoS.IsOn())
  {
    DomoS.Work();
  }
}

void loop() {
  delay(1000);
}



