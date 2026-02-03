#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#define NOB_EXPERIMENTAL_DELETE_OLD

#include "build/nob.h"
#include <unistd.h>
#include <sys/stat.h>

#define NOB_H_REPO "https://github.com/tsoding/nob.h.git"
#define NOB_H_RAW  "https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h"

#define BUILD_DIR      "build"
#define LIB_JSON       BUILD_DIR "/libaxgreet-json.a"
#define LIB_GREETD_IPC BUILD_DIR "/libaxgreet-greetd-ipc.a"
#define OUTPUT         "axgreeter"

static void ensure_nob_h(void);
static void ensure_build_dirs(void);
static bool build_axgreet_json(bool is_debug);
static bool build_axgreet_greetd_ipc(bool is_debug);
static bool build_axgreet(bool is_debug);
static void clean_all(void);
static void run_tests(void);

int main(int argc, char **argv) {
    ensure_nob_h();
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool is_debug = argc > 1 && strcmp(argv[1], "debug") == 0;

    if (argc > 1 && strcmp(argv[1], "clean") == 0) {
        clean_all();
        return 0;
    }

    ensure_build_dirs();

    /* 1) axgreet-json – standalone minimal JSON library → static lib */
    if (!build_axgreet_json(is_debug)) {
        nob_log(NOB_ERROR, "failed to build axgreet-json\n");
        exit(1);
    }

    /* 2) axgreet-greetd-ipc – greetd IPC bindings (depends on json) → static lib */
    if (!build_axgreet_greetd_ipc(is_debug)) {
        nob_log(NOB_ERROR, "failed to build axgreet-greetd-ipc\n");
        exit(1);
    }

    /* 3) axgreet – greeter binary (statically links both libs) */
    if (!build_axgreet(is_debug)) {
        nob_log(NOB_ERROR, "failed to build axgreet\n");
        exit(1);
    }

    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        run_tests();
    }

    return 0;
}

static void ensure_build_dirs(void) {
    if (mkdir(BUILD_DIR, 0755) != 0 && errno != EEXIST) {
        nob_log(NOB_ERROR, "failed to create %s: %s\n", BUILD_DIR, strerror(errno));
        exit(1);
    }
}

static void common_flags(Cmd *cmd, bool is_debug) {
    nob_cc(cmd);
    if (is_debug) {
        cmd_append(cmd, "-g", "-O0");
    } else {
        cmd_append(cmd, "-O2", "-Wall", "-Wextra");
    }
}

/* Compile one .c to .o; returns true on success */
static bool compile_to_obj(const char *src, const char *obj, const char *include_dir, bool is_debug) {
    Cmd cmd = {0};
    nob_cc(&cmd);
    if (is_debug)
        cmd_append(&cmd, "-g", "-O0");
    else
        cmd_append(&cmd, "-O2", "-Wall", "-Wextra");
    cmd_append(&cmd, "-c", "-I", include_dir, "-o", obj, src);
    return cmd_run(&cmd);
}

/* Build libaxgreet-json.a from axgreet-json/*.c */
static bool build_axgreet_json(bool is_debug) {
    const char *dir = "axgreet-json";
    const char *objs[] = {
        BUILD_DIR "/axgreet-json/json.o",
        BUILD_DIR "/axgreet-json/json_lexer.o",
        BUILD_DIR "/axgreet-json/json_parser.o",
    };
    const char *srcs[] = {
        "axgreet-json/json.c",
        "axgreet-json/json_lexer.c",
        "axgreet-json/json_parser.c",
    };

    if (mkdir(BUILD_DIR "/axgreet-json", 0755) != 0 && errno != EEXIST) {
        nob_log(NOB_ERROR, "mkdir %s/axgreet-json: %s\n", BUILD_DIR, strerror(errno));
        return false;
    }

    for (size_t i = 0; i < sizeof(srcs) / sizeof(srcs[0]); i++) {
        if (!compile_to_obj(srcs[i], objs[i], dir, is_debug)) {
            nob_log(NOB_ERROR, "compile %s failed\n", srcs[i]);
            return false;
        }
    }

    Cmd ar_cmd = {0};
    cmd_append(&ar_cmd, "ar", "rcs", LIB_JSON, objs[0], objs[1], objs[2]);
    if (!cmd_run(&ar_cmd)) {
        nob_log(NOB_ERROR, "ar failed for %s\n", LIB_JSON);
        return false;
    }
    return true;
}

