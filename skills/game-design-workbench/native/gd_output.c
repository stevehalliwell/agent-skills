#include "gd.h"
#include <string.h>

void gd_json_string_n(FILE *output, const char *value, size_t size) {
    const unsigned char *p, *end;
    if (!value) { fputs("null", output); return; }
    end = (const unsigned char *)value + size;
    fputc('"', output);
    for (p = (const unsigned char *)value; p < end; ++p) {
        if (*p == '"' || *p == '\\') { fputc('\\', output); fputc(*p, output); }
        else if (*p < 32) fprintf(output, "\\u%04x", *p);
        else fputc(*p, output);
    }
    fputc('"', output);
}
void gd_json_string(FILE *output, const char *value) {
    gd_json_string_n(output,value,value ? strlen(value):0);
}
static void member(FILE *output, const char *key, const char *value, int comma) {
    if (comma) fputc(',', output);
    gd_json_string(output, key); fputc(':', output); gd_json_string(output, value);
}
static int selected(const char *fields, const char *key) {
    size_t n = strlen(key);
    const char *p = fields;
    if (!p) return 1;
    while (*p) {
        const char *end = strchr(p, ','); size_t length = end ? (size_t)(end-p) : strlen(p);
        if (length == n && !memcmp(p, key, n)) return 1;
        if (!end) break;
        p = end + 1;
    }
    return 0;
}
static void record_summary(FILE *output, const GD_Model *model, int index, int connectivity) {
    const GD_Record *r = &model->records[index];
    const GD_Connectivity *c = &model->connectivity[index];
    fputc('{', output);
    member(output, "reference", r->reference, 0); member(output, "name", r->name, 1);
    member(output, "id", r->id, 1); member(output, "description", r->description, 1);
    member(output, "status", r->status, 1); member(output, "source_path", r->source_path, 1);
    if (connectivity) fprintf(output, ",\"partner_count\":%d,\"case_count\":%d,\"active_case_count\":%d", c->partners, c->cases, c->active_cases);
    fputc('}', output);
}
static void records(FILE *output, const GD_Model *model, GD_Role role, int connectivity) {
    int i, comma = 0;
    fputc('[', output);
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == role) {
        if (comma++) fputc(',', output);
        record_summary(output, model, i, connectivity);
    }
    fputc(']', output);
}
static void cases(FILE *output, const GD_Model *model) {
    int i, j, comma = 0;
    fputc('[', output);
    for (i = 0; i < model->record_count; ++i) {
        const GD_Record *r = &model->records[i];
        int edge_comma = 0;
        if (r->role != GD_CASE) continue;
        if (comma++) fputc(',', output);
        fputc('{', output);
        member(output, "reference", r->reference, 0); member(output, "name", r->name, 1);
        member(output, "id", r->id, 1); member(output, "description", r->description, 1);
        member(output, "status", r->status, 1);
        member(output, "coverage", r->coverage, 1); member(output, "source_path", r->source_path, 1);
        member(output, "body", r->body, 1);
        fputs(",\"relationships\":[", output);
        for (j = 0; j < model->edge_count; ++j) if (model->edges[j].source == i) {
            const GD_Edge *edge = &model->edges[j];
            if (edge_comma++) fputc(',', output);
            fputc('{', output); member(output, "field", gd_field_name(edge->field), 0);
            member(output, "reference", edge->reference, 1);
            fprintf(output, ",\"loaded\":%s,\"ambiguous\":%s}", edge->target >= 0 ? "true" : "false", edge->target == -2 ? "true" : "false");
        }
        fputs("]}", output);
    }
    fputc(']', output);
}
static void cells(FILE *output, const GD_Model *model) {
    int i, j;
    fputc('[', output);
    for (i = 0; i < model->cell_count; ++i) {
        const GD_Cell *cell = &model->cells[i];
        if (i) fputc(',', output);
        fputc('{', output); member(output, "noun", model->records[cell->noun].reference, 0);
        member(output, "verb", model->records[cell->verb].reference, 1);
        fprintf(output, ",\"active\":%s,\"cases\":[", cell->active ? "true" : "false");
        for (j = 0; j < cell->pair_count; ++j) {
            if (j) fputc(',', output);
            gd_json_string(output, model->records[model->pairs[cell->first_pair+j].interaction].reference);
        }
        fputs("]}", output);
    }
    fputc(']', output);
}
static void components(FILE *output, const GD_Model *model) {
    int i, j;
    fputc('[', output);
    for (i = 0; i < model->component_count; ++i) {
        const GD_Component *group = &model->components[i];
        if (i) fputc(',', output);
        fputc('{', output); member(output, "anchor", model->records[group->anchor].reference, 0);
        fprintf(output, ",\"%s\":%d,\"members\":[", model->skill_mode ? "link_count" : "active_pair_count", group->edge_count);
        for (j = 0; j < group->member_count; ++j) {
            if (j) fputc(',', output);
            gd_json_string(output, model->records[model->component_members[group->first_member+j]].reference);
        }
        fputs("]}", output);
    }
    fputc(']', output);
}
static void rank_value(FILE *output, int known, int64_t value) {
    if (known) fprintf(output, "%lld", (long long)value); else fputs("null", output);
}
static void skill_records(FILE *output, const GD_Model *model) {
    int i, j, comma = 0;
    fputc('[', output);
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_SKILL) {
        const GD_Record *record = &model->records[i];
        const GD_SkillState *state = &model->skills[i];
        int edge_comma = 0;
        if (comma++) fputc(',', output);
        fputc('{', output); member(output, "reference", record->reference, 0); member(output, "name", record->name, 1);
        member(output, "id", record->id, 1); member(output, "description", record->description, 1);
        member(output, "status", record->status, 1); member(output, "source_path", record->source_path, 1);
        member(output, "body", record->body, 1);
        fputs(",\"authored_rank\":", output); rank_value(output, record->has_rank, record->rank);
        fputs(",\"prerequisite_depth\":", output); if (state->depth >= 0) fprintf(output, "%d", state->depth); else fputs("null", output);
        fputs(",\"minimum_rank\":", output); rank_value(output, state->has_minimum, state->minimum_rank);
        fputs(",\"display_rank\":", output); rank_value(output, state->has_display, state->display_rank);
        fprintf(output, ",\"layout_layer\":%d,\"placement_complete\":%s,\"cycle\":%s,\"rank_conflict\":%s,\"root\":%s,\"declared_prerequisite_count\":%d,\"prerequisite_count\":%d,\"dependent_count\":%d,\"relationships\":[",
                state->layout_layer, state->incomplete ? "false" : "true", state->cycle ? "true" : "false", state->incomplete ? "null" : state->rank_conflict ? "true" : "false",
                state->declared_prerequisites ? "false" : "true", state->declared_prerequisites, state->prerequisite_count, state->dependent_count);
        for (j = 0; j < model->edge_count; ++j) if (model->edges[j].source == i) {
            const GD_Edge *edge = &model->edges[j];
            if (edge_comma++) fputc(',', output);
            fputc('{', output); member(output, "field", gd_field_name(edge->field), 0); member(output, "reference", edge->reference, 1);
            fprintf(output, ",\"loaded\":%s,\"ambiguous\":%s}", edge->target >= 0 ? "true" : "false", edge->target == -2 ? "true" : "false");
        }
        fputs("]}", output);
    }
    fputc(']', output);
}
static void skill_links(FILE *output, const GD_Model *model) {
    int i;
    fputc('[', output);
    for (i = 0; i < model->skill_link_count; ++i) {
        const GD_SkillLink *link = &model->skill_links[i];
        if (i) fputc(',', output);
        fputc('{', output); member(output, "prerequisite", model->records[link->prerequisite].reference, 0);
        member(output, "dependent", model->records[link->dependent].reference, 1); fputc('}', output);
    }
    fputc(']', output);
}
static void findings(FILE *output, const GD_Model *model) {
    int i;
    fputc('[', output);
    for (i = 0; i < model->finding_count; ++i) {
        const GD_Finding *item = &model->findings[i];
        const GD_Record *record = &model->records[item->record];
        if (i) fputc(',', output);
        fputc('{', output); member(output, "code", item->code, 0); member(output, "message", item->message, 1);
        member(output, "severity", !strncmp(item->code, "scaffolding.", 12) ? "info" : !strcmp(item->code, "skill.rank_conflict") ? "warning" : "error", 1);
        member(output, "record", record->reference, 1); member(output, "field", item->field, 1);
        member(output, "reference", item->reference, 1); member(output, "source_path", record->source_path, 1);
        fprintf(output, ",\"line\":%d}", item->line ? item->line : !strcmp(item->field, "rank") ? record->rank_line : 0);
    }
    fputc(']', output);
}
static void route_records(FILE *output, const GD_Model *model, const GD_Routes *routes, GD_Role role) {
    int i, e, comma = 0;
    fputc('[',output);
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == role) {
        const GD_Record *record = &model->records[i]; int edge_comma = 0;
        if (comma++) fputc(',',output);
        fputc('{',output); member(output,"reference",record->reference,0); member(output,"source_path",record->source_path,1);
        member(output,"id",record->id,1); member(output,"status",record->status,1);
        if (role == GD_CHALLENGE) fprintf(output,",\"entry\":%s,\"reachable\":%s",record->entry ? "true":"false",routes && routes->reachable[i] ? "true" : routes && routes->routes_complete ? "false" : "null");
        fputs(",\"relationships\":[",output);
        for (e = 0; e < model->edge_count; ++e) if (model->edges[e].source == i) {
            const GD_Edge *edge = &model->edges[e];
            if (edge_comma++) fputc(',',output);
            fputc('{',output); member(output,"field",gd_field_name(edge->field),0); member(output,"reference",edge->reference,1);
            fprintf(output,",\"loaded\":%s,\"ambiguous\":%s,\"line\":%d}",edge->target>=0 ? "true":"false",edge->target == -2 ? "true":"false",edge->line);
        }
        fputs("]}",output);
    }
    fputc(']',output);
}
int gd_write_routes_result(FILE *output, const GD_Model *model, const GD_Routes *routes, int json, const char *fields) {
    int i, j, comma = 0, complete = routes && !model->error_count && !model->errors_omitted;
    if (!json) {
        fprintf(output,"%d challenges; %d entry challenges; %d demand checks; declared routes %s\n",routes ? routes->challenge_count:0,routes ? routes->entry_count:0,routes ? routes->check_count:0,complete && routes->routes_complete ? "complete":"unknown");
        if (routes) for (i = 0; i < routes->check_count; ++i) {
            const GD_RouteCheck *check = &routes->checks[i];
            fprintf(output,"%s: %s: prior opportunity %s%s\n",model->records[check->challenge].reference,model->records[check->skill].reference,gd_support_name(check->coverage),check->current_introduction ? "; introduced here":"");
        }
        for (i = 0; i < model->finding_count; ++i) fprintf(output,"%s: %s: %s\n",model->findings[i].code,model->records[model->findings[i].record].reference,model->findings[i].message);
        if (model->findings_omitted) fprintf(output,"%d further findings omitted\n",model->findings_omitted);
        return !ferror(output);
    }
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"data\":{",complete ? "true":"false");
    if (selected(fields,"challenges")) { fputs("\"challenges\":",output); route_records(output,model,routes,GD_CHALLENGE); comma = 1; }
    if (selected(fields,"skills")) { if (comma++) fputc(',',output); fputs("\"skills\":",output); route_records(output,model,routes,GD_SKILL); }
    if (selected(fields,"checks")) {
        if (comma++) fputc(',',output); fputs("\"checks\":[",output);
        if (routes) for (i = 0; i < routes->check_count; ++i) {
            const GD_RouteCheck *check = &routes->checks[i];
            if (i) fputc(',',output);
            fprintf(output,"{\"index\":%d,",i); member(output,"challenge",model->records[check->challenge].reference,0);
            member(output,"skill",model->records[check->skill].reference,1); member(output,"demanded_by",model->records[check->demanded_by].reference,1);
            member(output,"demand_field",gd_field_name(check->demand_field),1); member(output,"coverage",gd_support_name(check->coverage),1);
            fprintf(output,",\"demand_line\":%d,\"direct_requires\":%s,\"direct_mastery\":%s,\"prerequisite\":%s,\"current_introduction\":%s,\"supported_route\":%s,\"unsupported_route\":%s}",check->demand_line,check->direct_requires ? "true":"false",check->direct_mastery ? "true":"false",check->prerequisite ? "true":"false",check->current_introduction ? "true":"false",check->supported_route ? "true":"false",check->unsupported_route ? "true":"false");
        }
        fputc(']',output);
    }
    if (selected(fields,"witnesses")) {
        if (comma++) fputc(',',output); fputs("\"witnesses\":[",output);
        if (routes) for (i = 0; i < routes->witness_count; ++i) {
            const GD_RouteWitness *w = &routes->witnesses[i];
            if (i) fputc(',',output);
            fprintf(output,"{\"check\":%d,\"supported\":%s,\"full_length\":%d,\"prefix_truncated\":%s,\"path\":[",w->check,w->supported ? "true":"false",w->full_length,w->full_length > w->length ? "true":"false");
            for (j = 0; j < w->length; ++j) { if (j) fputc(',',output); gd_json_string(output,model->records[w->path[j]].reference); }
            fputs("]}",output);
        }
        fprintf(output,"],\"witnesses_omitted\":%d",routes ? routes->witnesses_omitted:0);
    }
    if (selected(fields,"findings")) {
        if (comma++) fputc(',',output); fputs("\"findings\":",output); findings(output,model);
        fprintf(output,",\"findings_omitted\":%d",model->findings_omitted);
    }
    fprintf(output,"},\"coverage\":{\"complete\":%s,\"operation\":\"scaffolding\",\"routes_complete\":%s,\"requirements_complete\":%s,\"challenge_count\":%d,\"entry_count\":%d,\"check_count\":%d,",complete ? "true":"false",complete && routes->routes_complete ? "true":"false",complete && routes->requirements_complete ? "true":"false",routes ? routes->challenge_count:0,routes ? routes->entry_count:0,routes ? routes->check_count:0);
    member(output,"scope","All supplied statuses; entry-to-visit walks including revisits; earlier introduces/practises only. Not observed learning, physical reachability, or unencoded gate evaluation.",0);
    fprintf(output,",\"limits\":{\"records\":%d,\"relationships\":%d,\"checks\":%d,\"witnesses\":%d,\"witness_path_nodes\":%d},\"collections\":[",GD_MAX_RECORDS,GD_MAX_EDGES,GD_MAX_ROUTE_CHECKS,GD_MAX_ROUTE_WITNESSES,GD_MAX_ROUTE_PATH);
    for (i = 0; i < model->collection_count; ++i) { if (i) fputc(',',output); fputc('{',output); member(output,"alias",model->collections[i].alias,0); member(output,"directory",model->collections[i].directory,1); fputc('}',output); }
    fputs("]},\"errors\":[",output);
    for (i = 0; i < model->error_count; ++i) {
        const GD_Error *error = &model->errors[i]; if (i) fputc(',',output);
        fputc('{',output); member(output,"code",error->code,0); member(output,"message",error->message,1); member(output,"source_path",error->path,1); fprintf(output,",\"line\":%d}",error->line);
    }
    fprintf(output,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"Inspect witness paths and original role/next declarations; no automatic teaching or route repairs\"]}\n",model->errors_omitted);
    return !ferror(output);
}
int gd_write_result(FILE *output, const GD_Model *model, int operation, int json, const char *fields) {
    int i, comma = 0, connectivity = operation == 1, skills = operation == 2;
    if (!json) {
        if (skills) {
            for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_SKILL) {
                const GD_Record *record = &model->records[i]; const GD_SkillState *state = &model->skills[i];
                fprintf(output, "%s: authored rank=", record->reference); rank_value(output, record->has_rank, record->rank);
                fputs("; depth=", output); if (state->depth >= 0) fprintf(output, "%d", state->depth); else fputs("unknown", output);
                fputs("; minimum=", output); rank_value(output, state->has_minimum, state->minimum_rank);
                fputs("; display=", output); rank_value(output, state->has_display, state->display_rank);
                fprintf(output, "; placement=%s%s\n", state->incomplete ? "unknown" : "complete", state->cycle ? "; CYCLE" : state->rank_conflict ? "; RANK CONFLICT" : "");
            }
            for (i = 0; i < model->finding_count; ++i)
                fprintf(output, "%s: %s: %s%s%s\n", model->findings[i].code, model->records[model->findings[i].record].reference,
                        model->findings[i].message, model->findings[i].reference ? " — " : "", model->findings[i].reference ? model->findings[i].reference : "");
            if (model->findings_omitted) fprintf(output, "%d further findings omitted\n", model->findings_omitted);
        } else if (connectivity) {
            for (i = 0; i < model->record_count; ++i) {
                const GD_Record *r = &model->records[i];
                const GD_Connectivity *c = &model->connectivity[i];
                if (r->role == GD_NOUN || r->role == GD_VERB)
                    fprintf(output, "%s: %d active partners; %d active cases; %d total cases\n", r->reference, c->partners, c->active_cases, c->cases);
            }
        } else {
            fprintf(output, "%d declared cells; %d case memberships; %d unplaced cases\n", model->cell_count, model->pair_count, model->unplaced_cases);
            for (i = 0; i < model->cell_count; ++i) {
                const GD_Cell *c = &model->cells[i];
                fprintf(output, "%s x %s: %d cases; active=%s\n", model->records[c->noun].reference,
                        model->records[c->verb].reference, c->pair_count, c->active ? "yes" : "no");
            }
        }
        return !ferror(output);
    }
    fprintf(output, "{\"schema_version\":1,\"ok\":%s,\"data\":{", model->error_count || model->errors_omitted ? "false" : "true");
    if (skills) {
        if (selected(fields, "skills")) { fputs("\"skills\":", output); skill_records(output, model); comma = 1; }
        if (selected(fields, "links")) { if (comma++) fputc(',', output); fputs("\"links\":", output); skill_links(output, model); }
        if (selected(fields, "components")) { if (comma++) fputc(',', output); fputs("\"components\":", output); components(output, model); }
        if (selected(fields, "findings")) { if (comma++) fputc(',', output); fputs("\"findings\":", output); findings(output, model); fprintf(output, ",\"findings_omitted\":%d", model->findings_omitted); }
    } else {
    if (selected(fields, "nouns")) { fputs("\"nouns\":", output); records(output, model, GD_NOUN, connectivity); comma = 1; }
    if (selected(fields, "verbs")) { if (comma++) fputc(',', output); fputs("\"verbs\":", output); records(output, model, GD_VERB, connectivity); }
    if (!connectivity && selected(fields, "cases")) { if (comma++) fputc(',', output); fputs("\"cases\":", output); cases(output, model); }
    if (!connectivity && selected(fields, "cells")) { if (comma++) fputc(',', output); fputs("\"cells\":", output); cells(output, model); }
    if (!connectivity && selected(fields, "components")) { if (comma++) fputc(',', output); fputs("\"components\":", output); components(output, model); }
    }
    fprintf(output, "},\"coverage\":{\"complete\":%s,", model->error_count || model->errors_omitted ? "false" : "true");
    if (skills) fprintf(output, "\"placement_complete\":%s,", model->skill_incomplete || model->error_count || model->errors_omitted ? "false" : "true");
    member(output, "scope", skills ? "supplied skill declarations, all statuses; not schema validation or evidence of learning" : "supplied immediate Markdown files; not schema validation or exhaustive interaction coverage", 0);
    fprintf(output, ",\"record_count\":%d,\"relationship_count\":%d,\"unplaced_case_count\":%d,\"collections\":[", model->record_count, model->edge_count, model->unplaced_cases);
    for (i = 0; i < model->collection_count; ++i) {
        if (i) fputc(',', output);
        fputc('{', output); member(output, "alias", model->collections[i].alias, 0);
        member(output, "directory", model->collections[i].directory, 1); fputc('}', output);
    }
    fprintf(output, "],\"limits\":{\"directory_entries\":%d,\"records\":%d,\"relationships\":%d,\"case_memberships\":%d,\"file_bytes\":%d,\"frontmatter_bytes\":%d,\"retained_text_bytes\":%d}},\"errors\":[",
            GD_MAX_DIRECTORY_ENTRIES, GD_MAX_RECORDS, GD_MAX_EDGES, GD_MAX_PAIRS, GD_MAX_FILE, GD_MAX_FRONTMATTER, GD_TEXT_CAPACITY);
    for (i = 0; i < model->error_count; ++i) {
        const GD_Error *error = &model->errors[i];
        if (i) fputc(',', output);
        fputc('{', output); member(output, "code", error->code, 0); member(output, "message", error->message, 1);
        member(output, "source_path", error->path, 1); fprintf(output, ",\"line\":%d}", error->line);
    }
    fprintf(output, "],\"errors_omitted\":%d,\"breadcrumbs\":[\"Use the same explicit collections with the corresponding CLI or view operation\"]}\n", model->errors_omitted);
    return !ferror(output);
}

