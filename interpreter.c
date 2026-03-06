#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int echo( char *value);
int badcommandFileDoesNotExist();
int my_ls ();
int my_mkdir(char *dir);
int my_cd(char *dir);
int my_touch(char *filemname);
// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    }else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);   

     }else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1) return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2) return badcommand();
        return my_mkdir(command_args[1]);}
        else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2) return badcommand();
        return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2) return badcommand();
        return my_cd(command_args[1]);}

        else if (strcmp(command_args[0], "run")==0){
        if (args_size<2){return badcommand();}
        pid_t pid = fork();
        switch (pid){ 
           case 0: //child
              execvp(command_args[1], command_args + 1);
              perror("run");// failed execution
              _exit(1);

           case -1: //fork failed
              perror("fork");
              return -1;

           default: //parent
               waitpid(pid,NULL,0);
               return 0;
           }
        } else return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with Bye!\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}
int echo(char *value) {
    if (value[0] != '$'){
        printf("%s\n", value);
    }
    else {
        char *var = value + 1;
        printf("%s\n", mem_get_value(var));
    }
    
    return 0;
}

// helper: checks token is only [A-Za-z0-9] and non-empty 
static int is_alnum_str(const char *s) {
    if (s == NULL || *s == '\0') return 0;
    for (const unsigned char *p = (const unsigned char*)s; *p; p++) {
        if (!isalnum(*p)) return 0;
    }
    return 1;
}

/* qsort comparator for strings */
static int cmpstr(const void *a, const void *b) {
    const char *sa = *(const char * const *)a;
    const char *sb = *(const char * const *)b;
    return strcmp(sa, sb);
}
 // list entries in current directory, sorted
int my_ls() {
    DIR *d = opendir(".");
    if (!d) return 1;

    char *names[4096]; // store file names
    int n = 0;

    // read directory entries
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (n >= 4096) break;
        names[n++] = strdup(ent->d_name);
    }
    closedir(d);

   // ensure "." and ".." are present
    int has_dot = 0, has_dotdot = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(names[i], ".") == 0) has_dot = 1;
        if (strcmp(names[i], "..") == 0) has_dotdot = 1;
    }
    if (!has_dot && n < 4096) names[n++] = strdup(".");
    if (!has_dotdot && n < 4096) names[n++] = strdup("..");


    // sort then print one per line
    qsort(names, n, sizeof(char*), cmpstr);

    for (int i = 0; i < n; i++) {
        printf("%s\n", names[i]);
        free(names[i]);
    }
    return 0;
}
int my_touch (char *filename){
 // only allow simple names because itt is required 
if (!is_alnum_str(filename)) return 1;



    FILE *f = fopen(filename, "a");  // "a" creates file if missing or otherwise just opens/closes
    if (!f) return 1;
    fclose(f);
    return 0;
}

int my_cd(char *dirname) {
    // only allow simple directory token
    if (!is_alnum_str(dirname)) {
        printf("Bad command: my_cd\n");
        return 1;
    }
     // change process working directory
    if (chdir(dirname) != 0) {
        printf("Bad command: my_cd\n");
        return 1;
    }
    return 0;
}
int my_mkdir(char *dirname_token) {
    // supports my_mkdir NAME and  my_mkdir VAR
    if (dirname_token == NULL) {
        printf("Bad command: my_mkdir\n");
        return 1;
    }

    char dirname_buf[128];
    char *dirname = dirname_token;

    // my_mkdir $VAR
    if (dirname_token[0] == '$') {
        char *varname = dirname_token + 1;

        char *val = mem_get_value(varname);
        if (val == NULL || strcmp(val, "Variable does not exist") == 0) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }

        // must be single alnum token
        if (!is_alnum_str(val)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }

        strncpy(dirname_buf, val, sizeof(dirname_buf) - 1);
        dirname_buf[sizeof(dirname_buf) - 1] = '\0';
        dirname = dirname_buf;

    } else {
        if (!is_alnum_str(dirname_token)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
    }

    if (mkdir(dirname, 0777) != 0) {
        printf("Bad command: my_mkdir\n");
        return 1;
    }
    return 0;
}
