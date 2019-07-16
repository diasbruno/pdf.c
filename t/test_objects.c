#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pdf.h"
#include "describe/describe.h"


int main(void) {
  describe("parser") {
    it("should parse null") {
      assert(12 == 12);
    }
  }
  return assert_failures();
}
