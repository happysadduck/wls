#ifndef CONFIG_H
#define CONFIG_H

#define TOTAL_MEM 655360
#define LAYER_BUC_CNT 64
#define FACT_BUC_CNT 5088
#define PARA_BUC_CNT 2544
#define PROP_BUC_CNT 2544
#define MAX_SNODES 32
#define MAX_TNODES 5088

#define HELLO_WORDS "welcome to wolfarch logic system v26.1"

#define CONFLICT_MESSAGE "A_CONFLICT_HAS_OCCURRED!"

#define ANY_PROP_SIGN "any"
#define EXIST_PROP_SIGN "exist"

#define IMPORT_KEY "import"
#define GIVEN_KEY "given"
#define PROVE_KEY "prove"
#define SUPPOSE_KEY "suppose"
#define DEBUG_KEY "debug"
#define QUIT_KEY "quit"
#define SURRENDER_KEY "surrender"
#define EMPTY_KEY "empty"

#define NLEG_INV_CMD "parse faild: not legal prop invocation structure"
#define PROP_NFOUND "bad: no such proposition"
#define WPAR_CNT "parse failed: too few or too many parameters"
#define REDEF_PAR "bad: repieition define"
#define LACK_PRM "bad: lack necessary premise"
#define NLEG_DEF_CMD "parse failed: not legal prop definition structure"
#define MIX_IGPS                                                               \
    "keyword error: you can only import/given at the beginning of a proof"
#define KWORD_NFOUND "keyword error: no such keyword"
#define LACK_CON_PAR                                                           \
    "construction prove failed: not all target params have been constructed"
#define LACK_CON_FET                                                           \
    "construction prove failed: not all features of params are proved"
#define SUC_PROV_PRE "successfully proved"
#define NSTACK "memory used up"
#define NPOOL "pool used up"

#endif
