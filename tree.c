#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define NAME_MAX 255

// Global error flag to indicate if an error occurred
int error_occurred = 0;

// A linked list to keep track of visited directories
typedef struct VisitedNode {
    char name[NAME_MAX];
    struct VisitedNode *next;
} VisitedNode;

VisitedNode *visited_head = NULL; // Initialize the head of the visited list

// Function to add a directory to the visited list
void add_to_visited(const char *name) {
        VisitedNode *new_node = malloc(sizeof(VisitedNode));
        if (new_node) {
            memset(new_node, 0, sizeof(VisitedNode)); // Initialize memory
            strncpy(new_node->name, name, NAME_MAX - 1);
            new_node->name[NAME_MAX - 1] = '\0'; // Ensure null termination
            new_node->next = visited_head;
            visited_head = new_node;
        }
    }

// Function to check if a directory has been visited
int is_visited(const char *name) {
    VisitedNode *current = visited_head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Function to free the visited list
void free_visited_list() {
    VisitedNode *current = visited_head;
    while (current) {
        VisitedNode *temp = current;
        current = current->next;
        free(temp);
    }
}

// Function to print file permissions
void print_permissions(mode_t mode) {
    printf("%c", S_ISDIR(mode) ? 'd' : S_ISLNK(mode) ? 'l' : '-');
    printf("%c", (mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (mode & S_IROTH) ? 'r' : '-');
    printf("%c", (mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (mode & S_IXOTH) ? 'x' : '-');
}

// Function to check if a string represents an integer
int is_integer(const char *str) {
    char *endptr;
    strtol(str, &endptr, 10);  // Convert string to long
    return *str != '\0' && *endptr == '\0';
}

// Comparison function for qsort
int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to list the directory contents recursively
void list_directory(const char *path, int depth, int max_depth, int show_hidden, int show_details) {
    struct dirent *entry;
    DIR *dp;
    struct stat statbuf;
    char curr_path[PATH_MAX];

    if (lstat(path, &statbuf) != 0) {
        fprintf(stdout, "%s: %s\n", path, strerror(errno));
        error_occurred = 1;
        return;
    }

    if (depth == 0 && max_depth == 0) {
        if (show_details) {
            print_permissions(statbuf.st_mode);
            printf(" %s\n", path);
        } else {
            printf("%s\n", path);
        }
        return;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        if (depth == 0) {
            if (show_details) {
                print_permissions(statbuf.st_mode);
                printf(" %s", path);
            } else {
                printf("%s", path);
            }
        }

        if (chdir(path) != 0) {
            fprintf(stdout, ": %s\n", strerror(errno));
            error_occurred = 1;
            return;
        } else {
            printf("\n");
        }

        getcwd(curr_path, sizeof(curr_path));
        if (is_visited(curr_path)) {
            chdir("..");
            return;
        }

        add_to_visited(curr_path);

        dp = opendir(".");
        if (dp == NULL) {
            fprintf(stdout, ": %s\n", strerror(errno));
            error_occurred = 1;
            chdir("..");
            return;
        }

        char *entries[1024];
        int n = 0;

        while ((entry = readdir(dp))) {
            if (!show_hidden && entry->d_name[0] == '.') {
                continue;
            }
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            entries[n] = strdup(entry->d_name);
            n++;
        }

        closedir(dp);

        qsort(entries, n, sizeof(char *), compare);

        for (int i = 0; i < n; i++) {
            if (lstat(entries[i], &statbuf) != 0) {
                for (int j = 0; j < (depth + 1) * 4; j++) printf(" ");
                printf("%s: %s\n", entries[i], strerror(errno));
                error_occurred = 1;
                free(entries[i]);
                continue;
            }

            if (show_details) {
                print_permissions(statbuf.st_mode);
            }

            for (int j = 0; j < (depth + 1) * 4; j++) printf(" ");
            if (show_details) {
                printf(" %s", entries[i]);

                if (S_ISLNK(statbuf.st_mode)) {
                    char link_target[PATH_MAX];
                    ssize_t len = readlink(entries[i], link_target, sizeof(link_target) - 1);
                    if (len != -1) {
                        link_target[len] = '\0';
                        printf(" -> %s", link_target);
                    } else {
                        printf(" -> (unreadable)");
                    }
                }
            } else {
                printf("%s", entries[i]);
            }

            if (S_ISDIR(statbuf.st_mode) && (max_depth == -1 || depth < max_depth - 1)) {
                list_directory(entries[i], depth + 1, max_depth, show_hidden, show_details);
            } else {
                printf("\n");
            }
            free(entries[i]);
        }

        chdir(".."); // Change back to the parent directory
    } else {
        for (int i = 0; i < depth * 4; i++) printf(" ");
        if (show_details) {
            print_permissions(statbuf.st_mode);
            printf(" %s\n", path);
        } else {
            printf("%s\n", path);
        }
    }
}

int main(int argc, char *argv[]) {
    int show_hidden = 0, max_depth = -1, show_details = 0;
    char *root_path = ".";
    struct stat statbuf;
    int i = 1;

    for (i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            show_hidden = 1;
        } else if (strncmp(argv[i], "-d", 2) == 0) {
            if (!is_integer(argv[i + 1])) {
                fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
                exit(EXIT_FAILURE);
            }
            max_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0) {
            show_details = 1;
        } else {
            fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
            exit(EXIT_FAILURE);
        }
    }

    if (i < argc) {
        root_path = argv[i];
    }

    if (lstat(root_path, &statbuf) != 0) {
        fprintf(stdout, "%s: %s\n", root_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(statbuf.st_mode)) {
        list_directory(root_path, 0, max_depth, show_hidden, show_details);
    } else {
        if (show_details) {
            print_permissions(statbuf.st_mode);
            printf(" %s", root_path);
            if (S_ISLNK(statbuf.st_mode)) {
                char link_target[PATH_MAX];
                ssize_t len = readlink(root_path, link_target, sizeof(link_target) - 1);
                if (len != -1) {
                    link_target[len] = '\0';
                    printf(" -> %s", link_target);
                } else {
                    perror("readlink");
                }
            }
            printf("\n");
        } else {
            printf("%s\n", root_path);
        }
    }

    free_visited_list();

    if (error_occurred) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}