#define NOB_IMPLEMENTATION
#include "build/nob.h"

const char* compiler = "cc";
const char* output = "axgreet";
const char* sources[] = {
    "src/main.c",
    "src/greetd.c",
    "src/hexutil.c",
    "src/json/json.c",
    "src/json/json_lexer.c",
    "src/json/json_parser.c",
};

void test() {
    Cmd cmd = { 0 };
    char path_to_output[strlen(output) + 2];
    sprintf(path_to_output, "./%s", output);

    cmd_append(&cmd, "chmod", "+x", path_to_output);
    cmd_run(&cmd);

    cmd_append(&cmd, "sudo", "greetd", "--vt", "2", "--config", "greetd.toml");
    cmd_run(&cmd);

    cmd_append(&cmd, "rm", "/tmp/axgreet");
    cmd_run(&cmd);
}

void clean() {
    Cmd cmd = { 0 };
    cmd_append(&cmd, "rm", "-f", output);
    cmd_run(&cmd);
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    int is_debug = argc > 1 && strcmp(argv[1], "debug") == 0;
    if (argc > 1 && strcmp(argv[1], "clean") == 0) {
        clean();
        return 0;
    }

    Cmd cmd = { 0 };
    cmd_append(&cmd, "cc", "-o", output);
    if (is_debug) {
        cmd_append(&cmd, "-g", "-O0");
    } else {
        cmd_append(&cmd, "-O2", "-Wall", "-Wextra");
    }
    for (int i = 0; i < sizeof(sources) / sizeof(sources[0]); i++) {
        cmd_append(&cmd, sources[i]);
    }
    cmd_run(&cmd);

    if (argc > 1) {
        char* command = argv[1];
        if (strcmp(command, "test") == 0) {
            test();
        }
    }

    return 0;
}