int gd_write_mda_result(FILE *output, const GD_Model *model, const GD_Mda *mda, int json, const char *fields) {
    int i, j, comma = 0, complete = mda && mda->query_complete && !model->error_count && !model->errors_omitted;
    int query_references_complete = complete, scoped_links = 0;
    if (mda) for (i = 0; i < mda->link_count; ++i) {
        const GD_MdaLink *link = &mda->links[i]; const GD_Edge *edge = &model->edges[link->edge];
        if (mda->selected[edge->source] && link->resolution != GD_MDA_RESOLVED) query_references_complete = 0;
        if (mda->selected[edge->source] || (edge->target >= 0 && mda->selected[edge->target])) ++scoped_links;
    }
    if (!json) {
        fprintf(output,"%d/%d MDA nodes; %d selected/boundary links; direction %s; root %s\nDeclared associations only; not causation or observed experience.\n",
            mda ? mda->selected_count:0,mda ? mda->node_count:0,scoped_links,mda ? gd_mda_direction_name(mda->direction):"both",mda && mda->root >= 0 ? model->records[mda->root].reference:"all supplied nodes");
        if (mda) {
            for (i = 0; i < model->record_count; ++i) if (mda->selected[i]) fprintf(output,"%s [%s; status %s]: basis %s; aesthetic %s\n",model->records[i].reference,gd_mda_role_name(model->records[i].role),model->records[i].status ? model->records[i].status:"null",gd_mda_basis(model,mda,i),gd_mda_trace(mda,i));
            for (i = 0; i < mda->link_count; ++i) {
                const GD_MdaLink *link = &mda->links[i]; const GD_Edge *edge = &model->edges[link->edge];
                if (mda->selected[edge->source] || (edge->target >= 0 && mda->selected[edge->target])) fprintf(output,"%s -%s-> %s [%s; line %d]\n",model->records[edge->source].reference,gd_field_name(edge->field),edge->reference,gd_mda_resolution_name(link->resolution),edge->line);
            }
            for (i = 0; i < mda->finding_count; ++i) fprintf(output,"%s: %s: %s\n",mda->findings[i].code,model->records[mda->findings[i].record].reference,mda->findings[i].message);
            if (mda->findings_omitted) fprintf(output,"%d further findings omitted\n",mda->findings_omitted);
        }
        return !ferror(output);
    }
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"data\":{",complete ? "true":"false");
    if (selected(fields,"nodes")) {
        fputs("\"nodes\":[",output); comma = 1; j = 0;
        if (mda) for (i = 0; i < model->record_count; ++i) if (mda->selected[i]) {
            const GD_Record *record = &model->records[i]; int e, edge_comma = 0;
            if (j++) fputc(',',output); fputc('{',output);
            member(output,"reference",record->reference,0); member(output,"role",gd_mda_role_name(record->role),1);
            member(output,"name",record->name,1); member(output,"id",record->id,1); member(output,"status",record->status,1);
            member(output,"description",record->description,1); member(output,"source_path",record->source_path,1);
            member(output,"basis",gd_mda_basis(model,mda,i),1); member(output,"aesthetic_trace",gd_mda_trace(mda,i),1);
            member(output,"example_aesthetic",mda->nodes[i].aesthetic >= 0 ? model->records[mda->nodes[i].aesthetic].reference:NULL,1);
            fputs(",\"relationships\":[",output);
            for (e = 0; e < model->edge_count; ++e) if (model->edges[e].source == i) {
                const GD_Edge *edge = &model->edges[e]; if (edge_comma++) fputc(',',output);
                fputc('{',output); member(output,"field",gd_field_name(edge->field),0); member(output,"reference",edge->reference,1);
                fprintf(output,",\"line\":%d,\"loaded\":%s,\"ambiguous\":%s,\"context_only\":%s}",edge->line,edge->target >= 0 ? "true":"false",edge->target == -2 ? "true":"false",record->role == GD_MDA_MECHANIC || edge->field == GD_EXPERIMENTS ? "true":"false");
            }
            fputs("]}",output);
        }
        fputc(']',output);
    }
    if (selected(fields,"links")) {
        if (comma++) fputc(',',output); fputs("\"links\":[",output); j = 0;
        if (mda) for (i = 0; i < mda->link_count; ++i) {
            const GD_MdaLink *link = &mda->links[i]; const GD_Edge *edge = &model->edges[link->edge];
            const GD_Record *source = &model->records[edge->source];
            int target_selected = edge->target >= 0 && mda->selected[edge->target];
            if (!mda->selected[edge->source] && !target_selected) continue;
            if (j++) fputc(',',output); fprintf(output,"{\"index\":%d,",i);
            member(output,"source",source->reference,0); member(output,"field",gd_field_name(edge->field),1); member(output,"reference",edge->reference,1);
            member(output,"target",edge->target >= 0 ? model->records[edge->target].reference:NULL,1);
            member(output,"resolution",gd_mda_resolution_name(link->resolution),1); member(output,"source_path",source->source_path,1);
            member(output,"source_status",source->status,1); member(output,"target_status",edge->target >= 0 ? model->records[edge->target].status:NULL,1);
            fprintf(output,",\"line\":%d,\"source_in_scope\":%s,\"target_in_scope\":%s}",edge->line,mda->selected[edge->source] ? "true":"false",target_selected ? "true":"false");
        }
        fputc(']',output);
    }
    if (selected(fields,"paths")) {
        if (comma++) fputc(',',output); fputs("\"paths\":[",output); j = 0;
        if (mda && mda->root >= 0) for (i = 0; i < model->record_count; ++i) if (mda->selected[i]) {
            int nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full_length, k;
            int count = gd_mda_path(mda,i,nodes,links,GD_MAX_MDA_PATH,&full_length);
            if (j++) fputc(',',output); fputc('{',output); member(output,"to",model->records[i].reference,0);
            fprintf(output,",\"full_length\":%d,\"prefix_truncated\":%s,\"records\":[",full_length,full_length > count ? "true":"false");
            for (k = 0; k < count; ++k) { if (k) fputc(',',output); gd_json_string(output,model->records[nodes[k]].reference); }
            fputs("],\"steps\":[",output);
            for (k = 0; k+1 < count; ++k) {
                const GD_Edge *edge = &model->edges[mda->links[links[k]].edge]; if (k) fputc(',',output);
                fprintf(output,"{\"link\":%d,\"reversed\":%s}",links[k],edge->source == nodes[k] ? "false":"true");
            }
            fputs("]}",output);
        }
        fputc(']',output);
    }
    if (selected(fields,"gaps")) {
        if (comma++) fputc(',',output); fputs("\"gaps\":[",output);
        if (mda) for (i = 0; i < mda->gap_count; ++i) {
            int record = mda->gaps[i]; if (i) fputc(',',output); fputc('{',output);
            member(output,"record",model->records[record].reference,0); member(output,"basis",gd_mda_basis(model,mda,record),1);
            member(output,"aesthetic_trace",gd_mda_trace(mda,record),1); fputc('}',output);
        }
        fputc(']',output);
    }
    if (selected(fields,"findings")) {
        if (comma++) fputc(',',output); fputs("\"findings\":[",output);
        if (mda) for (i = 0; i < mda->finding_count; ++i) {
            const GD_Finding *item = &mda->findings[i]; if (i) fputc(',',output); fputc('{',output);
            member(output,"code",item->code,0); member(output,"record",model->records[item->record].reference,1);
            member(output,"message",item->message,1); member(output,"field",item->field,1); member(output,"reference",item->reference,1);
            member(output,"severity",!strncmp(item->code,"mda.reference_",14) && strcmp(item->code,"mda.reference_unloaded") && strcmp(item->code,"mda.reference_unavailable") ? "error":"info",1);
            member(output,"source_path",model->records[item->record].source_path,1); fprintf(output,",\"line\":%d}",item->line);
        }
        fprintf(output,"],\"findings_omitted\":%d",mda ? mda->findings_omitted:0);
    }
    fprintf(output,"},\"coverage\":{\"operation\":\"mda\",\"complete\":%s,\"query_references_complete\":%s,\"graph_references_complete\":%s,\"selected_node_count\":%d,\"total_node_count\":%d,\"selected_boundary_link_count\":%d,\"total_link_count\":%d,\"query\":{",complete ? "true":"false",query_references_complete ? "true":"false",complete && mda->graph_complete ? "true":"false",mda ? mda->selected_count:0,mda ? mda->node_count:0,scoped_links,mda ? mda->link_count:0);
    member(output,"from",mda && mda->root >= 0 ? model->records[mda->root].reference:NULL,0); member(output,"direction",mda ? gd_mda_direction_name(mda->direction):"both",1);
    fputs("},\"scope\":\"All supplied statuses. Declared associations, not causation, observed dynamics, emotion, guaranteed experience, or whole player story.\",\"paths\":\"One shortest witness per reached node when rooted; all selected and boundary declarations remain in links, not exhaustive path enumeration.\",\"negative_trace_claims\":\"Unknown if any supplied core reference is unresolved; unloaded aliases are not proven missing records.\",\"collections\":[",output);
    for (i = 0; i < model->collection_count; ++i) {
        if (i) fputc(',',output); fputc('{',output); member(output,"alias",model->collections[i].alias,0);
        member(output,"directory",model->collections[i].directory,1); member(output,"role",gd_mda_role_name(model->collections[i].role),1); fputc('}',output);
    }
    fprintf(output,"],\"limits\":{\"records\":%d,\"relationships\":%d,\"witness_path_nodes\":%d,\"findings\":%d}},\"errors\":[",GD_MAX_RECORDS,GD_MAX_EDGES,GD_MAX_MDA_PATH,GD_MAX_ERRORS);
    for (i = 0; i < model->error_count; ++i) {
        const GD_Error *error = &model->errors[i]; if (i) fputc(',',output); fputc('{',output);
        member(output,"code",error->code,0); member(output,"message",error->message,1); member(output,"source_path",error->path,1); fprintf(output,",\"line\":%d}",error->line);
    }
    fprintf(output,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"Use gd view mda with the same explicit mappings, --from reference, and --direction forward|backward|both\"]}\n",model->errors_omitted);
    return !ferror(output);
}

