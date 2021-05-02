#ifndef _UTIL_H
 #define _UTIL_H

#include <hiredis/hiredis.h>
#include <string>
#include <vector>

using namespace std;


int splitc(char *string, char *fields[], int  nfields, char sep );

class Redis {
    public:
        Redis(const char *addr, const char *pswd);
        ~Redis();
        const char *getLocalAddr() { return local_addr; }

        int setKey(const char *key, const char *val);
        string getKey(const char *key);
        bool isOk();

    private:
        redisContext *c;
        char local_addr[64];
};

#endif // _UTIL_H

