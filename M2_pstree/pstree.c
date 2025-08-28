#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>

#define MAX_NAME_LEN 256
#define MAX_PROCESSES 32768

typedef struct {
    int show_pids;
    int numeric_sort;
    int show_version;
} options_t;

typedef struct process{
    int pid;
    int ppid;
    char name[MAX_NAME_LEN];
    struct process **children;
    int child_count;
    int child_capacity;
} process_t;

process_t processes[MAX_PROCESSES];
int process_count = 0;

void print_version() {
    printf("pstree_gula00 1.0\n");
}

int read_process_info(int pid, process_t *proc) {
    char path[256];
    FILE *file;
    char comm[MAX_NAME_LEN];
    int ppid;
    char state;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    file = fopen(path, "r");

    if (fscanf(file, "%d %s %c %d", &proc->pid, comm, &state, &proc->ppid) != 4) {
        fclose(file);
        return 0;
    }
    
    if (comm[0] == '(' && comm[strlen(comm)-1] == ')') {
        comm[strlen(comm)-1] = '\0';
        strcpy(proc->name, comm + 1);
    } else {
        perror("pharase comm");
    }

    proc->children = NULL;
    proc->child_count = 0;
    proc->child_capacity = 0;

    fclose(file);
    return 1;
}

int is_number(const char *str) {
    if (!str || !*str) return 0;
    while (*str) {
        if (!isdigit((*str))) return 0;
        str++;
    }
    return 1;
}

void scan_processes() {
    DIR *proc_dir;
    struct dirent *entry;

    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("opendir /proc");
        exit(1);
    }

    process_count = 0;
    while((entry = readdir(proc_dir)) != NULL) {
        if (is_number(entry->d_name)) {
            int pid = atoi(entry->d_name);
            if (read_process_info(pid, &processes[process_count])) {
                process_count++;
                if (process_count >= MAX_PROCESSES) {
                    break;
                } 
            }
        }
    }

    closedir(proc_dir);
}

void add_child(process_t *parent, process_t *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity = parent->child_capacity ? parent->child_capacity * 2 : 4;
        parent->children = realloc(parent->children, parent->child_capacity * sizeof(process_t*));
    }
    parent->children[parent->child_count++] = child;
}

void build_tree() {
    // for (int i = 0; i < process_count; i++) {
    //     processes[i].children = NULL;
    // }

    // child i -> parent j
    for (int i = 0; i < process_count; i++) {
        // Skip init's parent
        if (processes[i].ppid == 0) continue;

        for (int j = 0; j < process_count; j++) {
            if (processes[j].pid == processes[i].ppid) {
                add_child(&processes[j], &processes[i]);
                break;
            }
        } 
    }
}

void print_tree(process_t *proc, const char *prefix, int is_last, options_t *opts) {
    printf("%s", prefix);

    if (is_last) {
        printf("└─");
    } else {
        printf("├─");
    }
    
    printf("%s", proc->name);
    if (opts->show_pids) {
        printf("(%d)", proc->pid);
    }
    printf("\n");

    // Print children
    for (int i = 0; i < proc->child_count; i++) {
        char new_prefix[4096];
        snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "│   ");
        print_tree(proc->children[i], new_prefix, i == proc->child_count - 1, opts);
    }

}

process_t* find_root() {
    for (int i = 0; i < process_count; i++) {
        if (processes[i].pid == 1) {
            return &processes[i];
        }
    }
    // Is no pid 1 possible?
    printf("not found");
    return NULL;
}

// SORT CHILDREN
int compare_processes(const void *a, const void *b) {
    process_t *proc_a = *(process_t**)a;
    process_t *proc_b = *(process_t**)b;
    return proc_a->pid - proc_b->pid;
}

void sort_children(process_t *proc) {
    if (proc->child_count > 1) {
        qsort(proc->children, proc->child_count, sizeof(process_t*), compare_processes);
    }
    for (int i = 0; i < proc->child_count; i++) {
        sort_children(proc->children[i]);
    }
}

int main(int argc, char* argv[]) {
    options_t opts = {0, 0, 0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0) {
            opts.show_pids = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric-sort") == 0) {
            opts.numeric_sort = 1;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            opts.show_version = 1;
        } else {
            fprintf(stderr, "%s: invalid option '%s'\n", argv[0], argv[i]);
            return 1;
        }
    }

    if (opts.show_version) {
        print_version();
        return 0;
    }

    scan_processes();
    printf("process_count: %d\n", process_count);
    build_tree();
    
    if (opts.numeric_sort) {
        sort_children(root);
    }

    process_t* root = find_root();
    print_tree(root, "", 1, &opts);

    
    for (int i = 0; i < process_count; i++) {
        free(processes[i].children);
    }

    return 0;
}
