#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#define NOB_EXPERIMENTAL_DELETE_OLD

#include "build/nob.h"
#include <unistd.h>
#include <sys/stat.h>

#define NOB_H_REPO "https://github.com/tsoding/nob.h.git"
#define NOB_H_RAW  "https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h"

#define OUTPUT "axgreet"

void sources(Cmd* cmd) {
    nob_cc_inputs(cmd, "src/main.c");
    nob_cc_inputs(cmd, "src/greetd.c");
    nob_cc_inputs(cmd, "src/hexutil.c");
    nob_cc_inputs(cmd, "src/json/json.c");
    nob_cc_inputs(cmd, "src/json/json_lexer.c");
    nob_cc_inputs(cmd, "src/json/json_parser.c");
}

void debug_flags(Cmd* cmd) {
    cmd_append(cmd, "-g", "-O0");
}

void release_flags(Cmd* cmd) {
    cmd_append(cmd, "-O2", "-Wall", "-Wextra");
}

void test();
void clean();
static void ensure_nob_h();

int main(int argc, char** argv) {
    ensure_nob_h();
    NOB_GO_REBUILD_URSELF(argc, argv);

    int is_debug = argc > 1 && strcmp(argv[1], "debug") == 0;
    if (argc > 1 && strcmp(argv[1], "clean") == 0) {
        clean();
        return 0;
    }

    Cmd cmd = { 0 };
    nob_cc(&cmd);
    nob_cc_output(&cmd, OUTPUT);
    if (is_debug) {
        debug_flags(&cmd);
    } else {
        release_flags(&cmd);
    }
   

    sources(&cmd);
    cmd_run(&cmd);

    if (argc > 1) {
        char* command = argv[1];
        if (strcmp(command, "test") == 0) {
            test();
        }
    }

    return 0;
}

void test() {
    Cmd cmd = { 0 };
    char path_to_output[sizeof(OUTPUT) + 2];
    sprintf(path_to_output, "./%s", OUTPUT);

    cmd_append(&cmd, "chmod", "+x", path_to_output);
    cmd_run(&cmd);

    cmd_append(&cmd, "sudo", "greetd", "--vt", "2", "--config", "greetd.toml");
    cmd_run(&cmd);

    cmd_append(&cmd, "rm", "/tmp/axgreet");
    cmd_run(&cmd);
}

void clean() {
    Cmd cmd = { 0 };
    cmd_append(&cmd, "rm", "-f", OUTPUT);
    cmd_run(&cmd);
}

static void ensure_nob_h(void) {
    if (access("build/nob.h", F_OK) == 0) {
        return;
    }
    nob_log(NOB_INFO, "build/nob.h not found, fetching...\n");
    if (mkdir("build", 0755) != 0 && errno != EEXIST) {
        nob_log(NOB_INFO, "failed to create build/: %s\n", strerror(errno));
        exit(1);
    }
    int ret = system("git clone --depth 1 " NOB_H_REPO " build/.nob_h_repo "
                     "&& cp build/.nob_h_repo/nob.h build/nob.h "
                     "&& rm -rf build/.nob_h_repo");
    if (ret != 0) {
        ret = system("curl -L -s -o build/nob.h " NOB_H_RAW);
    }
    if (ret != 0) {
        ret = system("wget -q -O build/nob.h " NOB_H_RAW);
    }
    if (ret != 0) {
        nob_log(NOB_ERROR, "failed to fetch nob.h (tried git, curl, wget)\n");
        exit(1);
    }
    nob_log(NOB_INFO, "fetched build/nob.h\n");
}
