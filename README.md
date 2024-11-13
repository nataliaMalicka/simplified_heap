# Simplified Heap
 A simulated memory allocator that deals with fragmentation and efficient allocation. The header and footer store the byte size of the payload plus meta data - 1 indicates that a block is allocated and 0 that it is free. A call to malloc finds the first free block that can store the requested bytes. Empty blocks are coalesced. 

 Completed as part of a course in Computer Information Systems. 
