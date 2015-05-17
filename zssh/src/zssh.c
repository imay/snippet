// What I want is a command-line tool.
// I can use this to send a command to remote host with ssh
// especially, I can pass the password when I call this function
//
// For example: 
//  pssh -h user@remote.host -p xxxx -c ''

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libssh/libssh.h>

void process_remote_command(
        const char* host,
        const char* user,
        const char* passwd, 
        const char* command) {
    fprintf(stdout, "######################### %s #########################\n", host);
    ssh_session ss = NULL;
    ssh_channel sc = NULL;
    int ret = SSH_OK;
    // new ssh session
    ss = ssh_new();
    if (ss == NULL) {
        fprintf(stderr, "ssh new session failed.\n");
        return;
    }
    // connect ssh session
    ret = ssh_options_set(ss, SSH_OPTIONS_HOST, host);
    if (ret != SSH_OK) {
        fprintf(stderr, "ssh set SSH_OPTIONS_HOST failed. host=%s,error=%s\n", 
                host, ssh_get_error(ss));
        goto free_ss;
    }
    if (user != NULL) {
        ret = ssh_options_set(ss, SSH_OPTIONS_USER, user);
        if (ret != SSH_OK) {
            fprintf(stderr, "ssh set SSH_OPTIONS_HOST failed. user=%s,error=%s\n", 
                    user, ssh_get_error(ss));
            goto free_ss;
        }
    }
    ret = ssh_connect(ss);
    if (ret != SSH_OK) {
        fprintf(stderr, "ssh connect failed. host=%s,error=%s\n", 
                host, ssh_get_error(ss));
        goto free_ss;
    }
    // auth password
    ret = ssh_userauth_password(ss, NULL, passwd);
    if (ret != SSH_OK) {
        fprintf(stderr, "ssh connect failed. password=%s,error=%s\n", 
                passwd, ssh_get_error(ss));
        goto disconect_ss;
    }
    // new ssh channel
    sc = ssh_channel_new(ss);
    if (sc == NULL) {
        fprintf(stderr, "ssh new channel failed. error=%s\n", ssh_get_error(ss));
        goto disconect_ss;
    }
    ret = ssh_channel_open_session(sc);
    if (ret != SSH_OK) {
        fprintf(stderr, "ssh connect failed. error=%s\n", ssh_get_error(ss));
        goto free_sc;
    }
    ret = ssh_channel_request_exec(sc, command);
    if (ret != SSH_OK) {
        fprintf(stderr, "ssh execute command failed. comand=%s, error=%s\n", 
                command, ssh_get_error(ss));
        goto close_sc;
    }
    // output result
    for (;;) {
        char buf[1024];
        int nbytes = ssh_channel_read(sc, buf, sizeof(buf), 0);
        if (nbytes <= 0) {
            break;
        }
        write(1, buf, nbytes);
    }

close_sc:
    ssh_channel_close(sc);
free_sc:
    ssh_channel_free(sc);
disconect_ss:
    ssh_disconnect(ss);
free_ss:
    ssh_free(ss);
}

enum option_return_e {
    /* starts with 300, ascii is used for short args */
    OPT_HELP = 300,
    OPT_VERSION,
    OPT_HOSTS,
    OPT_HOST,
    OPT_USER,
    OPT_PASSWORD,
};

struct option k_pssh_options[] = {
    /* name, has_arg, flag, val */
    {"help", 0, NULL, OPT_HELP},
    {"version", 0, NULL, OPT_VERSION},
    {"hosts", 1, NULL, OPT_HOSTS},
    {"user", 1, NULL, OPT_HOST},
    {"password", 1, NULL, OPT_PASSWORD},
    /* endof struct */
    {NULL, 0, NULL, 0} 
};

void print_version() {
    fprintf(stdout, "pssh version: 0.1\n");
}

void print_help() {
    fprintf(stdout, 
            "Usage: pssh [OPTIONS] command [...]\n"
            "\n"
            "Options:\n"
            "--version              show program's version number and exit\n"
            "--help                 show this help message and exit\n"
            "-h HOST_FILE, --hosts=HOST_FILE\n"
            "                       hosts file (each line \"[user@]host[:port]\")\n"
            "-H HOST_STRING, --host=HOST_STRING\n"
            "                       additional host entries (\"[user@]host[:port]\")\n"
            "-l USER, --user=USER   username (OPTIONAL)\n"
            "-p password, --password=password\n"
            "                       password (OPTIONAL)\n"
            "Example: pssh -h host.txt whoaim\n"
        );
}

