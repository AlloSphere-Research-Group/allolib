
#include "gtest/gtest.h"

#include "al/ui/al_ParameterServer.hpp"

#include <fstream>

TEST(ParameterSever, Handshake) {
  al::ParameterServer s;
  s.startHandshakeServer();
  al::al_sleep(0.05);
  al::ParameterServer c("", 9011);
  c.startCommandListener("");
  // FIXME should work without this wait
  al::al_sleep(0.3);

  al::Parameter p{"test", "", 0.5, 0.0, 1.0};
  al::Parameter p2{"test", "", 0.5, 0.0, 1.0};

  s << p << p2;
  c << p << p2;

  p.set(0.1f);
  al::al_sleep(0.1);
  EXPECT_DOUBLE_EQ(p2.get(), 0.1f);
  p2.set(0.3f);
  al::al_sleep(0.1);
  EXPECT_DOUBLE_EQ(p.get(), 0.3f);

  c.stopServer();
  s.stopServer();
}
