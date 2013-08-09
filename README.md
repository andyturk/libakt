libakt
======

This is a library of general purpose C++ code intended to be used in an embedded environment, often with [ChibiOS/RT](http://www.chibios.org/dokuwiki/doku.php).

The directory structure of the source follows the namespace structure of the code itself. For GCC, make the repository available to the compiler with `-I blah/blah/libakt`, then use the namespace structure in the #include directive. For example, to use the akt::Ring template, `#include "akt/ring.h"`