static void prerequisite_values(FILE *output, const GD_Model *model, int record) {
    int i, comma = 0;
    fputc('[', output);
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        if (edge->source != record || edge->field != GD_PREREQUISITES) continue;
        if (comma++) fputc(',', output);
        gd_json_string(output, edge->reference);
    }
    fputc(']', output);
}
int gd_write_draft_result(FILE *output, const GD_Draft *draft, GD_Model *current) {
    const GD_Model *base = draft->baseline, *edited = draft->model;
    int mapping[GD_MAX_RECORDS], i, j, comma = 0;
    int disk_loaded = !current->error_count && !current->errors_omitted;
    for (i = 0; i < base->record_count; ++i) {
        mapping[i] = -1;
        if (!disk_loaded || !(draft->rank_changed[i] || draft->prerequisites_changed[i])) continue;
        for (j = 0; j < current->record_count; ++j) {
            const GD_Record *a = &base->records[i], *b = &current->records[j];
            if (b->role == GD_SKILL && !strcmp(a->reference, b->reference) && !strcmp(a->source_path, b->source_path) &&
                ((!a->id && !b->id) || (a->id && b->id && !strcmp(a->id,b->id)))) { mapping[i] = j; break; }
        }
        if (mapping[i] < 0) gd_error(current, "draft.current_identity", "Edited record is absent, moved, or has changed identity on disk; do not retarget automatically", base->records[i].source_path, 0);
    }
    fprintf(output, "{\"schema_version\":1,\"ok\":%s,\"data\":{\"applied\":false,\"changes\":[", current->error_count || current->errors_omitted ? "false" : "true");
    for (i = 0; i < base->record_count; ++i) {
        int field_comma = 0, index = mapping[i];
        if (!(draft->rank_changed[i] || draft->prerequisites_changed[i])) continue;
        if (comma++) fputc(',', output);
        fputc('{', output); member(output, "reference", base->records[i].reference, 0);
        member(output, "id", base->records[i].id, 1); member(output, "source_path", base->records[i].source_path, 1);
        fprintf(output, ",\"current_disk_available\":%s,\"fields\":{", index >= 0 ? "true" : "false");
        if (draft->rank_changed[i]) {
            fputs("\"rank\":{\"loaded\":", output); rank_value(output, base->records[i].has_rank, base->records[i].rank);
            fputs(",\"current_disk\":", output); rank_value(output, index >= 0 && current->records[index].has_rank, index >= 0 ? current->records[index].rank : 0);
            fputs(",\"edited\":", output); rank_value(output, edited->records[i].has_rank, edited->records[i].rank); fputc('}', output); field_comma = 1;
        }
        if (draft->prerequisites_changed[i]) {
            if (field_comma) fputc(',', output);
            fputs("\"prerequisites\":{\"loaded\":", output); prerequisite_values(output, base, i);
            fputs(",\"current_disk\":", output); if (index >= 0) prerequisite_values(output, current, index); else fputs("null", output);
            fputs(",\"edited\":", output); prerequisite_values(output, edited, i); fputc('}', output);
        }
        fputs("}}", output);
    }
    fputs("],\"affected\":[", output); comma = 0;
    for (i = 0; i < edited->record_count; ++i) {
        const GD_SkillState *state = &edited->skills[i];
        if (edited->records[i].role != GD_SKILL || (!(draft->rank_changed[i] || draft->prerequisites_changed[i]) && !memcmp(state, &base->skills[i], sizeof(*state)))) continue;
        if (comma++) fputc(',', output);
        fputc('{', output); member(output, "reference", edited->records[i].reference, 0);
        fputs(",\"authored_rank\":", output); rank_value(output, edited->records[i].has_rank, edited->records[i].rank);
        fputs(",\"minimum_rank\":", output); rank_value(output, state->has_minimum, state->minimum_rank);
        fputs(",\"display_rank\":", output); rank_value(output, state->has_display, state->display_rank);
        fprintf(output, ",\"placement_complete\":%s,\"cycle\":%s,\"rank_conflict\":%s}", state->incomplete ? "false" : "true", state->cycle ? "true" : "false", state->incomplete ? "null" : state->rank_conflict ? "true" : "false");
    }
    fputs("],\"findings\":", output); findings(output, edited);
    fprintf(output, ",\"findings_omitted\":%d},\"coverage\":{\"complete\":%s,\"placement_complete\":%s,\"operation\":\"skill-atoms.draft-changes\",\"disk_changed_since_load\":%s,\"modified_records\":%d,\"history_limit\":%d,\"collections\":[",
            edited->findings_omitted, current->error_count || current->errors_omitted ? "false" : "true", edited->skill_incomplete ? "false" : "true",
            !disk_loaded ? "null" : base->source_digest != current->source_digest ? "true" : "false", draft->modified_count, GD_MAX_DRAFT_EDITS);
    for (i = 0; i < base->collection_count; ++i) {
        if (i) fputc(',', output);
        fputc('{', output); member(output, "alias", base->collections[i].alias, 0); member(output, "directory", base->collections[i].directory, 1); fputc('}', output);
    }
    fputs("],\"scope\":\"User net edits against loaded baseline; current disk read at finish, not a lock. Agent must recheck before applying.\"},\"errors\":[", output);
    for (i = 0; i < current->error_count; ++i) {
        const GD_Error *error = &current->errors[i];
        if (i) fputc(',', output);
        fputc('{', output); member(output, "code", error->code, 0); member(output, "message", error->message, 1);
        member(output, "source_path", error->path, 1); fprintf(output, ",\"line\":%d}", error->line);
    }
    fprintf(output, "],\"errors_omitted\":%d}\n", current->errors_omitted);
    return !ferror(output);
}

