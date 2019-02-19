#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <cstring>
#include <iostream>
#include <list>
#include <sstream>


int main(int argc, char **argv) {
  Catch::Session sesssion;
  sesssion.configData().showDurations = Catch::ShowDurations::OrNot::Always;
  int result = sesssion.run(argc, argv);
  return (result < 0xff ? result : 0xff);
}
