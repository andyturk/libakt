#pragma once

#ifndef assert
#ifndef NDEBUG
#define assert(x) do {if(!(x)) for(;;);} while (0)
#else
#define assert(x) do {} while (0)
#endif
#endif