static void control_scalar(FILE *output, const GD_ControlValue *value) {
    if (value->kind == GD_LITERAL_NUMBER) fprintf(output,"%.17g",value->number);
    else if (value->kind == GD_LITERAL_BOOLEAN) fputs(value->number ? "true":"false",output);
    else if (value->kind == GD_LITERAL_STRING) gd_json_string(output,value->string);
    else fputs("null",output);
}
static void csv_string(FILE *output, const char *text) {
    const char *p;
    fputc('"',output);
    for (p=text ? text:"";*p;++p) { if (*p=='"') fputc('"',output); fputc(*p,output); }
    fputc('"',output);
}
int gd_write_controls_result(FILE *output, const GD_Model *model, const GD_Controls *controls,
                             int json, const char *fields, const char *export_format) {
    int i,j,comma=0,complete=controls && controls->complete && !model->error_count && !model->errors_omitted;
    if (export_format && strcmp(export_format,"json") && strcmp(export_format,"csv") && strcmp(export_format,"ini")) return 0;
    if (export_format && complete) {
        int csv=!strcmp(export_format,"csv"), ini=!strcmp(export_format,"ini");
        if (csv) fputs("key,type,value,unit\r\n",output);
        else fputs(ini ? "[controls]\n":"{",output);
        for (i=0;i<model->record_count;++i) if (controls->requested[i]) {
            const GD_Record *record=&model->records[i];
            const GD_ControlValue *value=&controls->values[i];
            if (csv) {
                csv_string(output,record->name); fputc(',',output); csv_string(output,record->control.type); fputc(',',output);
                if (value->kind==GD_LITERAL_STRING) csv_string(output,value->string); else control_scalar(output,value);
                fputc(',',output); csv_string(output,record->control.unit); fputs("\r\n",output);
                if (value->kind==GD_LITERAL_STRING && value->string) {
                    const char *prefix=value->string; while (*prefix==' ') ++prefix;
                    if (*prefix && strchr("=+-@\t\r\n",*prefix))
                        fputs("gd: controls.csv_formula: enum value may be treated as a spreadsheet formula; data unchanged\n",stderr);
                }
            } else {
                if (ini) fprintf(output,"%s=",record->name);
                else { if (comma++) fputc(',',output); gd_json_string(output,record->name); fputc(':',output); }
                control_scalar(output,value); if (ini) fputc('\n',output);
            }
        }
        if (!csv && !ini) fputs("}\n",output);
        return !ferror(output);
    }
    /* Errors use the normal envelope, never a plausible partial game export. */
    if (!json && !export_format) {
        fprintf(output,"controls: %d/%d resolved; complete=%s\n",controls ? controls->resolved_count:0,controls ? controls->selected_count:0,complete ? "true":"false");
        if (controls) for (i=0;i<model->record_count;++i) if (controls->selected[i]) {
            fprintf(output,"%s (%s) = ",model->records[i].reference,model->records[i].control.type ? model->records[i].control.type:"unspecified");
            if (controls->state[i]==1) control_scalar(output,&controls->values[i]); else fputs("unresolved",output);
            fprintf(output," [%s]%s%s\n",model->records[i].control.unit ? model->records[i].control.unit:"",controls->requested[i] ? "":" dependency only",controls->overridden[i] ? " temporary override":"");
        }
        return !ferror(output);
    }
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"data\":{",complete ? "true":"false");
    if (selected(fields,"controls")) {
        fputs("\"controls\":[",output);
        if (controls) for (i=0;i<model->record_count;++i) if (controls->selected[i]) {
            const GD_Record *record=&model->records[i]; const GD_ControlDefinition *definition=&record->control;
            if (comma++) fputc(',',output);
            fputc('{',output); member(output,"reference",record->reference,0); member(output,"id",record->id,1);
            member(output,"status",record->status,1); member(output,"source_path",record->source_path,1);
            member(output,"type",definition->type,1); member(output,"unit",definition->unit,1);
            member(output,"expression",definition->expression,1);
            fprintf(output,",\"requested\":%s,\"resolved\":%s,\"overridden\":%s,\"value\":",controls->requested[i] ? "true":"false",controls->state[i]==1 ? "true":"false",controls->overridden[i] ? "true":"false");
            if (controls->state[i]==1) control_scalar(output,&controls->values[i]); else fputs("null",output);
            fputs(",\"authored_value\":",output);
            if (definition->kind==GD_LITERAL_STRING) gd_json_string(output,definition->value);
            else if (definition->kind==GD_LITERAL_BOOLEAN) fputs(definition->value && (definition->value[0]=='t' || definition->value[0]=='T') ? "true":"false",output);
            else if (definition->kind==GD_LITERAL_NUMBER) { double value; if (gd_control_number(definition->value,&value,0)) fprintf(output,"%.17g",value); else fputs("null",output); }
            else fputs("null",output);
            {
                double bound;
                fputs(",\"minimum\":",output); if (definition->minimum && gd_control_number(definition->minimum,&bound,0)) fprintf(output,"%.17g",bound); else fputs("null",output);
                fputs(",\"maximum\":",output); if (definition->maximum && gd_control_number(definition->maximum,&bound,0)) fprintf(output,"%.17g",bound); else fputs("null",output);
            }
            fputs(",\"dependencies\":[",output);
            {
                int dependency_comma=0;
                for (j=controls->first[i];j<controls->first[i]+controls->count[i];++j) if (controls->tokens[j].target>=0) {
                    const GD_ControlToken *token=&controls->tokens[j];
                    if (dependency_comma++) fputc(',',output);
                    fputc('{',output); member(output,"reference",model->records[token->target].reference,0); fprintf(output,",\"offset\":%d}",token->offset);
                }
            }
            fputs("],\"allowed_values\":[",output);
            {
                const char *allowed=definition->allowed;
                for (j=0;j<definition->allowed_count;++j,allowed+=strlen(allowed)+1) { if (j) fputc(',',output); gd_json_string(output,allowed); }
            }
            fprintf(output,"],\"expression_line\":%d,\"value_line\":%d}",definition->lines[2],definition->lines[1]);
        }
        fputc(']',output);
    }
    fprintf(output,"},\"coverage\":{\"operation\":\"controls\",\"complete\":%s,\"selected_count\":%d,\"resolved_count\":%d,\"tokens\":%d,\"token_limit\":%d,\"expression_token_limit\":%d,\"stack_limit\":%d,\"unit_term_limit\":%d,\"scope\":\"Selected controls and required dependencies; temporary overrides never mutate source; binary64 arithmetic, no implicit unit conversion\"},\"errors\":[",complete ? "true":"false",controls ? controls->selected_count:0,controls ? controls->resolved_count:0,controls ? controls->token_count:0,GD_MAX_CONTROL_TOKENS,GD_MAX_EXPRESSION_TOKENS,GD_MAX_EXPRESSION_STACK,GD_MAX_UNIT_TERMS);
    for (i=0;i<model->error_count;++i) {
        const GD_Error *error=&model->errors[i]; if (i) fputc(',',output);
        fputc('{',output); member(output,"code",error->code,0); member(output,"message",error->message,1);
        member(output,"source_path",error->path,1); fprintf(output,",\"line\":%d}",error->line);
    }
    fprintf(output,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"Inspect controls or export --as json|csv|ini with the same explicit mappings; overrides are temporary\"]}\n",model->errors_omitted);
    return !ferror(output);
}

