#include "gd_resource_graph.h"
#include <string.h>
#include <assert.h>

static const char *const node_names[]={"case","facet","resource_role"};
static const char *const relation_names[]={"requirement","exclusion","state_write","quantity_write","role_binding","preservation","invocation"};
static void member(FILE *out, const char *key, const char *value) {
    fputc(',',out); gd_json_string(out,key); fputc(':',out); gd_json_string(out,value);
}
static int selected(const char *fields, const char *key) {
    const char *p=fields; size_t size=strlen(key);
    if (!p) return 1;
    while (*p) {
        const char *end=strchr(p,','); size_t length=end ? (size_t)(end-p):strlen(p);
        if (length==size && !memcmp(p,key,size)) return 1;
        if (!end) break; p=end+1;
    }
    return 0;
}
/* Opaque identity components: percent-escaped UTF-8, not array offsets, paths
   or hashes. Loader rejects duplicate qualified source identities. Unresolved
   facet occurrences stay distinct; no axis identity inferred from spelling. */
static void component(FILE *out, const char *text) {
    const unsigned char *p=(const unsigned char *)(text ? text:"");
    for (;*p;++p) {
        if ((*p>='a' && *p<='z') || (*p>='A' && *p<='Z') || (*p>='0' && *p<='9') || *p=='-' || *p=='_' || *p=='.') fputc(*p,out);
        else fprintf(out,"%%%02X",(unsigned)*p);
    }
}
static void record_identity(FILE *out, const GD_Model *model, int index) {
    component(out,model->records[index].reference);
}
static int case_record(const GD_ResourceGraph *graph, int index) { return graph->rules->sources[graph->rules->cases[index].source].record; }
static void case_identity(FILE *out, const GD_ResourceGraph *graph, int index) {
    record_identity(out,graph->model,case_record(graph,index)); fputc('#',out); component(out,graph->rules->cases[index].name);
}
static int local_declaration(const GD_ResourceGraph *graph, const GD_ResourceRelation *edge) {
    const GD_RuleParticipant *actor=&graph->rules->participants[edge->participant];
    switch (edge->kind) {
        case GD_RESOURCE_REQUIREMENT: return graph->rules->condition_tokens[edge->declaration].offset;
        case GD_RESOURCE_EXCLUSION: return graph->rules->condition_tokens[edge->declaration].offset;
        case GD_RESOURCE_STATE_WRITE: return edge->declaration-actor->first_state_effect;
        case GD_RESOURCE_QUANTITY_WRITE: return edge->declaration-actor->first_assignment;
        case GD_RESOURCE_PRESERVATION: return edge->declaration-actor->first_preservation;
        default: return 0;
    }
}
static void node_identity(FILE *out, const GD_ResourceGraph *graph, const int *first_relation, int index) {
    const GD_ResourceNode *node=&graph->nodes[index];
    fputc('"',out); fputs(node_names[node->kind],out); fputc(':',out);
    if (node->kind==GD_RESOURCE_CASE) case_identity(out,graph,node->case_index);
    else if (node->kind==GD_RESOURCE_ROLE) {
        component(out,node->pool_reference);
    } else {
        if (node->pool_reference) { fputs("pool:",out); component(out,node->pool_reference); }
        else { case_identity(out,graph,node->case_index); fputc(':',out); component(out,graph->rules->participants[node->participant].name); }
        fputc(':',out);
        if (node->axis>=0) record_identity(out,graph->model,node->axis);
        else {
            const GD_ResourceRelation *edge;
            assert(first_relation[index]>=0 && first_relation[index]<graph->relation_count);
            edge=&graph->relations[first_relation[index]];
            fprintf(out,"unresolved:%s:%d",relation_names[edge->kind],local_declaration(graph,edge));
        }
    }
    fputc('"',out);
}
static void source(FILE *out, const GD_Model *model, int record, int line) {
    const GD_Record *item=&model->records[record];
    fputs("{\"record\":",out); gd_json_string(out,item->reference);
    member(out,"record_id",item->id); member(out,"source_path",item->source_path);
    member(out,"status",item->status); member(out,"coverage",item->coverage); fprintf(out,",\"line\":%d}",line);
}
static void case_declaration(FILE *out, const GD_ResourceGraph *graph, int index) {
    const GD_RuleCase *item=&graph->rules->cases[index]; int i;
    fputs("{\"name\":",out); gd_json_string(out,item->name); member(out,"context",item->context);
    member(out,"unknown_conditions",item->unknown_conditions); member(out,"unknown_effects",item->unknown_effects);
    fputs(",\"participants\":[",out);
    for (i=item->first_participant;i<item->first_participant+item->participant_count;++i) {
        const GD_RuleParticipant *actor=&graph->rules->participants[i];
        if (i!=item->first_participant) fputc(',',out);
        fputs("{\"name\":",out); gd_json_string(out,actor->name); member(out,"noun",actor->noun);
        member(out,"noun_resolution",gd_rule_resolution_name(actor->noun_resolution));
        member(out,"requires",actor->requires); member(out,"skip_if",actor->skip_if); member(out,"other_axes",actor->other_axes);
        member(out,"becomes",actor->becomes); member(out,"causes",actor->causes); member(out,"preserve",actor->preserve);
        fprintf(out,",\"line\":%d}",actor->line);
    }
    fputs("]}",out);
}
static void node(FILE *out, const GD_ResourceGraph *graph, const int *first_relation, int index) {
    const GD_ResourceNode *item=&graph->nodes[index];
    fputs("{\"id\":",out); node_identity(out,graph,first_relation,index); member(out,"kind",node_names[item->kind]);
    if (item->kind==GD_RESOURCE_CASE) {
        fputs(",\"source\":",out); source(out,graph->model,case_record(graph,item->case_index),graph->rules->cases[item->case_index].line);
        fputs(",\"declaration\":",out); case_declaration(out,graph,item->case_index);
    } else if (item->kind==GD_RESOURCE_ROLE) {
        member(out,"reference",item->pool_reference); member(out,"resolution",gd_rule_resolution_name(item->resolution));
        if (item->record>=0) {
            const GD_Record *record=&graph->model->records[item->record];
            fputs(",\"source\":",out); source(out,graph->model,item->record,0);
            member(out,"declared_noun",record->resource_noun); member(out,"description",record->description); member(out,"body",record->body);
        }
    } else {
        const GD_RuleParticipant *actor=&graph->rules->participants[item->participant];
        member(out,"axis",item->axis>=0 ? graph->model->records[item->axis].reference:NULL);
        member(out,"unit",item->axis>=0 ? graph->model->records[item->axis].axis.unit:NULL);
        member(out,"pool",item->pool_reference);
        /* Shared facets have several contributors. This is representative
           provenance, not a declaration that the first actor owns the pool. */
        fputs(",\"representative_declaration\":{\"source\":",out);
        source(out,graph->model,case_record(graph,item->case_index),actor->line);
        member(out,"case",graph->rules->cases[item->case_index].name); member(out,"participant",actor->name); fputc('}',out);
        member(out,"identity_scope",item->pool_reference ? "explicit_pool_axis":item->axis>=0 ? "case_participant_axis":"unresolved_declaration_occurrence");
    }
    fputc('}',out);
}
static void target(FILE *out, const GD_Model *model, GD_RuleTarget value) {
    fputs(",\"target\":{\"resolution\":",out); gd_json_string(out,gd_rule_resolution_name(value.resolution));
    member(out,"axis",value.axis_record>=0 ? model->records[value.axis_record].reference:NULL);
    member(out,"state",value.state); fputc('}',out);
}
static void relation(FILE *out, const GD_ResourceGraph *graph, const int *first_relation, int index) {
    const GD_ResourceRelation *edge=&graph->relations[index]; const GD_Rules *rules=graph->rules;
    const GD_RuleParticipant *actor=&rules->participants[edge->participant];
    int case_index=edge->kind==GD_RESOURCE_ROLE_BINDING ? graph->nodes[edge->facet].case_index:graph->nodes[edge->activation].case_index;
    fputs("{\"kind\":",out); gd_json_string(out,relation_names[edge->kind]);
    fputs(",\"activation\":",out); node_identity(out,graph,first_relation,edge->activation);
    fputs(",\"facet\":",out); node_identity(out,graph,first_relation,edge->facet);
    fputs(",\"source\":",out); source(out,graph->model,case_record(graph,case_index),edge->line);
    member(out,"case",rules->cases[case_index].name); member(out,"participant",actor->name);
    fprintf(out,",\"conditional_preservation\":%s",edge->conditional ? "true":"false");
    member(out,"meaning",edge->kind==GD_RESOURCE_INVOCATION ? "guarded_case_reference_not_flattened":edge->kind==GD_RESOURCE_ROLE_BINDING ? "identity_binding_not_transfer":"direct_declaration_not_independent_flow");
    if (edge->kind==GD_RESOURCE_REQUIREMENT || edge->kind==GD_RESOURCE_EXCLUSION) {
        const GD_RuleConditionToken *token=&rules->condition_tokens[edge->declaration];
        fputs(",\"text\":",out); gd_json_string_n(out,token->text,token->length); target(out,graph->model,token->target);
        fprintf(out,",\"condition_offset\":%d",token->offset);
    } else if (edge->kind==GD_RESOURCE_STATE_WRITE) {
        const GD_RuleStateEffect *effect=&rules->state_effects[edge->declaration];
        fputs(",\"text\":",out); gd_json_string_n(out,effect->text,effect->length); target(out,graph->model,effect->target);
    } else if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) {
        const GD_RuleAssignment *assignment=&rules->assignments[edge->declaration];
        member(out,"declared_axis",assignment->axis); member(out,"expression",assignment->expression); target(out,graph->model,assignment->target);
        fprintf(out,",\"delta\":{\"recognized\":%s,\"known\":%s,\"checks_complete\":%s,\"number\":",
            edge->delta_recognized ? "true":"false",edge->delta.known ? "true":"false",edge->delta.checks_complete ? "true":"false");
        if (edge->delta.known) fprintf(out,"%.17g",edge->delta.number); else fputs("null",out);
        member(out,"state",edge->delta.state==GD_EXPRESSION_VALUE ? "resolved":edge->delta.state==GD_EXPRESSION_UNIT_ERROR ? "unit_error":edge->delta.state==GD_EXPRESSION_ARITHMETIC_ERROR ? "arithmetic_error":"unknown");
        fputc('}',out);
    } else if (edge->kind==GD_RESOURCE_PRESERVATION) {
        const GD_RulePreservation *promise=&rules->preservations[edge->declaration];
        fputs(",\"text\":",out); gd_json_string_n(out,promise->text,promise->length);
        int axis=graph->nodes[edge->facet].axis;
        member(out,"preservation",promise->kind==GD_PRESERVE_ALL ? "all":promise->kind==GD_PRESERVE_OTHER ? "other":"axis"); target(out,graph->model,promise->target);
        member(out,"expanded_axis",axis>=0 ? graph->model->records[axis].reference:NULL);
    } else if (edge->kind==GD_RESOURCE_ROLE_BINDING) {
        const GD_RulePool *pool=&rules->pools[edge->declaration];
        member(out,"declared_axis",pool->axis); member(out,"reference",pool->reference); member(out,"resource_resolution",gd_rule_resolution_name(pool->resource_resolution));
        member(out,"identity_resolution",gd_rule_resolution_name(pool->identity_resolution)); target(out,graph->model,pool->target);
    } else if (edge->kind==GD_RESOURCE_INVOCATION) {
        const GD_RuleCall *call=&graph->calls->calls[edge->declaration];
        const GD_RuleCase *callee=&rules->cases[graph->nodes[edge->facet].case_index]; int i,comma=0;
        fputs(",\"text\":",out); gd_json_string_n(out,call->text,call->length);
        member(out,"resolution",gd_rule_resolution_name(call->resolution)); fputs(",\"bindings\":[",out);
        for (i=call->first_binding;i<call->first_binding+call->binding_count;++i) {
            const GD_RuleBinding *binding=&graph->calls->bindings[i];
            if (binding->callee<callee->first_participant || binding->callee>=callee->first_participant+callee->participant_count) continue;
            if (comma++) fputc(',',out);
            fputs("{\"callee\":",out); gd_json_string(out,rules->participants[binding->callee].name);
            member(out,"caller",rules->participants[binding->caller].name); fputc('}',out);
        }
        fputc(']',out);
    }
    fputc('}',out);
}
int gd_write_resource_result(FILE *out, const GD_Model *model, const GD_ResourceGraph *graph, int json, const char *fields) {
    int first_relation[GD_RESOURCE_GRAPH_NODES],i,comma=0;
    int ok=!model->error_count && !model->errors_omitted && graph;
    if (graph) {
        for (i=0;i<graph->node_count;++i) first_relation[i]=-1;
        for (i=0;i<graph->relation_count;++i) if (first_relation[graph->relations[i].facet]<0) first_relation[graph->relations[i].facet]=i;
    }
    if (!json) {
        fprintf(out,"Resources: %d retained nodes, %d retained relations. Direct declarations only; all supplied statuses retained.\n",graph ? graph->node_count:0,graph ? graph->relation_count:0);
        fputs("Incidence is not production flow. Conditions remain whole-case guards; predicates are not consumed inputs. Directed routes and composed outcomes unavailable. Signed deltas are not final bounded outcomes; applicability not evaluated.\n",out);
        if (graph) {
            fprintf(out,"Graph limited: %s; construction work %d/%d; unexamined cases %d; roles %d; unresolved facets %d.\n",graph->limited ? "yes":"no",graph->work,GD_RESOURCE_GRAPH_WORK,graph->unexamined_cases,graph->unexamined_roles,graph->unresolved_facets);
            fprintf(out,"Unknown case coverage %d; unexamined invocation bindings %d; preservation %d; findings omitted %d. Call bindings attached: %s; call compile limited: %s; quantity limited: %s.\n",
                graph->unknown_case_coverage,graph->composition_unexamined,graph->preservation_unexamined,graph->rules->findings_omitted,
                graph->calls_bound ? "yes":"no",graph->calls && graph->calls->compile_limited ? "yes":"no",graph->rules->quantity_limited ? "yes":"no");
            fprintf(out,"Binding completeness: axis definitions %s; conditions %s; effects %s; pools %s.\n",
                graph->rules->axis_definitions_complete ? "complete":"incomplete",graph->rules->condition_bindings_complete ? "complete":"incomplete",
                graph->rules->effect_bindings_complete ? "complete":"incomplete",graph->rules->pool_bindings_complete ? "complete":"incomplete");
            for (i=0;i<graph->node_count;++i) { node(out,graph,first_relation,i); fputc('\n',out); }
            for (i=0;i<graph->relation_count;++i) { relation(out,graph,first_relation,i); fputc('\n',out); }
            for (i=0;i<graph->rules->finding_count;++i) fprintf(out,"%s: %s\n",graph->rules->findings[i].code,graph->rules->findings[i].message);
        }
        return !ferror(out);
    }
    fprintf(out,"{\"schema_version\":1,\"ok\":%s,\"data\":{",ok ? "true":"false");
    if (selected(fields,"nodes")) {
        fputs("\"nodes\":[",out); if (graph) for (i=0;i<graph->node_count;++i) { if (i) fputc(',',out); node(out,graph,first_relation,i); }
        fputc(']',out); comma=1;
    }
    if (selected(fields,"relations")) {
        if (comma++) fputc(',',out); fputs("\"relations\":[",out);
        if (graph) for (i=0;i<graph->relation_count;++i) { if (i) fputc(',',out); relation(out,graph,first_relation,i); } fputc(']',out);
    }
    if (selected(fields,"routes")) {
        if (comma++) fputc(',',out);
        fputs("\"routes\":{\"available\":false,\"reason\":\"Directed conditional routes and composed outcomes are not implemented; this is a direct-declaration incidence report, not a no-path result.\"}",out);
    }
    if (selected(fields,"findings")) {
        if (comma++) fputc(',',out); fputs("\"findings\":[",out);
        if (graph) for (i=0;i<graph->rules->finding_count;++i) {
            const GD_Finding *finding=&graph->rules->findings[i]; if (i) fputc(',',out);
            fputs("{\"source\":",out); source(out,model,finding->record,finding->line);
            member(out,"code",finding->code); member(out,"message",finding->message); member(out,"field",finding->field); member(out,"reference",finding->reference); fputc('}',out);
        }
        fputc(']',out);
    }
    fprintf(out,"},\"coverage\":{\"operation\":\"resources\",\"complete\":false,\"scope\":\"direct_declarations_not_flow\",\"node_count\":%d,\"relation_count\":%d,\"graph_limited\":%s,\"work\":%d,",
        graph ? graph->node_count:0,graph ? graph->relation_count:0,graph && graph->limited ? "true":"false",graph ? graph->work:0);
    fprintf(out,"\"unexamined_cases\":%d,\"unexamined_roles\":%d,\"invocations_unexamined\":%d,\"preservation_unexamined\":%d,\"unresolved_facets\":%d,\"unknown_case_coverage\":%d,\"findings_omitted\":%d,",
        graph ? graph->unexamined_cases:0,graph ? graph->unexamined_roles:0,graph ? graph->composition_unexamined:0,graph ? graph->preservation_unexamined:0,
        graph ? graph->unresolved_facets:0,graph ? graph->unknown_case_coverage:0,graph ? graph->rules->findings_omitted:0);
    fprintf(out,"\"axis_definitions_complete\":%s,\"condition_bindings_complete\":%s,\"effect_bindings_complete\":%s,\"pool_bindings_complete\":%s,\"quantity_limited\":%s,",
        graph && graph->rules->axis_definitions_complete ? "true":"false",graph && graph->rules->condition_bindings_complete ? "true":"false",
        graph && graph->rules->effect_bindings_complete ? "true":"false",graph && graph->rules->pool_bindings_complete ? "true":"false",graph && graph->rules->quantity_limited ? "true":"false");
    fprintf(out,"\"call_bindings_attached\":%s,\"call_compile_limited\":%s,",graph && graph->calls_bound ? "true":"false",graph && graph->calls && graph->calls->compile_limited ? "true":"false");
    fputs("\"routes_available\":false,\"composed_outcomes_available\":false,\"applicability_evaluated\":false,\"deltas_are_final_outcomes\":false,\"condition_semantics\":\"Raw whole-case context, requirements and exclusions retained on case nodes; individual incidence relations are not independent AND requirements or applicable outcomes. All statuses retained, no route eligibility implied. Use rules for compiled conditions and explicit scenario evaluation.\",",out);
    fprintf(out,"\"limits\":{\"nodes\":%d,\"relations\":%d,\"construction_work\":%d}},\"errors\":[",GD_RESOURCE_GRAPH_NODES,GD_RESOURCE_GRAPH_RELATIONS,GD_RESOURCE_GRAPH_WORK);
    for (i=0;i<model->error_count;++i) {
        const GD_Error *error=&model->errors[i]; if (i) fputc(',',out);
        fputs("{\"code\":",out); gd_json_string(out,error->code); member(out,"message",error->message); member(out,"source_path",error->path); fprintf(out,",\"line\":%d}",error->line);
    }
    fprintf(out,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"Use the same explicit bindings with rules for typed declarations and conditional scenario evaluation; resource routes remain unavailable.\"]}\n",model->errors_omitted);
    return !ferror(out);
}
