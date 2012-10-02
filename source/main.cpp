
#include "objectscript.h"
#include <histedit.h>
#include <stdio.h>

#ifndef OS_HISTORY_SIZE
#define OS_HISTORY_SIZE 1000
#endif /* !OS_HISTORY_SIZE */


static char*
prompt_cb(EditLine*)
{
    static char promptstr[] = "(os) ";
    return promptstr;
}



int main(int argc, char *argv[])
{

   printf(
         "OS version %s"
         "\n"
         "\n", OS_VERSION);
    HistEvent hevent;
    History *hh(history_init());
    history(hh, &hevent, H_SETUNIQUE, 1);
    history(hh, &hevent, H_SETSIZE, OS_HISTORY_SIZE);

    EditLine *el(el_init("os", stdin, stdout, stderr));
    el_set(el, EL_SIGNAL, 1);
    el_set(el, EL_EDITOR, "emacs");
    el_set(el, EL_PROMPT, prompt_cb);
    el_set(el, EL_HIST, history, hh);

    ObjectScript::OS *shell(ObjectScript::OS::create());

    const char *line = NULL;
    int linelen = -1;

    while ((line = el_gets(el, &linelen)) && linelen > 0) {
        shell->eval(line);
        history(hh, &hevent, H_ENTER, line);
    }

    el_end(el);
    history_end(hh);
    shell->release();
}