struct opt_args_t {
    char* host_file;
    char* host;
    char* user;
    char* command;
    char* passwd;
};

void init_opts(struct opt_args_t* opts) {
    opts->host_file = NULL;
    opts->host = NULL;
    opts->user = NULL;
    opts->passwd = NULL;
    opts->command = NULL;
}

void destroy_opts(struct opt_args_t* opts) {
    if (opts->host_file != NULL) {
        free(opts->host_file);
    }
    if (opts->host != NULL) {
        free(opts->host);
    }
    if (opts->user != NULL) {
        free(opts->user);
    }
    if (opts->passwd != NULL) {
        free(opts->passwd);
    }
    if (opts->command != NULL) {
        free(opts->command);
    }
}

void debug_opts(struct opt_args_t* arg) {
    fprintf(stdout, "host_file is %s\n", arg->host_file ? : "NULL");
    fprintf(stdout, "host is %s\n", arg->host ? : "NULL");
    fprintf(stdout, "user is %s\n", arg->user ? : "NULL");
    fprintf(stdout, "passwd is %s\n", arg->passwd ? : "NULL");
    fprintf(stdout, "command is %s\n", arg->command ? : "NULL");
}

// parse arguments, this function will call 'exit' to stop this process in following situation:
// 1. meet exit argument, for example '--help', '--version'
// 2. meet unknown arguments.
void parse_args(int argc, char* argv[], struct opt_args_t* opts) {
    int finish = 0;
    char* pos;
    char* end;
    while (!finish) {
        int ret = 0;
        int opt_idx = 0;

        ret = getopt_long(argc, argv, 
                          "+h:l:H:p:",
                          k_pssh_options, &opt_idx);
        switch (ret) {
        case OPT_HELP:
            print_help();
            exit(0);
        case OPT_VERSION:
            print_version();
            exit(0);
        case 'h':
        case OPT_HOSTS:
            opts->host_file = strdup(optarg);
            break;
        case 'H':
        case OPT_HOST:
            opts->host = strdup(optarg);
            break;
        case 'l':
        case OPT_USER:
            opts->user = strdup(optarg);
            break;
        case 'p':
        case OPT_PASSWORD:
            opts->passwd = strdup(optarg);
            break;
        case '?':
            /* invalid argument */
            exit(-1);
        case -1:
            /* over */
            finish = 1;
            break;
        default:
            fprintf(stderr, "Unknown argument. ret=%d\n", ret);
            exit(-1);
        }
    }
    // now is command
    if (optind == argc) {
        print_help();
        exit(0);
    }
    pos = opts->command = malloc(4096);
    // reserved for '\0'
    end = pos + 4096 - 1;
    for (; optind < argc; ++optind) {
        char* src = argv[optind];
        while (pos < end && *src) {
            *pos++ = *src++;
        }
        if (pos >= end) {
            fprintf(stderr, "command is too long....\n");
            exit(-1);
        }
        if (optind < argc - 1) {
            *pos++ = ' ';
        } else {
            *pos++ = '\0';
        }
    }
}

static void process_host(struct opt_args_t* opts) {
    process_remote_command(opts->host, opts->user, opts->passwd, opts->command);
}

static void process_hosts(struct opt_args_t* opts) {
    char* line = NULL;
    size_t read = 0;
    size_t len = 0;
    FILE* fp = fopen(opts->host_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "open host_file(%s) failed, because: %m\n", opts->host_file);
        exit(-1);
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        if (read == 0) {
            continue;
        }
        if (read == 1 && isspace(line[read - 1])) {
            continue;
        }
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        process_remote_command(line, opts->user, opts->passwd, opts->command);
    }

    if (line != NULL) {
        free(line);
    }

close:
    fclose(fp);
}

void process(struct opt_args_t* opts) {
    if (opts->host == NULL && opts->host_file == NULL) {
        fprintf(stderr, "Please pass host or host_file.\n");
        exit(-1);
    }
    if (opts->passwd == NULL) {
        opts->passwd = malloc(256);
        if (ssh_getpass("Please input password:", opts->passwd, 256, 0, 0)) {
            fprintf(stderr, "Password you input is invalid.\n");
            exit(-1);
        }
    }
    if (opts->host_file != NULL) {
        process_hosts(opts);
    } else {
        process_remote_command(opts->host, opts->user, opts->passwd, opts->command);
    }
}

int main(int argc, char* argv[]) {
    struct opt_args_t opts;
    init_opts(&opts);
    parse_args(argc, argv, &opts);
    process(&opts);
    destroy_opts(&opts);
    return 0;
}

