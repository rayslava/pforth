\ Core FORTH words

: nip   ( a b -- b ) swap drop ;
: tuck  ( a b -- b a b ) swap over ;

: 2dup  ( a b -- a b a b ) over over ;
: 2drop ( a b -- ) drop drop ;