/* Build libaxgreet-greetd-ipc.a; depends on json (headers from axgreet-json via -I) */
static bool build_axgreet_greetd_ipc(bool is_debug) {
    const char *obj = BUILD_DIR "/axgreet-greetd-ipc/greetd.o";
    const char *src = "axgreet-greetd-ipc/greetd.c";

    if (mkdir(BUILD_DIR "/axgreet-greetd-ipc", 0755) != 0 && errno != EEXIST) {
        nob_log(NOB_ERROR, "mkdir %s/axgreet-greetd-ipc: %s\n", BUILD_DIR, strerror(errno));
        return false;
    }

    Cmd cmd = {0};
    nob_cc(&cmd);
    if (is_debug)
        cmd_append(&cmd, "-g", "-O0");
    else
        cmd_append(&cmd, "-O2", "-Wall", "-Wextra");
    cmd_append(&cmd, "-c",
               "-I", "axgreet-json",     /* json.h */
               "-I", "axgreet-greetd-ipc", /* greetd.h, log.h */
               "-o", obj, src);
    if (!cmd_run(&cmd)) {
        nob_log(NOB_ERROR, "compile %s failed\n", src);
        return false;
    }

    Cmd ar_cmd = {0};
    cmd_append(&ar_cmd, "ar", "rcs", LIB_GREETD_IPC, obj);
    if (!cmd_run(&ar_cmd)) {
        nob_log(NOB_ERROR, "ar failed for %s\n", LIB_GREETD_IPC);
        return false;
    }
    return true;
}

/* Build axgreet binary: src/main.c + src/hexutil.c, statically link both libs */
static bool build_axgreet(bool is_debug) {
    const char *main_src = "axgreet/main.c";
    if (access(main_src, F_OK) != 0) {
        nob_log(NOB_INFO, "skipping axgreet (no %s)\n", main_src);
        return true;
    }

    Cmd cmd = {0};
    nob_cc(&cmd);
    if (is_debug)
        cmd_append(&cmd, "-g", "-O0");
    else
        cmd_append(&cmd, "-O2", "-Wall", "-Wextra");
    cmd_append(&cmd, "-I", "axgreet-greetd-ipc", "-I", "axgreet-json", "-I", "src");
    nob_cc_output(&cmd, OUTPUT);
    nob_cc_inputs(&cmd, "axgreet/main.c");
    cmd_append(&cmd, LIB_GREETD_IPC, LIB_JSON);
    if (!cmd_run(&cmd)) {
        nob_log(NOB_ERROR, "link %s failed\n", OUTPUT);
        return false;
    }
    return true;
}

static void clean_all(void) {
    Cmd cmd = {0};
    cmd_append(&cmd, "rm", "-rf", BUILD_DIR "/axgreet-json", BUILD_DIR "/axgreet-greetd-ipc",
               LIB_JSON, LIB_GREETD_IPC);
    cmd_run(&cmd);
    cmd = (Cmd){0};
    cmd_append(&cmd, "rm", "-f", OUTPUT);
    cmd_run(&cmd);
}

static void run_tests(void) {
    Cmd cmd = {0};
    char path_to_output[64];
    snprintf(path_to_output, sizeof(path_to_output), "./%s", OUTPUT);
    cmd_append(&cmd, "chmod", "+x", path_to_output);
    cmd_run(&cmd);
    cmd = (Cmd){0};
    cmd_append(&cmd, "sudo", "greetd", "--vt", "4", "--config", "greetd.toml");
    cmd_run(&cmd);
    cmd = (Cmd){0};
    cmd_append(&cmd, "rm", "/tmp/axgreet");
    cmd_run(&cmd);
}

static void ensure_nob_h(void) {
    if (access("build/nob.h", F_OK) == 0) return;
    nob_log(NOB_INFO, "build/nob.h not found, fetching...\n");
    if (mkdir("build", 0755) != 0 && errno != EEXIST) {
        nob_log(NOB_ERROR, "failed to create build/: %s\n", strerror(errno));
        exit(1);
    }
    int ret = system("git clone --depth 1 " NOB_H_REPO " build/.nob_h_repo "
                     "&& cp build/.nob_h_repo/nob.h build/nob.h "
                     "&& rm -rf build/.nob_h_repo");
    if (ret != 0) ret = system("curl -L -s -o build/nob.h " NOB_H_RAW);
    if (ret != 0) ret = system("wget -q -O build/nob.h " NOB_H_RAW);
    if (ret != 0) {
        nob_log(NOB_ERROR, "failed to fetch nob.h (tried git, curl, wget)\n");
        exit(1);
    }
    nob_log(NOB_INFO, "fetched build/nob.h\n");
}
