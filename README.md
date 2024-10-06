### CSC 357 Assignment 5 - Directories

Purpose:
- This assignment involves working with file system operations to recursively list the contents of a directory. The task will use system calls to traverse the directory structure, display file and directory information, and handle various user-specified options.

Ground Rules:
- Your code must compile without memory leaks using gcc and valgrind.
- Programs must run on Cal Poly’s UNIX servers and only use POSIX standard system calls and library functions.

Directory Tree Traversal:
- Implement a program named tree that recursively lists all files and directories under a given root path, following a depth-first, pre-order traversal.
- If no path is provided, the root path defaults to the current directory (.).
- Files and directories must be printed in alphabetical order, with the root path listed first and each subsequent level indented by 4 spaces.

Optional Arguments:
- -a: Include hidden files (those starting with a .), but exclude special directories . and ...
- -d N: Limit the depth of the listing to N levels below the root. If N=0, only the root is listed.
- -l: Precede each file’s listing with its mode (permissions and type) and expand soft links.
- Arguments may appear in any order, but -a, -d N, and -l must precede the root path if they are provided.
- If invalid arguments are provided or the argument format is incorrect, print a usage message to stderr and exit with EXIT_FAILURE.
- If a file cannot be accessed (e.g., due to permissions), print an error message (similar to perror) appended to the file's listing. The program should still exit normally (EXIT_SUCCESS) unless there are inaccessible files, in which case it exits with EXIT_FAILURE.

Error Handling:
- If the root path or any files within the directory cannot be accessed, print the corresponding error message.
- The program should still attempt to list as much of the directory tree as possible before exiting.

System Calls and Functions:
- The following system calls and functions may be useful:
- chdir/fchdir: Change the current working directory.
- S_ISDIR, S_ISLNK: Check if a file is a directory or a symbolic link.
- readlink: Read the contents of a symbolic link.
- strerror: Get a string for the most recent error.
- Use lstat instead of stat to avoid following symbolic links.

Testing:
- The program must be tested to ensure it behaves as expected for various input arguments and directory structures.
- Testing will be performed using diff to compare the output against expected results.
