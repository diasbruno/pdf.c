//
// describe.c
//
// Copyright (c) 2016 Stephen Mathieson
// MIT licensed
//

#include "describe.h"
#include "console-colors/console-colors.h"

int
__before_specification(int current) {
  return current;
}

void
__after_specification(int before, int current, const char *specification) {
  int failed = before == current;
  cc_fprintf(
      CC_FG_WHITE
    , stdout
    , "    %4s"
    , failed ? "ok" : "fail"
  );
  cc_fprintf(CC_FG_GRAY, stdout, " %s\n", specification);
}
