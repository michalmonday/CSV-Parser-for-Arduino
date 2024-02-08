// This file was created with the purpose of using the CSV_Parser library in non-Arduino projects.
// Thanks to that it can be tested and developed on a regular computer, which may be more convenient.

#ifndef NON_ARDUINO_ADAPTATIONS_H
#define NON_ARDUINO_ADAPTATIONS_H
#ifdef NON_ARDUINO

  #include <string>
  #include <stdio.h>
  #include <stdarg.h>
  #define String std::string

  // define enum for HEX DEC etc. from Arduino.h
  enum {
    DEC = 10,
    HEX = 16,
    OCT = 8,
    BIN = 2
  };

  class Stream {
  public:
    virtual void write(uint8_t) = 0;
    // virtual int available() = 0;
    virtual int read() = 0;
    virtual void print(const char *s) = 0;
    virtual void print(int i) = 0;
    virtual void print(int, int) = 0;
    virtual void println(const char *s) = 0;
    virtual void println(int i, int base) = 0;
    virtual void println() = 0;
    virtual void printf(const char *fmt, ...) = 0;
  };
  class SerialClass : public Stream {
  public:
    void write(uint8_t c) { putchar(c); }
    // int available() { return ; }
    int read() { return getchar(); }
    void print(const char *s) { printf("%s", s); }
    void print(int i) { printf("%c", i); }
    void print(int i, int base) {
      if (base == DEC) {
        printf("%d", i);
      } else if (base == HEX) {
        printf("%x", i);
      } else if (base == OCT) {
        printf("%o", i);
      } else if (base == BIN) {
        printf("%b", i);
      }
    }
    void println(const char *s) { printf("%s\n", s); }
    void println(int i, int base) {
      print(i, base);
      printf("\n");
    }
    void println() { printf("\n"); }
    void printf(const char *fmt, ...) {
      va_list args;
      va_start(args, fmt);
      vprintf(fmt, args);
      va_end(args);
    }
  };

  extern SerialClass Serial;

#endif
#endif