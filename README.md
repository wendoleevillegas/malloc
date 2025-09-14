Description <br>
In my CSE3320 course, I built myown implementation of malloc and free. That is, you will need to implement a library that interacts with the operating system to perform heap management. Additionally, I performed a comparison between using generative AI (ChatGPT) 
to determine its competency when working with low level languages such as C/C++. The findings from this research are located in the OS Malloc Report.pdf.

Building and Running the Code <br>
The code compiles into four shared libraries and six test programs. To build the code, change to your top level assignment directory and type: <br>
<br>
make 
<br> <br>
Once you have the library, you can use it to override the existing malloc by using LD_PRELOAD. The following example shows running the ffnf test using the First Fit shared library: <br> <br>

$ env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf <br> <br>
To run the other heap management schemes replace libmalloc-ff.so with the appropriate library: <br> <br>

Best-Fit: libmalloc-bf.so
First-Fit: libmalloc-ff.so
Next-Fit: libmalloc-nf.so
Worst-Fit: libmalloc-wf.so
<br> <br>
Using the framework of malloc and free provided on the course github repository:
Implement splitting and coalescing of free blocks. If two free blocks are adjacent then combine them. If a free block is larger than the requested size then split the block into two.
Implement three additional heap management strategies: Next Fit, Worst Fit, Best Fit (First Fit has already been implemented for you).
Counters exist in the code for tracking of the following events:
Number of times the user calls malloc successfully
Number of times the user calls free successfully
Number of times we reuse an existing block
Number of times we request a new block
Number of times we split a block
Number of times we coalesce blocks
Number blocks in free list
Total amount of memory requested
Maximum size of the heap
<br> <br>
The code will print the statistics ( THESE STATS ARE FAKE) upon exit and should look like similar to:
<br> <br>
mallocs: 8 <br>
frees: 8 <br>
reuses: 1 <br>
grows: 5 <br>
splits: 1 <br>
coalesces: 1 <br>
blocks: 5 <br>
requested: 7298 <br>
max heap: 4096 <br>
