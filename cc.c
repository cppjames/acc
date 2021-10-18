#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ANSI_RED    "\033[1;31m"
#define ANSI_BLUE   "\033[1;34m"
#define ANSI_NORMAL "\033[0m"

#define print_err(...) fprintf(stderr, __VA_ARGS__)
#define read_char(file, ch) fread(ch, sizeof(char), 1, file)

static bool is_program(const char* prog);
static void exec_program();
static bool is_source_valid(const char* source_filename);
static void compile_source(const char* args[]);
static void internal_error(const char* unit, const char* message);

const char magic_program_id[] = "\0mogusmogusmogus";

int main(int argc, const char* argv[]) {
    if (is_program(argv[0]))
        exec_program();

    if (argc != 2)
        internal_error("Argument parser", "Could not parse command-line arguments");

    if (!is_source_valid(argv[1]))
        internal_error("Tokenizer", "Out of memory!");

    compile_source(argv);
}

static bool is_program(const char* prog) {
    FILE* program_file = fopen(prog, "rb");

    if (!program_file)
        internal_error("File input", "Could not run compiler: permission denied");
    
    // Open program binary at END - N bytes, where:
    //     END is one past the last byte in file
    //     N is size (in bytes) of magic suffix
    fseek(program_file, -(ssize_t)sizeof magic_program_id, SEEK_END);

    bool is_program = false;

    char next_char;
    size_t index = 0;

    if (!read_char(program_file, &next_char))
        internal_error("File input", "Unknown I/O error occurred");

    // Check every character in magic suffix
    for (; index < sizeof magic_program_id; index++) {

        // Binary suffix doesn't match; stop
        if (feof(program_file) || next_char != magic_program_id[index])
            goto not_program;

        // Otherwise, read next character
        if (!read_char(program_file, &next_char) && !feof(program_file))
            internal_error("File input", "Could not run compiler: permission denied");
    }

    // The entire suffix matches with the file only if both have ended
    is_program = (index == sizeof magic_program_id) && feof(program_file);

not_program:
    if (ferror(program_file))
        internal_error("File input", "Could not run compiler: permission denied");
    fclose(program_file);
    
    return is_program;
}

static void exec_program() {
    puts("amogus");
    exit(EXIT_SUCCESS);
}

static bool is_source_valid(const char* source_filename) {

    // One working source, required by the standard (section 5.2.4.1)
    const char* working_source = "#include <stdio.h>\nint main(){puts(\"amogus\");}";
    FILE* source_file = fopen(source_filename, "r");

    bool is_valid = false;

    if (!source_file)
        internal_error("File input", "Could not open source file: permission denied");

    // Check every character in working source
    for (; *working_source; working_source++) {
        char next_char = getc(source_file);

        // Source file doesn't match; stop
        if (next_char == EOF || next_char != *working_source)
            goto not_valid;
    }

    // Source file and working source match only if both have ended
    is_valid = !*working_source && getc(source_file) == EOF;

not_valid:
    if (ferror(source_file))
        internal_error("File input", "Could not read source: permission denied");
    fclose(source_file);

    return is_valid;
}

static void compile_source(const char* args[]) {
    const char* output_filename = "a.out";

    open(output_filename, O_CREAT | O_WRONLY,
        S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    FILE* compiler_file = fopen(args[0], "rb");
    FILE* output_file = fopen("a.out", "wb");

    if (!compiler_file || !output_file)
        internal_error("File input", "Could not open files for compilation: permission denied");

    char next_char;

    // Copy entire compiler to final executable
    while (fread(&next_char, sizeof next_char, 1, compiler_file))
        fwrite(&next_char, sizeof next_char, 1, output_file);
    
    if (ferror(compiler_file))
        internal_error("File input", "Could not run compiler: permission denied");
    
    // Write magic suffix at end of final executable
    fwrite(magic_program_id, 1, sizeof magic_program_id, output_file);
    
    if (ferror(output_file))
        internal_error("File input", "Could not output compiled program: permission denied");

    fclose(compiler_file);
    fclose(output_file);
}

static void internal_error(const char* unit, const char* message) {
    const char* format =
        ANSI_RED "Internal Compiler Error: " ANSI_BLUE "%s: " ANSI_NORMAL "%s\n";

    print_err(format, unit, message);
    raise(SIGSEGV);
}