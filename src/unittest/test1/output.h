// File: output.h

#include <unistd.h>
#include <cstdio>

#define DO_OUTPUT( arg1, arg2, arg3 )          \
   if ( arg1 <= output::getOutputType(arg2) )  \
      output::doOutput(arg1, arg2, (int)getpid());

class output
{
   public:

      static int getOutputType(int arg)
      {
         arg = 0;
         return 1;
      };

      static void doOutput(int arg1, int arg2, int arg3)
      {
         printf("%d %d %d", arg1, arg2, arg3);
      };
};
