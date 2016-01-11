#ifndef __BITVM_H
#define __BITVM_H

// #define DEBUG_MEMLEAKS 1

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "MicroBitCustomConfig.h"
#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"
#define printf(...) uBit.serial.printf(__VA_ARGS__)
// #define printf(...)

// for marking glue functions
#define GLUE /*glue*/

#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdint.h>

#ifdef DEBUG_MEMLEAKS
#include <set>
#endif

namespace bitvm {

  typedef enum {
    ERR_INVALID_BINARY_HEADER = 5,
    ERR_OUT_OF_BOUNDS = 8,
    ERR_REF_DELETED = 7,
    ERR_SIZE = 9,
  } ERROR;

  extern uint32_t *globals;
  extern int numGlobals;


  inline void die() { uBit.panic(42); }

  void error(ERROR code, int subcode = 0);

  inline void check(int cond, ERROR code, int subcode = 0)
  {
    if (!cond) error(code, subcode);
  }

  void exec_binary(uint16_t *pc);

  extern const uint32_t functionsAndBytecode[];
  extern uint16_t *bytecode;


#ifdef DEBUG_MEMLEAKS
  class RefObject;
  extern std::set<RefObject*> allptrs;
  void debugMemLeaks();
#endif

  // A base abstract class for ref-counted objects.
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

    // Call to disable pointer tracking on the current instance. Currently used
    // by string literals.
    void canLeak()
    {
#ifdef DEBUG_MEMLEAKS
      allptrs.erase(this);
#endif
    }

    // Increment/decrement the ref-count. Decrementing to zero deletes the current object.
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

    // This is used by index_of function, overridden in RefString
    virtual bool equals(RefObject *other)
    {
      return this == other;
    }
  };

  // Checks if object has a VTable, or if its RefCounted* from the runtime.
  // XXX 'inline' needs to be on separate line for embedding script
  inline
  bool hasVTable(uint32_t e)
  {
    return (*((uint32_t*)e) & 1) == 0;
  }

  // The standard calling convention is:
  //   - when a pointer is loaded from a local/global/field etc, and incr()ed
  //     (in other words, its presence on stack counts as a reference)
  //   - after a function call, all pointers are popped off the stack and decr()ed
  // This does not apply to the RefRecord and st/ld(ref) methods - they unref()
  // the RefRecord* this.
  inline
  void incr(uint32_t e)
  {
    if (e) {
      if (hasVTable(e))
        ((RefObject*)e)->ref();
      else
        ((RefCounted*)e)->incr();
    }
  }

  inline 
  void decr(uint32_t e)
  {
    if (e) {
      if (hasVTable(e))
        ((RefObject*)e)->unref();
      else
        ((RefCounted*)e)->decr();
    }
  }

  // Ref-counted wrapper around any C++ object.
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

  // A ref-counted collection of either primitive or ref-counted objects (String, Image,
  // user-defined record, another collection)
  class RefCollection
    : public RefObject
  {
  public:
    // 1 - collection of refs (need decr)
    // 2 - collection of strings (in fact we always have 3, never 2 alone)
    uint16_t flags;
    std::vector<uint32_t> data;

    RefCollection(uint16_t f)
    {
      flags = f;
    }

    virtual ~RefCollection()
    {
      // printf("KILL "); this->print();
      if (flags & 1)
        for (uint32_t i = 0; i < data.size(); ++i) {
          decr(data[i]);
          data[i] = 0;
        }
      data.resize(0);
    }

    virtual void print()
    {
      printf("RefCollection %p r=%d flags=%d size=%d [%p, ...]\n", this, refcnt, flags, data.size(), data.size() > 0 ? data[0] : 0);
    }
  };

  // A ref-counted byte buffer
  class RefBuffer
    : public RefObject
  {
  public:
    std::vector<uint8_t> data;

    virtual ~RefBuffer()
    {
      data.resize(0);
    }

    virtual void print()
    {
      printf("RefBuffer %p r=%d size=%d [%p, ...]\n", this, refcnt, data.size(), data.size() > 0 ? data[0] : 0);
    }
  };

  // A ref-counted, user-defined Touch Develop object.
  class RefRecord
    : public RefObject
  {
  public:
    // Total number of fields.
    uint8_t len; 
    // Number of fields which are ref-counted pointers; these always come first
    // on the fields[] array.
    uint8_t reflen; 
    // The object is allocated, so that there is space at the end for the fields.
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
      return fields[idx];
    }

    inline uint32_t ldref(int idx)
    {
      //printf("LD %p len=%d reflen=%d idx=%d\n", this, len, reflen, idx);
      check(0 <= idx && idx < reflen, ERR_OUT_OF_BOUNDS, 2);
      uint32_t tmp = fields[idx];
      incr(tmp);
      return tmp;
    }

    inline void st(int idx, uint32_t v)
    {
      check(reflen <= idx && idx < len, ERR_OUT_OF_BOUNDS, 3);
      fields[idx] = v;
    }

    inline void stref(int idx, uint32_t v)
    {
      //printf("ST %p len=%d reflen=%d idx=%d\n", this, len, reflen, idx);
      check(0 <= idx && idx < reflen, ERR_OUT_OF_BOUNDS, 4);
      decr(fields[idx]);
      fields[idx] = v;
    }
  };

  class RefAction;
  typedef uint32_t (*ActionCB)(RefAction *, uint32_t *, uint32_t arg);

  // Ref-counted function pointer. It's currently always a ()=>void procedure pointer.
  class RefAction
    : public RefObject
  {
  public:
    // This is the same as for RefRecord.
    uint8_t len;
    uint8_t reflen;
    ActionCB func; // The function pointer
    uint32_t fields[];

    // fields[] contain captured locals
    virtual ~RefAction()
    {
      for (int i = 0; i < this->reflen; ++i) {
        decr(fields[i]);
        fields[i] = 0;
      }
    }

    virtual void print()
    {
      printf("RefAction %p r=%d pc=0x%lx size=%d (%d refs)\n", this, refcnt, (const uint8_t*)func - (const uint8_t*)bytecode, len, reflen);
    }

    inline void st(int idx, uint32_t v)
    {
      //printf("ST [%d] = %d ", idx, v); this->print();
      check(0 <= idx && idx < len, ERR_OUT_OF_BOUNDS, 10);
      check(fields[idx] == 0, ERR_OUT_OF_BOUNDS, 11); // only one assignment permitted
      fields[idx] = v;
    }

    inline uint32_t run(int arg)
    {
      this->ref();
      uint32_t r = this->func(this, &this->fields[0], arg);
      this->unref();
      return r;
    }
  };

  // These two are used to represent locals written from inside inline functions
  class RefLocal
    : public RefObject
  {
  public:
    uint32_t v;

    virtual void print()
    {
      printf("RefLocal %p r=%d v=%d\n", this, refcnt, v);
    }

    RefLocal() : v(0) {}
  };

  class RefRefLocal
    : public RefObject
  {
  public:
    uint32_t v;

    virtual void print()
    {
      printf("RefRefLocal %p r=%d v=%p\n", this, refcnt, (void*)v);
    }

    RefRefLocal() : v(0) {}

    virtual ~RefRefLocal()
    {
      decr(v);
    }
  };
}

#endif

/* mbed functions to import. The pointer-table generation script will pick these up. */
#if POINTER_GENERATOR_DOESNT_REALLY_DO_IFDEFS
void wait_us(int us);
#endif

// vim: ts=2 sw=2 expandtab