int gd_write_control_import(FILE *output, const GD_Model *model, const GD_ControlImport *plan, int json, const char *fields) {
    int i,comma=0,complete=plan && plan->complete && !model->error_count && !model->errors_omitted;
    if (!json) {
        fprintf(output,"controls import: %d literal changes; %d derived inputs; ready=%s; applied=false\n",plan ? plan->changed_count:0,plan ? plan->derived_count:0,complete ? "true":"false");
        if (plan) for (i=0;i<plan->count;++i) {
            const GD_ControlInput *entry=&plan->inputs[i];
            if (!entry->changed) continue;
            fprintf(output,"%s: %s%s\n",model->records[entry->record].reference,entry->text,entry->derived ? " (formula preserved)":" (proposed literal)");
        }
        return !ferror(output);
    }
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"data\":{\"applied\":false,\"ready\":%s",complete ? "true":"false",complete ? "true":"false");
    if (selected(fields,"changes")) fputs(",\"changes\":[",output);
    if (selected(fields,"changes") && plan) for (i=0;i<plan->count;++i) {
        const GD_ControlInput *entry=&plan->inputs[i]; const GD_Record *record=&model->records[entry->record];
        if (!entry->changed || entry->derived) continue;
        if (comma++) fputc(',',output);
        fputc('{',output); member(output,"reference",record->reference,0); member(output,"id",record->id,1); member(output,"source_path",record->source_path,1);
        fputs(",\"current\":",output); control_scalar(output,&plan->current->values[entry->record]);
        member(output,"imported_text",entry->text,1);
        fputs(",\"candidate\":",output); if (plan->candidate && plan->candidate->state[entry->record]==1) control_scalar(output,&plan->candidate->values[entry->record]); else fputs("null",output);
        fprintf(output,",\"input_line\":%d}",entry->line);
    }
    if (selected(fields,"changes")) fputc(']',output);
    if (selected(fields,"derived")) fputs(",\"derived\":[",output); comma=0;
    if (selected(fields,"derived") && plan) for (i=0;i<plan->count;++i) if (plan->inputs[i].derived) {
        const GD_ControlInput *entry=&plan->inputs[i];
        if (comma++) fputc(',',output);
        fputc('{',output); member(output,"reference",model->records[entry->record].reference,0); member(output,"imported_text",entry->text,1);
        fprintf(output,",\"changed_input\":%s,\"conflict\":%s,\"current\":",entry->changed ? "true":"false",entry->conflict ? "true":"false");
        control_scalar(output,&plan->current->values[entry->record]); fputs(",\"candidate\":",output);
        if (plan->candidate && plan->candidate->state[entry->record]==1) control_scalar(output,&plan->candidate->values[entry->record]); else fputs("null",output);
        fputc('}',output);
    }
    /* An invalid batch never exposes an executable valid-subset update array. */
    if (selected(fields,"derived")) fputc(']',output);
    if (selected(fields,"items")) fputs(",\"items\":[",output); comma=0;
    if (selected(fields,"items") && complete) for (i=0;i<plan->count;++i) if (plan->inputs[i].changed && !plan->inputs[i].derived) {
        int record=plan->inputs[i].record;
        if (comma++) fputc(',',output);
        fputc('{',output); member(output,"name",model->records[record].name,0); fputs(",\"fields\":{\"value\":",output);
        control_scalar(output,&plan->candidate->values[record]); fputs("}}",output);
    }
    if (selected(fields,"items")) fputc(']',output);
    fprintf(output,",\"source_digest\":\"%016llx\"},\"coverage\":{\"operation\":\"controls.import\",\"complete\":%s,\"input_count\":%d,\"changed_count\":%d,\"validated_count\":%d,\"scope\":\"Current selected export and dependencies; also validate status-admitted declared dependents of changed literals. No historical baseline, new controls, formula replacement, source writes, or proof of cross-file atomicity. Digest is a change indicator, not a security hash.\"},\"errors\":[",(unsigned long long)model->source_digest,complete ? "true":"false",plan ? plan->count:0,plan ? plan->changed_count:0,plan && plan->candidate ? plan->candidate->selected_count:0);
    for (i=0;i<model->error_count;++i) {
        const GD_Error *error=&model->errors[i]; if (i) fputc(',',output);
        fputc('{',output); member(output,"code",error->code,0); member(output,"message",error->message,1); member(output,"source_path",error->path,1); fprintf(output,",\"line\":%d}",error->line);
    }
    fprintf(output,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"Review proposed values and source freshness before authorized public-Attendant update; this plan does not apply itself\"]}\n",model->errors_omitted);
    return !ferror(output);
}
