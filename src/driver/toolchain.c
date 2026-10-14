#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <spawn.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

#include "driver.h"

// everything the toolchain runs starts with wlang's environment
extern char** environ;

#define DEFAULT_CC "cc"

const char* default_c_compiler(void) {
    const char* cc = getenv("CC");
    return (cc && *cc) ? cc : DEFAULT_CC;
}

char* path_join(const char* dir, const char* name) {
    size_t size = strlen(dir) + 1 + strlen(name) + 1;
    char* path = malloc(size);
    snprintf(path, size, "%s/%s", dir, name);
    return path;
}

// starts argv[0] (searched in PATH when search_path) and waits for it.
// returns its exit code, 128 + N if signal N killed it, or -1 if it couldn't start (errno set)
static int spawn_and_wait(char* const argv[], bool search_path) {
    // the child shares the terminal: anything buffered here must come out first
    fflush(stdout);
    fflush(stderr);

    pid_t pid;
    int error = search_path ? posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ)
                            : posix_spawn(&pid, argv[0], NULL, NULL, argv, environ);
    if (error != 0) {
        errno = error;
        return -1;
    }

    // Ctrl-C reaches the child too; wlang outlives it so it can clean up and pass on the status
    void (*old_int)(int) = signal(SIGINT, SIG_IGN);
    void (*old_quit)(int) = signal(SIGQUIT, SIG_IGN);

    int status;
    pid_t waited;
    do {
        waited = waitpid(pid, &status, 0);
    } while (waited == -1 && errno == EINTR);

    signal(SIGINT, old_int);
    signal(SIGQUIT, old_quit);

    if (waited == -1) {
        return -1;
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return WEXITSTATUS(status);
}

bool compile_c(const char* cc, const char* c_path, const char* exe_path) {
    char* argv[] = { (char*)cc, (char*)c_path, "-o", (char*)exe_path, NULL };
    int code = spawn_and_wait(argv, true);

    if (code == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "wlang: C compiler '%s' not found (use --cc or set CC)\n", cc);
        } else {
            fprintf(stderr, "wlang: can't run C compiler '%s': %s\n", cc, strerror(errno));
        }
        return false;
    }
    if (code != 0) {
        fprintf(stderr, "wlang: the C compiler rejected the generated code. This is a bug in wlang, please report it.\n");
        return false;
    }
    return true;
}

int run_program(const char* path, char* const args[], int arg_count) {
    char** argv = malloc(sizeof(char*) * (arg_count + 2));
    argv[0] = (char*)path;
    for (int i = 0; i < arg_count; i++) {
        argv[i + 1] = args[i];
    }
    argv[arg_count + 1] = NULL;

    int code = spawn_and_wait(argv, false);
    if (code == -1) {
        fprintf(stderr, "wlang: can't run '%s': %s\n", path, strerror(errno));
    }
    free(argv);
    return code;
}

char* make_temp_dir(void) {
    const char* tmp = getenv("TMPDIR");
    if (!tmp || !*tmp) {
        tmp = "/tmp";
    }

    char* dir = path_join(tmp, "wlang-XXXXXX");
    if (!mkdtemp(dir)) {
        fprintf(stderr, "wlang: can't create a temporary directory in '%s': %s\n", tmp, strerror(errno));
        free(dir);
        return NULL;
    }
    return dir;
}

// the directory only ever holds plain files wlang (and the C compiler) wrote
void remove_temp_dir(const char* dir) {
    DIR* handle = opendir(dir);
    if (handle) {
        struct dirent* entry;
        while ((entry = readdir(handle)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char* path = path_join(dir, entry->d_name);
            unlink(path);
            free(path);
        }
        closedir(handle);
    }
    rmdir(dir);
}
