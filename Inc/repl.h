#ifndef repl_h
#define repl_h

enum ReplSignals {
    DUMMY_SIG = Q_USER_SIG,
    MAX_PUB_SIG,          /* the last published signal */

    TIMEOUT_SIG,
    MAX_SIG               /* the last signal */
};

void Repl_ctor(void);
extern QActive * const AO_Repl; /* opaque pointer */

#endif /* repl_h */
