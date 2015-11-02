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

  const int V3BINARY = 0x4204;

  extern uint32_t *globals;
  extern uint32_t *strings;
  extern int numGlobals, numStrings;


  inline void die() { uBit.panic(42); }

  void error(ERROR code, int subcode = 0);

  inline void check(int cond, ERROR code, int subcode = 0)
  {
    if (!cond) error(code, subcode);
  }

  void exec_binary(uint16_t *pc);

  extern const uint32_t functionsAndBytecode[16000];


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

  // The standard calling convention is:
  //   - when a pointer is loaded from a local/global/field etc, and incr()ed
  //     (in other words, its presence on stack counts as a reference)
  //   - after a function call, all pointers are popped off the stack and decr()ed
  // This does not apply to the RefRecord and st/ld(ref) methods - they unref()
  // the RefRecord* this.
  // XXX 'inline' needs to be on separate line for embedding script
  inline
  void incr(uint32_t e)
  {
    if (e) {
      ((RefObject*)e)->ref();
    }
  }

  inline 
  void decr(uint32_t e)
  {
    if (e) {
      ((RefObject*)e)->unref();
    }
  }

  // Ref-counted string. In future we should be able to just wrap ManagedString
  class RefString
    : public RefObject
  {
  public:
    ManagedString v;

    RefString(const ManagedString& i) : v(i) {}
    RefString(const char *ptr) : v(ManagedString(ptr)) {}
    RefString(const char *ptr, const int size) : v(ManagedString(ptr, size)) {}

    virtual ~RefString()
    {
    }

    virtual bool equals(RefObject *other_)
    {
      RefString *other = (RefString*)other_;
      return this->v == other->v;
    }

    virtual void print()
    {
      printf("RefString %p r=%d, %s\n", this, refcnt, v.toCharArray());
    }

    inline int charAt(int index) { return v.charAt(index); }
    inline int length() { return v.length(); }
    inline const char *toCharArray() { return v.toCharArray(); }
  };


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

  // A ref-counted collection of primitive objects (Number, Boolean)
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

  // A ref-counted collection of ref-counted objects (String, Image,
  // user-defined record, another collection)
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

    // All of the functions below unref() self. This is for performance reasons -
    // the code emitter will not emit the unrefs for them.
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
      printf("RefAction %p r=%d pc=0x%lx size=%d (%d refs)\n", this, refcnt, (const uint8_t*)func - (const uint8_t*)functionsAndBytecode, len, reflen);
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

// vim: ts=2 sw=2 expandtab
