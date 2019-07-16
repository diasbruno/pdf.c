 #define POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "strdup/strdup.h"
#include "logger/logger.h"
#include "pdf.h"

#define nil NULL

static char buff[1024] = {0};

static int is_digit(int c) {
  return (c >= '0' && c <= '9') || c == '.' || c == '-'; }

// static int is_ocatl(int c) {
//   return c >= '0' && c <= '7'; }

static int
is_hex(int c) {
  return (c >= '0' && c <= '9') ||
    (c >= 'a' && c <= 'f') ||
    (c >= 'A' && c <= 'F');
}

static int
is_name_char(int c) {
  return
    0x21 <= c && c <= 0x7e &&
    strchr("()<>[]{}/#", c) == 0;
}

static int
is_space_char(int c) {
  return
    c == '\n' ||
    c == '\r' || c == ' ' ||
    c == '\t';
}

int
pdf_is_space_char(int c) {
  return
    c == '%' ||
    is_space_char(c);
}

static size_t
skip_comment(pdf_parser_t* p) {
  char* walk = (char*)p->content;
  do {
    walk++;
    logger_info("comment", "char %c", *walk);
  } while (*walk != '\n');
  return (walk + 1) - p->content;
}

void
pdf_skip_spaces(pdf_parser_t* p) {
  int c = 0;
  while ((c = pdf_parser_next(p)) && pdf_is_space_char(c));
}

pdf_element_t*
read_object(pdf_parser_t* p);

pdf_element_t*
read_number(pdf_parser_t* p) {
  int c = 0;
  char* b = &buff[0];
  while ((c = *pdf_parser_current(p)) && is_digit(c)) {
    *(b++) = c;
    pdf_parser_next(p);
  }
  *(b++) = '\0';
  logger_info("info", "number %s.", buff);
  return nil;
}

pdf_element_t*
read_string(pdf_parser_t* p) {
  int c = 0;
  char* b = &buff[0];
  while ((c = pdf_parser_next(p)) && c != ')') *(b++) = c;
  *(b++) = ')';
  *(b++) = '\0';
  logger_info("info", "string %s", buff);
  return nil;
}

pdf_element_t*
read_name(pdf_parser_t* p) {
  int c = 0;
  char* b = buff;
  assert(*pdf_parser_current(p) == '/');
  while ((c = pdf_parser_next(p)) && is_name_char(c)) *(b++) = c;
  *(b++) = '\0';
  logger_info("info", "name %s", buff);
  return nil;
}

pdf_element_t*
read_array(pdf_parser_t* p) {
  int c = 0;
  logger_info("info", "Arr {");
  while ((c = pdf_parser_next(p)) && c != ']') read_object(p);
  logger_info("info", "} End Arr");
  return nil;
}

// static void
// read_dict(const char* f) {}

pdf_element_t*
read_stream(pdf_parser_t* p) {
  int c = 0;
  while ((c = pdf_parser_next(p)) && c != '\n');
  logger_info("info", "stream %s", "");
  return nil;
}

pdf_element_t*
read_dict_or_stream(pdf_parser_t* p) {
  int c = 0;
  logger_info("info", "Dict {");
  while ((c = pdf_parser_next(p)) && c != '>') {
    if (c == '\r') {
      printf("meh\n");
    }
    read_object(p);
  }
  pdf_skip_spaces(p);
  c = pdf_parser_next(p);
  if (c == 's') read_stream(p);
  logger_info("info", "} End dict");
  return nil;
}


pdf_element_t*
read_true(pdf_parser_t* p) {
  int st = strncmp(pdf_parser_current(p), "true", 4) == 0;
  if (st) {
    return PDF_ELEMENT_TAG;
  }
  return nil;
}

pdf_element_t*
read_false(pdf_parser_t* p) {
  int st = strncmp(pdf_parser_current(p), "false", 5) == 0;
  if (st) {
    return PDF_ELEMENT_TAG;
  }
  return nil;
}

pdf_element_t*
read_null(pdf_parser_t* p) {
  int st = strncmp(pdf_parser_current(p), "null", 4) == 0;
  if (st) {
    return PDF_ELEMENT_NULL;
  }
  return nil;
}

pdf_element_t*
read_hex_string(pdf_parser_t* p) {
  int c = 0;
  char* b = &buff[0];
  while ((c = pdf_parser_next(p)) && c != '>' && is_hex(c)) {
    *(b++) = c;
  }
  *(b++) = '>';
  *(b++) = '\0';
  logger_info("info", "hex number %s", buff);
  return NULL;
}

pdf_element_t*
read_object(pdf_parser_t* p) {
  pdf_element_t* e = nil;
  memset(buff, 0, 1024);
  int c = *pdf_parser_current(p);
  if (c == 0) {
    return NULL;
  }
  logger_info("read_object", "reading current %c\n", c);
  if (is_digit(c)) {
    e = read_number(p);
  } else {
    switch (c) {
    case '%': { p->cursor = p->cursor + (size_t)skip_comment(p); } break;
    case ' ': { pdf_skip_spaces(p); } break;
    case '(': { e = read_string(p); } break;
    case '/': { e = read_name(p);   } break;
    case 't': { e = read_true(p);   } break; // only possibility is 'true'
    case 'f': { e = read_false(p);  } break; // only possibility is 'false'
    case 'n': { e = read_null(p);   } break; // only possibility is 'null'
    case '[': { e = read_array(p);  } break;
    case 'e': { return nil;         } break; // some kind of empty indirect object
    case '<': {
      // if next is also '<', it's a dictionary or stream
      // else hex string
      (void)pdf_parser_next(p);
      if ((c = pdf_parser_peek(p)) == '<') {
        (void)pdf_parser_next(p);
        e = read_dict_or_stream(p);
      } else {
        e = read_hex_string(p);
      }
    } break;
    }
  }
  return e;
}

pdf_t* pdf_create(const char* name) {
  pdf_t* pdf = malloc(sizeof(pdf_t));
  pdf->name = strdup(name);
  pdf->content = malloc(sizeof(pdf_element_t) * 128);
  pdf->next = pdf->content;
  pdf->count = 128;
  return pdf;
}

void pdf_free(pdf_t* pdf) {
  free(pdf->name);
  free(pdf->content);
  pdf->next = NULL;
}

pdf_parser_t*
pdf_parser_new(const char* content) {
  pdf_parser_t* p = malloc(sizeof(pdf_parser_t));
  if (!p) return NULL;
  p->content = content;
  p->cursor = 0;
  p->size = strlen(content) - 1;
  return p;
}

void
pdf_parser_free(pdf_parser_t* p) { free(p); }

int
pdf_parser_peek(pdf_parser_t* p) {
  int c = p->content[p->cursor + 1];
  logger_info("peek", "char %c", c);
  return c;
}

int
pdf_parser_next(pdf_parser_t* p) {
  int c = p->content[(++p->cursor)];
  logger_info("next", "char %c", c);
  return c;
}

int
pdf_parser_back(pdf_parser_t* p) {
  int c = p->content[(--p->cursor)];
  logger_info("back", "char %c", c);
  return c;
}

const char*
pdf_parser_current(pdf_parser_t* p) {
  return p->content + p->cursor;
}

void pdf_parse(pdf_parser_t* p) {
  while (p->cursor < p->size)
    read_object(p);
}
