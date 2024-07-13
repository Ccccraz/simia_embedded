#include <Arduino.h>
#include "listener.h"
#include "event_center.h"

simia::Listener listener{9600};

void start(std::shared_ptr<void> pvPtr){
  Serial.println("start");
}

void set_speed(std::shared_ptr<void> args){
  auto origin = std::static_pointer_cast<uint8_t>(args);
  Serial.print("Speed: ");
  Serial.println(*origin, HEX);
}

void setup()
{
  Serial.println("start");
  simia::EventCenter::instance().regsiter(simia::CMD::Start, start);
  simia::EventCenter::instance().regsiter(simia::CMD::SetSpeed, set_speed);
}

void loop()
{
  listener.listen();
}