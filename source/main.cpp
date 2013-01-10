

#ifdef OS_USE_LIBEDIT
# include <histedit.h>
# ifndef  OS_HISTORY_SIZE
#  define OS_HISTORY_SIZE 1000
# endif /* !OS_HISTORY_SIZE */
#else  /* OS_USE_LIBEDIT */
# include <unistd.h>
# define EditLine void
# define _GNU_SOURCE 1  /* Neded for getline() */
#endif /* OS_USE_LIBEDIT */

#include <stdio.h>
#include "objectscript.h"

static char*
prompt_cb(EditLine*)
{
    static char promptstr[] = "(os) ";
    return promptstr;
}


int main(int argc, char *argv[])
{
    ObjectScript::OS *shell(ObjectScript::OS::create());

#ifdef OS_USE_LIBEDIT
    HistEvent hevent;
    History *hh(history_init());
    history(hh, &hevent, H_SETUNIQUE, 1);
    history(hh, &hevent, H_SETSIZE, OS_HISTORY_SIZE);

    EditLine *el(el_init("os", stdin, stdout, stderr));
    el_set(el, EL_SIGNAL, 1);
    el_set(el, EL_EDITOR, "emacs");
    el_set(el, EL_PROMPT, prompt_cb);
    el_set(el, EL_HIST, history, hh);

    const char *line = NULL;
    int linelen = -1;
    while ((line = el_gets(el, &linelen)) && linelen > 0) {
        shell->eval(line);
        history(hh, &hevent, H_ENTER, line);
    }
    el_end(el);
    history_end(hh);
#else  /* OS_USE_LIBEDIT */
    char *line = NULL;
    size_t len;

    bool tty_input = isatty(STDIN_FILENO);

    if (tty_input)
        printf ("%s", prompt_cb(NULL));

    while (getline(&line, &len, stdin) != -1) {
        shell->eval(line);
        if (tty_input)
            printf("%s", prompt_cb(NULL));
    }

    if (line)
        free(line);
#endif /* OS_USE_LIBEDIT */

    shell->release();
}

