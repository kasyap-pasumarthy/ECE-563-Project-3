Dynamic Instruction Scheduler

In this project I made an out of order superscalar pipeline following Tomasulo's algorithm. The included traces were used to run the simulator and check against the validation runs. The report was only made to satisfy the grading requirements and as such does not go into detail regarding the simulator itself.

I also attempted to model a data cache linked to the functional units but was unsuccessful in doing so. 

To run the simulator, type 'make' in console to run the Makefile and compile the code. Once that's done, give command line inputs as follows:

sim (S) (N) (BLOCKSIZE) (L1_size) (L1_ASSOC) (L2_SIZE) (L2_ASSOC) (tracefile)
 S: Scheduling Queue size.
 N: Peak fetch, dispatch, and issue rate.
 BLOCKSIZE: Positive integer. Block size in bytes. (Same block size for all caches in the memory hierarchy.)
 L1_SIZE: Positive integer. L1 cache size in bytes.
 L1_ASSOC: Positive integer. L1 set-associativity (1 is direct-mapped).
 L2_SIZE: Positive integer. L2 cache size in bytes. L2_SIZE = 0 signifies that there is no L2 cache.
 L2_ASSOC: Positive integer. L2 set-associativity (1 is direct-mapped).
 tracefile: is the filename of the input trace.
Note:
BLOCKSIZE, L1_SIZE, L1_ASSOC, L2_SIZE, L2_ASSOC are 0 if no data caches are modeled.

(<tracefile> is the filename of the input trace.) 

The simulator outputs everything to console.