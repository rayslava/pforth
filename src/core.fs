\ Core FORTH words

: nip   ( a b -- b ) swap drop ;
: tuck  ( a b -- b a b ) swap over ;

: 2dup  ( a b -- a b a b ) over over ;
: 2drop ( a b -- ) drop drop ;

: CR 10 EMIT ;

: 1+ 1 + ;
: 1- 1 - ;
: 2+ 2 + ;
: 2- 2 - ;
: 2/ 2 / ;
: 0< 0 < ;
: 0= 0 == ;
: 0> 0 > ;
: = == ;
