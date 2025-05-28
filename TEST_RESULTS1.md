VAR 1 - MPI

--TEST RESULTS--

for:

bigfile.txt:

=== FINAL RESULTS ===
Total lines: 17827
Total words: 499128
Total characters: 1354782
Total vowels: 178260
Processing time: 0.0145 seconds
Processes used: 8

bigfile2.txt:

=== FINAL RESULTS ===
Total lines: 4976
Total words: 139328
Total characters: 378178
Total vowels: 49760
Processing time: 0.0040 seconds
Processes used: 8

bigfile3.txt:


=== FINAL RESULTS ===
Total lines: 12131
Total words: 339640
Total characters: 921886
Total vowels: 121300
Processing time: 0.0101 seconds
Processes used: 8

giantfile.txt (1 GB)


=== FINAL RESULTS ===
Total lines: 231116
Total words: 2311150
Total characters: 11324637
Total vowels: 3004495
Processing time: 0.1240 seconds
Processes used: 8


VAR 2 - STL

=== FINAL RESULTS === (~1MB)
Total lines: 17827
Total words: 499128
Total characters: 1354782
Total vowels: 178260
Threads used: 12

=== FINAL RESULTS (1GB)
Total lines: 231116
Total words: 2311156
Total characters: 11324637
Total vowels: 3004495
Threads used: 12
