#ifndef __BITVM_H
#define __BITVM_H

// #define DEBUG_MEMLEAKS 1

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"
#define printf(...) uBit.serial.printf(__VA_ARGS__)
// #define printf(...)

#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdint.h>

#ifdef DEBUG_MEMLEAKS
#include <set>
#endif

namespace bitvm {

  typedef enum {
    ERR_INVALID_FUNCTION_HEADER = 4,
    ERR_INVALID_BINARY_HEADER = 5,
    ERR_STACK_ONRETURN = 6,
    ERR_REF_DELETED = 7,
    ERR_OUT_OF_BOUNDS = 8,
    ERR_SIZE = 9,
  } ERROR;

  const int V2BINARY = 0x4203;

  extern uint32_t *globals;
  extern uint32_t *strings;
  extern int numGlobals, numStrings;


  inline void die() { uBit.panic(42); }

  void error(ERROR code, int subcode = 0);

  inline void check(int cond, ERROR code, int subcode = 0)
  {
    if (!cond) error(code, subcode);
  }

  uint32_t exec_function(const uint16_t *pc, uint32_t *args);
  int exec_binary();


  extern void * const functions[];
  extern const int enums[];

  extern const unsigned short bytecode[];


#ifdef DEBUG_MEMLEAKS
  class RefObject;
  extern std::set<RefObject*> allptrs;
  void debugMemLeaks();
#endif


  class RefObject
  {
  public:
    uint16_t refcnt;

    RefObject()
    {
      refcnt = 1;
#ifdef DEBUG_MEMLEAKS
      allptrs.insert(this);
#endif
    }

    void canLeak()
    {
#ifdef DEBUG_MEMLEAKS
      allptrs.erase(this);
#endif
    }

    inline void ref()
    {
      check(refcnt > 0, ERR_REF_DELETED);
      //printf("INCR "); this->print();
      refcnt++;
    }

    inline void unref()
    {
      //printf("DECR "); this->print();
      if (--refcnt == 0) {
        delete this;
      }
    }

    virtual void print()
    {
      printf("RefObject %p\n", this);
    }

    virtual ~RefObject()
    {
      // This is just a base class for ref-counted objects.
      // There is nothing to free yet, but derived classes will have things to free.
#ifdef DEBUG_MEMLEAKS
      allptrs.erase(this);
#endif
    }

    virtual bool equals(RefObject *other)
    {
      return this == other;
    }
  };

  inline void incr(uint32_t e)
  {
    if (e) {
      ((RefObject*)e)->ref();
    }
  }

  inline void decr(uint32_t e)
  {
    if (e) {
      ((RefObject*)e)->unref();
    }
  }

  class RefString
    : public RefObject
  {
  public:
    uint16_t len;
    char *data;

    virtual ~RefString()
    {
      char *tmp = data;
      //printf("DEL: %s\n", tmp);
      data = NULL;
      delete tmp;
    }

    virtual bool equals(RefObject *other_)
    {
      RefString *other = (RefString*)other_;
      return this->len == other->len && memcmp(this->data, other->data, this->len) == 0;
    }

    virtual void print()
    {
      printf("RefString %p r=%d, %s\n", this, refcnt, data);
    }

    int charAt(int index)
    {
      return (index >=0 && index < this->len) ? this->data[index] : 0;
    }
  };


  template <class T>
  class RefStruct
    : public RefObject
  {
  public:
    T v;

    virtual ~RefStruct()
    {
    }

    virtual void print()
    {
      printf("RefStruct %p r=%d\n", this, refcnt);
    }

    RefStruct(const T& i) : v(i) {}
  };

  class RefCollection
    : public RefObject
  {
  public:
    std::vector<uint32_t> data;

    virtual void print()
    {
      printf("RefCollection %p r=%d size=%d\n", this, refcnt, data.size());
    }
  };

  class RefRefCollection
    : public RefObject
  {
  public:
    std::vector<RefObject*> data;
    virtual ~RefRefCollection()
    {
      // printf("KILL "); this->print();
      for (uint32_t i = 0; i < data.size(); ++i) {
        if (data[i])
          data[i]->unref();
        data[i] = NULL;
      }
      data.resize(0);
    }

    virtual void print()
    {
      printf("RefRefCollection %p r=%d size=%d [%p, ...]\n", this, refcnt, data.size(), data.size() > 0 ? data[0] : NULL);
    }
  };

  class RefRecord
    : public RefObject
  {
  public:
    uint8_t len;
    uint8_t reflen;
    uint32_t fields[];

    virtual ~RefRecord()
    {
      //printf("DELREC: %p\n", this);
      for (int i = 0; i < this->reflen; ++i) {
        decr(fields[i]);
        fields[i] = 0;
      }
    }

    virtual void print()
    {
      printf("RefRecord %p r=%d size=%d (%d refs)\n", this, refcnt, len, reflen);
    }

    inline uint32_t ld(int idx)
    {
      check(reflen <= idx && idx < len, ERR_OUT_OF_BOUNDS, 1);
      uint32_t tmp = fields[idx];
      unref();
      return tmp;
    }

    inline uint32_t ldref(int idx)
    {
      //printf("LD %p len=%d reflen=%d idx=%d\n", this, len, reflen, idx);
      check(0 <= idx && idx < reflen, ERR_OUT_OF_BOUNDS, 2);
      uint32_t tmp = fields[idx];
      incr(tmp);
      unref();
      return tmp;
    }

    inline void st(int idx, uint32_t v)
    {
      check(reflen <= idx && idx < len, ERR_OUT_OF_BOUNDS, 3);
      fields[idx] = v;
      unref();
    }

    inline void stref(int idx, uint32_t v)
    {
      //printf("ST %p len=%d reflen=%d idx=%d\n", this, len, reflen, idx);
      check(0 <= idx && idx < reflen, ERR_OUT_OF_BOUNDS, 4);
      decr(fields[idx]);
      fields[idx] = v;
      unref();
    }
  };

  class RefAction;
  typedef uint32_t (*ActionCB)(RefAction *, uint32_t *);

  class RefAction
    : public RefObject
  {
  public:
    uint8_t len;
    uint8_t reflen;
    ActionCB func;
    uint32_t fields[];

    virtual ~RefAction()
    {
      for (int i = 0; i < this->reflen; ++i) {
        decr(fields[i]);
        fields[i] = 0;
      }
    }

    virtual void print()
    {
      printf("RefAction %p r=%d pc=0x%lx size=%d (%d refs)\n", this, refcnt, ((const uint16_t*)func - bytecode) * 2, len, reflen);
    }

    inline void st(int idx, uint32_t v)
    {
      //printf("ST [%d] = %d ", idx, v); this->print();
      check(0 <= idx && idx < len, ERR_OUT_OF_BOUNDS, 10);
      check(fields[idx] == 0, ERR_OUT_OF_BOUNDS, 11); // only one assignment permitted
      fields[idx] = v;
    }

    inline uint32_t run()
    {
      this->ref();
      uint32_t r = this->func(this, &this->fields[0]);
      this->unref();
      return r;
    }
  };
}

#endif

// vim: ts=2 sw=2 expandtab
