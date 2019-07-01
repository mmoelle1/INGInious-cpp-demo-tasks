#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

#include <libintl.h>
#include <locale.h>
#define _(STRING) gettext(STRING)
#include <dlfcn.h>
#include <malloc.h>

#include "wrap.h"

#define TAGS_NB_MAX 20
#define TAGS_LEN_MAX 30

extern bool wrap_monitoring;
extern struct wrap_stats_t stats;
extern struct wrap_monitor_t monitored;
extern struct wrap_fail_t failures;
extern struct wrap_log_t logs;

extern sigjmp_buf segv_jmp;

int true_stderr;
int true_stdout;
int pipe_stderr[2], usr_pipe_stderr[2];
int pipe_stdout[2], usr_pipe_stdout[2];
extern int stdout_cpy, stderr_cpy;
struct itimerval it_val;

CU_pSuite pSuite = NULL;


struct info_msg {
    char *msg;
    struct info_msg *next;
};

struct __test_metadata {
    struct info_msg *fifo_in;
    struct info_msg *fifo_out;
    char problem[140];
    char descr[250];
    unsigned int weight;
    unsigned char nb_tags;
    char tags[TAGS_NB_MAX][TAGS_LEN_MAX];
    int err;
} test_metadata;


void set_test_metadata(char *problem, char *descr, unsigned int weight)
{
    test_metadata.weight = weight;
    strncpy(test_metadata.problem, problem, sizeof(test_metadata.problem));
    strncpy(test_metadata.descr, descr, sizeof(test_metadata.descr));
}

void push_info_msg(char *msg)
{
    if (strstr(msg, "#") != NULL || strstr(msg, "\n") != NULL) {
        test_metadata.err = EINVAL;
        return;
    }

    struct info_msg *item = malloc(sizeof(struct info_msg));
    if (item == NULL)
        test_metadata.err = ENOMEM;

    item->next = NULL;
    item->msg = malloc(strlen(msg) + 1);
    if (item->msg == NULL)
        test_metadata.err = ENOMEM;

    strcpy(item->msg, msg);
    if (test_metadata.fifo_in == NULL && test_metadata.fifo_out == NULL) {
        test_metadata.fifo_in = item;
        test_metadata.fifo_out = item;
    } else {
        test_metadata.fifo_out->next = item;
        test_metadata.fifo_out = item;
    }
}

void set_tag(char *tag)
{
    int i=0;
    while (tag[i] != '\0' && i < TAGS_LEN_MAX) {
        if (!isalnum(tag[i]) && tag[i] != '-' && tag[i] != '_')
            return;
        i++;
    }

    if (test_metadata.nb_tags < TAGS_NB_MAX)
        strncpy(test_metadata.tags[test_metadata.nb_tags++], tag, TAGS_LEN_MAX);
}

void segv_handler(int sig, siginfo_t *unused, void *unused2) {
    wrap_monitoring = false;
    push_info_msg(_("Your code produced a segfault."));
    set_tag("sigsegv");
    wrap_monitoring = true;
    siglongjmp(segv_jmp, 1);
}

void alarm_handler(int sig, siginfo_t *unused, void *unused2)
{
    wrap_monitoring = false;
    push_info_msg(_("Your code exceeded the maximal allowed execution time."));
    set_tag("timeout");
    wrap_monitoring = true;
    siglongjmp(segv_jmp, 1);
}


int sandbox_begin()
{
    // Start timer
    it_val.it_value.tv_sec = 2;
    it_val.it_value.tv_usec = 0;
    it_val.it_interval.tv_sec = 0;
    it_val.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &it_val, NULL);

    // Intercepting stdout and stderr
    dup2(pipe_stdout[1], STDOUT_FILENO);
    dup2(pipe_stderr[1], STDERR_FILENO);
    // Emptying the user pipes
    char buf[BUFSIZ];
    int n;
    while ((n = read(usr_pipe_stdout[0], buf, BUFSIZ)) > 0);
    while ((n = read(usr_pipe_stderr[0], buf, BUFSIZ)) > 0);

    wrap_monitoring = true;
    return 0;
}

void sandbox_fail()
{
    CU_FAIL("Segfault or timeout");
}

void sandbox_end()
{
    wrap_monitoring = false;

    // Remapping stderr to the orignal one ...
    dup2(true_stdout, STDOUT_FILENO); // TODO
    dup2(true_stderr, STDERR_FILENO);

    // ... and looking for a double free warning
    char buf[BUFSIZ];
    int n;
    while ((n = read(pipe_stdout[0], buf, BUFSIZ)) > 0) {
        write(usr_pipe_stdout[1], buf, n);
        write(STDOUT_FILENO, buf, n);
    }


    while ((n = read(pipe_stderr[0], buf, BUFSIZ)) > 0) {
        if (strstr(buf, "double free or corruption") != NULL) {
            CU_FAIL("Double free or corruption");
            push_info_msg(_("Your code produced a double free."));
            set_tag("double_free");
        }
        write(usr_pipe_stderr[1], buf, n);
        write(STDERR_FILENO, buf, n);
    }


    it_val.it_value.tv_sec = 0;
    it_val.it_value.tv_usec = 0;
    it_val.it_interval.tv_sec = 0;
    it_val.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &it_val, NULL);
}


