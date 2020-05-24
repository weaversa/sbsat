#ifndef system_h_included
#define system_h_included

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>



enum
{
  unknown = 2
};


/* bool, true, false, unknown are defined at this point */

/**
 * Any of ::false, ::true, or ::unknown.
 */
typedef uint8_t mbool;

#define int2bool(i) ((i) ? true : false)
#define bool2int(b) ((b) ? 1 : 0)
#define mbool2int(b) ((b) == true ? 1 : ((b) == false ? 0 : 2))

typedef intmax_t  literal;
typedef uintmax_t variable;
#define LITERAL_MAX  ((literal) INTMAX_MAX)
#define VARIABLE_MAX ((variable) UINTMAX_MAX)


#define Max(x,y) ((x) > (y) ? (x) : (y))
#define Min(x,y) ((x) < (y) ? (x) : (y))


#endif
