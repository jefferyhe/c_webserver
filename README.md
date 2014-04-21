• A two-mode multi-thread web server by extending a simple server framework, which is able to serve massive client requests and generate responses effectively. 
• Mode 1: create a thread for each request, and free it after done. 
• Mode 2: Maintain a limited number of threads in a thread pool with condition variables, and server the clients continuously. (Use POSIX Threads).
