#ifndef __BITVM_REFTYPES_H
#define __BITVM_REFTYPES_H

#include <stdio.h>
#include <string.h>
#include <vector>

#ifdef DEBUG_MEMLEAKS
#include <set>
#endif

namespace bitvm {

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

        void ref()
        {
            check(refcnt > 0, ERR_REF_DELETED);
            refcnt++;
        }

        void unref()
        {
            //printf("DELOBJ: %p %d\n", this, refcnt);
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
            //printf("INCR %x\n", e);
            ((RefObject*)e)->ref();
        }
    }

    inline void decr(uint32_t e)
    {
        if (e)
            ((RefObject*)e)->unref();
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
            printf("RefString %p %s\n", this, data);
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
            printf("RefStruct %p\n", this);
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
            printf("RefCollection %p size=%d\n", this, data.size());
        }
    };

    class RefRefCollection
        : public RefObject
    {
    public:
        std::vector<RefObject*> data;
        virtual ~RefRefCollection()
        {
            for (uint32_t i = 0; i < data.size(); ++i) {
                if (data[i])
                    data[i]->unref();
                data[i] = NULL;
            }
            data.resize(0);
        }

        virtual void print()
        {
            printf("RefRefCollection %p size=%d\n", this, data.size());
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
            printf("RefRecord %p size=%d (%d refs)\n", this, len, reflen);
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

    class RefAction
        : public RefObject
    {
    public:
        uint8_t len;
        uint8_t reflen;
        uint32_t startptr;
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
            printf("RefAction %p pc=0x%lx size=%d (%d refs)\n", this, startptr, len, reflen);
        }

        inline void st(int idx, uint32_t v)
        {
            //printf("ST [%d] = %d ", idx, v); this->print();
            check(0 <= idx && idx < len, ERR_OUT_OF_BOUNDS, 10);
            check(fields[idx] == 0, ERR_OUT_OF_BOUNDS, 11); // only one assignment permitted
            fields[idx] = v;
        }

        uint32_t run()
        {
            return exec_function(bytecode + this->startptr, this->fields);
        }
    };

    namespace string {
        RefString *fromLiteral(const char *p);
    }
}
#endif
// vim: ts=4 sw=4
