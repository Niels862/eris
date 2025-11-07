#include "frontend/semantic-analyzer.h"

typedef struct eris_analyzer_t eris_analyzer_t;



typedef void(*eris_analyzer_visit_t)(eris_analyzer_t *an, void *node);

struct eris_analyzer_t {
    eris_analyzer_visit_t *visitors;
};

void eris_analyze_semantics(eris_node_source_t *root) {
    (void)root;
}
