# Memory-System

Created a memory system framework for the heap layout resulting in a 2x faster memory system compared to built-in memory systems. This project was completed using original template structure<sup>1</sup> from a class project (Optimize C++).
I successfully created and executed both the logic and its corresponding implementation. Leveraging my code, the project successfully navigated a rigorous stress test involving more than 100 randomized memory allocation and de-allocation operations, affirming its robust performance under challenging conditions.

__Techniques__
* Overloaded New/Delete operators for heap assignment and alignment specification.
* Added allocators (implemented with next fit algorithm) and de-allocators for the memory system.
* Implemented memory flags, and headers to identify each allocated memory block.


## Credit
1 - Copyright 2022, Ed Keenan, all rights reserved.
