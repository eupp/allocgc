In this test garbage collector woudn't be run, because few memory will be allocate.
If you want to run gc on this test, open gc_new.h lockated in ROOT_RIR/sources, add change constant 50000000 to 0
in string (now it is 77) "if (counter > 50000000 && nesting_level == 0) {".
