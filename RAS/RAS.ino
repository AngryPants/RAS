// Include RAS Files
#include "RAS.h"

void setup() {
  RAS::Begin();
}

void loop() {
  RAS::Update();
}
