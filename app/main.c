#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fs/fs.h"
#include "pdf.h"

int main(int count, char* args[]) {
  pdf_parser_t* parser = NULL;
  int rc = 0;

  if (fs_exists(args[1]) != 0) {
    perror("cannot open file.\n");
    rc = 1;
    goto end;
  }

  // size_t size = fs_size(args[1]);
  char* content = fs_read(args[1]);
  parser = pdf_parser_new(content);
  pdf_parse(parser);

  free((char*)parser->content);
  pdf_parser_free(parser);

 end:
  return rc;
}
