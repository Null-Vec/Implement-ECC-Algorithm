Compile the server program:$ gcc -maes -msse2 -mpclmul -o server server.c -lcrypto
Execution: $ ./server   
N.B.  SERVER_IP "127.0.0.1" PORT: 8888

Compile the server program:$ gcc -maes -msse2 -mpclmul -o client client.c -lcrypto
Execution: $ ./client

-maes:    Enables the use of AES (Advanced Encryption Standard) instructions provided by the CPU.
-msse2:   Enables the use of SSE2 (Streaming SIMD Extensions 2) instructions.
-mpclmul: Enables the use of PCLMULQDQ (Carry-Less Multiplication) instruction.
-lcrypto: Links the program with the OpenSSL crypto library.
