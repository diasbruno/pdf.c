#ifndef __PDF_C_H__
#define __PDF_C_H__ 1

#include <stdint.h>

#define TAG ((uint64_t)0x00FFFFFFFFFF)
#define UNTAG ((uint64_t)0x7F0000000000)
#define PDF_ELEMENT_TAG ((pdf_element_t*)TAG)
#define PDF_ELEMENT_NULL ((pdf_element_t*)(TAG | ((uint64_t)(PDF_NULL << 8))))

enum pdf_type_t {
  PDF_UNKNOWN    = 0x0,
  PDF_NULL       = 0x1,
  PDF_FALSE      = 0x2,
  PDF_TRUE       = 0x3,
  PDF_NUMBER     = 0x4,
  PDF_STRING     = 0x5,
  PDF_HEX_STRING = 0x6,
  PDF_NAME       = 0x7,
  PDF_STREAM     = 0x8,
  PDF_OBJECT     = 0x9,
  PDF_ARRAY      = 0xA };

typedef struct { int type; char* str; } pdf_element_t;
typedef struct { pdf_element_t* content; pdf_element_t* next; long count; char* name; } pdf_t;
typedef struct { const char* content; size_t cursor; size_t size; } pdf_parser_t;

pdf_parser_t*
pdf_parser_new(const char*);

void
pdf_parser_free(pdf_parser_t*);

const char*
pdf_parser_current(pdf_parser_t*);

int
pdf_parser_peek(pdf_parser_t*);

int
pdf_parser_walk(pdf_parser_t*, int);

int
pdf_parser_next(pdf_parser_t*);

int
pdf_parser_back(pdf_parser_t*);

void
pdf_parse(pdf_parser_t*);

pdf_t*
pdf_create(const char* name);

void
pdf_free(pdf_t* pdf);

#endif // __PDF_C_H__