int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void start_test()
{
    bzero(&test_metadata,sizeof(test_metadata));
    bzero(&stats,sizeof(stats));
    bzero(&failures,sizeof(failures));
    bzero(&monitored,sizeof(monitored));
    bzero(&logs,sizeof(logs));
}

int __real_exit(int status);
int __wrap_exit(int status){
    return status;
}

int run_tests(int argc, char *argv[], void *tests[], int nb_tests) {
    for (int i=1; i < argc; i++) {
        if (!strncmp(argv[i], "LANGUAGE=", 9))
                putenv(argv[i]);
    }
    setlocale (LC_ALL, "");
    bindtextdomain("tests", getenv("PWD"));
    bind_textdomain_codeset("messages", "UTF-8");
    textdomain("tests");

    mallopt(M_PERTURB, 142); // newly allocated memory with malloc will be set to ~142

    // Code for detecting properly double free errors
    mallopt(M_CHECK_ACTION, 1); // don't abort if double free
    true_stderr = dup(STDERR_FILENO); // preparing a non-blocking pipe for stderr
    true_stdout = dup(STDOUT_FILENO); // preparing a non-blocking pipe for stderr

    int *pipes[] = {pipe_stderr, pipe_stdout, usr_pipe_stdout, usr_pipe_stderr};
    for(int i=0; i < 4; i++) { // Configuring pipes to be non-blocking
        pipe(pipes[i]);
        int flags = fcntl(pipes[i][0], F_GETFL, 0);
        fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK);
    }
    stdout_cpy = usr_pipe_stdout[0];
    stderr_cpy = usr_pipe_stderr[0];

    putenv("LIBC_FATAL_STDERR_=2"); // needed otherwise libc doesn't print to program's stderr

    /* make sure that we catch segmentation faults */
    struct sigaction sa;

    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    static char stack[SIGSTKSZ];
    stack_t ss = {
        .ss_size = SIGSTKSZ,
        .ss_sp = stack,
    };

    sa.sa_flags     = SA_NODEFER|SA_ONSTACK|SA_RESTART;
    sa.sa_sigaction = segv_handler;
    sigaltstack(&ss, 0);
    sigfillset(&sa.sa_mask);
    int ret = sigaction(SIGSEGV, &sa, NULL);
    if (ret)
        return ret;
    sa.sa_sigaction = alarm_handler;
    ret = sigaction(SIGALRM, &sa, NULL);
    if (ret)
        return ret;


    /* Output file containing succeeded / failed tests */
    FILE* f_out = fopen("results.txt", "w");
    if (!f_out)
        return -ENOENT;


    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    for (int i=0; i < nb_tests; i++) {
        Dl_info  DlInfo;
        if (dladdr(tests[i], &DlInfo) == 0)
            return -EFAULT;

        CU_pTest pTest;
        if ((pTest = CU_add_test(pSuite, DlInfo.dli_sname, tests[i])) == NULL) {
                CU_cleanup_registry();
                return CU_get_error();
        }

        printf("\n==== Results for test %s : ====\n", DlInfo.dli_sname);

        start_test();

        if (CU_basic_run_test(pSuite,pTest) != CUE_SUCCESS)
            return CU_get_error();

        if (test_metadata.err)
            return test_metadata.err;

        int nb = CU_get_number_of_tests_failed();
        if (nb > 0)
            ret = fprintf(f_out, "%s#FAIL#%s#%d#", test_metadata.problem,
                    test_metadata.descr, test_metadata.weight);

        else
            ret = fprintf(f_out, "%s#SUCCESS#%s#%d#", test_metadata.problem,
                    test_metadata.descr, test_metadata.weight);
        if (ret < 0)
            return ret;

        for(int i=0; i < test_metadata.nb_tags; i++) {
            ret = fprintf(f_out, "%s", test_metadata.tags[i]);
            if (ret < 0)
                return ret;

            if (i != test_metadata.nb_tags - 1) {
                ret = fprintf(f_out, ",");
                if (ret < 0)
                    return ret;
            }
        }


        while (test_metadata.fifo_in != NULL) {
            struct info_msg *head = test_metadata.fifo_in;
            ret = fprintf(f_out, "#%s", head->msg);

            if (head->msg != NULL)
                free(head->msg);
            test_metadata.fifo_in = head->next;
            free(head);

            if (ret < 0)
                return ret;
        }

        test_metadata.fifo_out = NULL;
        ret = fprintf(f_out, "\n");
        if (ret < 0)
            return ret;

    }

    fclose(f_out);

    /* Run all tests using the CUnit Basic interface */
    //CU_basic_run_tests();
    //CU_automated_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
