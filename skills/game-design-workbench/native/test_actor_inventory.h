#include "gd_actor_inventory.h"
#include "gd_structure_cli.h"
#include "gd_actor_identity.h"
#include "test_structure_memory.h"
TEST(actor_inventory_local_bindings_declarations_and_work,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorInventory *inventory,*partial;
    GD_Allocator allocator; RouteFailure failure; int i,j,root,first,second,kinds[6]={0},work;
    char id[GD_MAX_PATH],other[GD_MAX_PATH];
    TEST_ASSERT(model && setup("actor-inventory")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"unit","---\nid: unit-definition\naxes: [actions/Life, actions/Life]\n---\n"));
    TEST_ASSERT(write_record(0,"unused","---\naxes: []\n---\n"));
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(1,"Contact","---\nvalue_type: enum\nallowed_values: [near]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nid: source-identity\ncontext_axes: [actions/Contact]\n---\n```gd-rule\n"
        "case shared:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\n    B:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\n"
        "case caller:\n    target: Left\n    context: ANY\n    Left:\n        noun: things/unit\n        requires: ANY\n        causes: cases/rules#shared with A=Left, B=Right; cases/rules#shared with A=Left, B=Right\n        preserve: ALL\n    Right:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count && analysis->call_count==2);
    inventory=gd_actor_inventory_build(model->allocator,analysis,GD_ACTOR_INVENTORY_WORK);
    TEST_ASSERT(inventory && !inventory->limited && inventory->node_count==10 && inventory->link_count==18);
    TEST_ASSERT(!inventory->unexamined_records && !inventory->unexamined_sources && !inventory->unexamined_cases && !inventory->unexamined_participants && !inventory->unexamined_edges && !inventory->unexamined_calls);
    root=gd_rule_case_find(model,rules,"cases/rules#caller"); TEST_ASSERT(root>=0);
    first=inventory->participant_nodes[rules->cases[root].first_participant]; second=inventory->participant_nodes[rules->cases[root].first_participant+1];
    TEST_ASSERT(first>=0 && second>=0 && first!=second && inventory->nodes[first].record==inventory->nodes[second].record);
    TEST_ASSERT(inventory->definition_nodes[find_record(model,"things/unused")]>=0 && !inventory->incomplete_call_owners);
    for (i=0;i<inventory->node_count;++i) {
        TEST_ASSERT(gd_actor_inventory_kind_name(inventory->nodes[i].kind) && gd_actor_inventory_node_id(inventory,i,id,sizeof(id)));
        for (j=0;j<i;++j) { TEST_ASSERT(gd_actor_inventory_node_id(inventory,j,other,sizeof(other)) && strcmp(id,other)); }
        strcpy(other,"old"); TEST_ASSERT(!gd_actor_inventory_node_id(inventory,i,other,2) && !*other);
    }
    for (i=0;i<inventory->link_count;++i) {
        const GD_ActorInventoryLink *edge=&inventory->links[i]; ++kinds[edge->kind];
        TEST_ASSERT(gd_actor_inventory_link_name(edge->kind) && edge->from>=0 && edge->to>=0 && edge->from<inventory->node_count && edge->to<inventory->node_count);
        if (edge->kind==GD_ACTOR_MAPS_BINDING) {
            const GD_RuleBinding *map=&analysis->bindings[edge->binding];
            TEST_ASSERT(inventory->nodes[edge->from].participant==map->caller && inventory->nodes[edge->to].participant==map->callee && gd_rule_call_participant(analysis,edge->declaration,map->callee)==map->caller);
        }
        if (edge->kind==GD_ACTOR_DECLARED_NOUN) TEST_ASSERT(gd_actor_inventory_link_line(inventory,i)==rules->participants[edge->declaration].lines[0] && gd_actor_inventory_link_line(inventory,i)>rules->participants[edge->declaration].line);
        if (edge->kind==GD_ACTOR_CONTAINS_BINDING) TEST_ASSERT(gd_actor_inventory_link_line(inventory,i)==rules->participants[edge->declaration].line);
        if (edge->kind==GD_ACTOR_DECLARED_CONTEXT_AXIS) TEST_ASSERT(inventory->nodes[edge->from].kind==GD_ACTOR_CASE && inventory->nodes[edge->to].kind==GD_ACTOR_AXIS_DEFINITION);
    }
    TEST_ASSERT(kinds[GD_ACTOR_CONTAINS_BINDING]==4 && kinds[GD_ACTOR_DECLARED_NOUN]==4 && kinds[GD_ACTOR_DECLARED_AXIS]==2 && kinds[GD_ACTOR_DECLARED_CONTEXT_AXIS]==2 && kinds[GD_ACTOR_INVOKES]==2 && kinds[GD_ACTOR_MAPS_BINDING]==4);
    work=inventory->work;
    for (i=0;i<work;++i) {
        partial=gd_actor_inventory_build(model->allocator,analysis,i); TEST_ASSERT(partial && partial->limited && partial->work<=i);
        for (j=0;j<partial->link_count;++j) TEST_ASSERT(partial->links[j].from>=0 && partial->links[j].to>=0 && partial->links[j].from<partial->node_count && partial->links[j].to<partial->node_count);
        gd_actor_inventory_destroy(partial);
    }
    partial=gd_actor_inventory_build(model->allocator,analysis,work); TEST_ASSERT(partial && !partial->limited && partial->node_count==inventory->node_count && partial->link_count==inventory->link_count); gd_actor_inventory_destroy(partial);
    failure=(RouteFailure){model->allocator,0}; allocator=(GD_Allocator){&failure,route_allocate,route_release};
    TEST_ASSERT(!gd_actor_inventory_build(allocator,analysis,GD_ACTOR_INVENTORY_WORK));
    TEST_ASSERT(!gd_actor_inventory_build(model->allocator,NULL,0) && !gd_actor_inventory_build(model->allocator,analysis,-1) && !gd_actor_inventory_build(model->allocator,analysis,GD_ACTOR_INVENTORY_WORK+1));
    TEST_ASSERT(!gd_actor_inventory_kind_name((GD_ActorInventoryKind)-1) && !gd_actor_inventory_link_name((GD_ActorInventoryLinkKind)99) && !gd_actor_inventory_node_id(inventory,-1,id,sizeof(id)));
    printf("actor declaration inventory: %llu bytes; %d charged work units\n",(unsigned long long)sizeof(*inventory),work);
    {
        GD_ExplorerProject project={0}; GD_StructureOptions options={0}; GD_StructureResult *result; int stage;
        project.allocator=model->allocator; project.models[GD_EXPLORER_RULE_DATA]=model; project.rules=rules; project.calls=analysis; project.available=1u<<GD_EXPLORER_ACTORS;
        options.question="direct-connections"; options.projection="actor-inventory"; options.work_limit=GD_GRAPH_ANALYSIS_WORK;
        TEST_ASSERT(gd_actor_inventory_node_id(inventory,first,id,sizeof(id))); options.from=id;
        result=gd_structure_result_build(model,&project,&options);
        TEST_ASSERT(result && result->inventory && result->view==GD_EXPLORER_ACTORS && result->query->complete && result->query->request.source==first);
        TEST_ASSERT(result->query->nodes[first].incoming==1 && result->query->nodes[first].outgoing==3 && result->query->nodes[first].partners==3 && result->query->nodes[first].incident==4);
        TEST_ASSERT(gd_structure_result_record(result,first)==inventory->nodes[first].record && gd_structure_result_node_id(result,first,other,sizeof(other)) && !strcmp(id,other));
        for (i=0;i<result->layout->edge_count;++i) TEST_ASSERT(gd_structure_result_edge_record(result,i)==inventory->nodes[inventory->links[i].from].record && !strcmp(gd_graph_relation_name(result->layout->edges[i].relation),gd_actor_inventory_link_name(inventory->links[i].kind)));
        { FILE *output=tmpfile(); char text[65536]; size_t length; TEST_ASSERT(output && gd_write_structure_result(output,result,1,"nodes,links,evidence,findings")); rewind(output); length=fread(text,1,sizeof(text)-1,output); text[length]=0; TEST_ASSERT(strstr(text,"\"projection\":\"actor-inventory\"") && strstr(text,"\"native_kind\":\"local_binding\"") && strstr(text,"\"compiled_binding\":") && strstr(text,"\"actor_inventory_coverage\":") && strstr(text,"\"rule_findings\":")); fclose(output); }
        gd_structure_result_destroy(result);
        options.relations[0]="declared_noun"; options.relation_count=1;
        result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->query->nodes[first].incident==1 && result->query->nodes[first].partners==1); gd_structure_result_destroy(result); options.relation_count=0;
        for (stage=0;stage<5;++stage) {
            failure=(RouteFailure){model->allocator,stage}; project.allocator=(GD_Allocator){&failure,route_allocate,route_release};
            result=gd_structure_result_build(model,&project,&options); project.allocator=model->allocator;
            TEST_ASSERT(!result && model->error_count && !strcmp(model->errors[model->error_count-1].code,"query.memory")); model->error_count=0;
        }
        options.from="not-an-inventory-id"; TEST_ASSERT(!gd_structure_result_build(model,&project,&options) && !strcmp(model->errors[0].code,"query.selector_missing")); model->error_count=0;
    }
    {
        GD_QueryRequest request={0}; GD_Query *query; GD_QueryScene *scene,*primary; int identity_work=0,alive=-1;
        request.kind=GD_QUERY_ACTOR_STATES; request.case_index=root; request.noun=-1; request.axis=find_record(model,"actions/Life"); request.effect_kind=-1; request.participant="Left"; request.work_limit=GD_ACTOR_WORK;
        query=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(query && query->variants && query->variants->variant_count==2);
        scene=gd_query_scene_actor_topology(model->allocator,query,GD_ACTOR_WORK); primary=gd_query_scene_build(query,GD_QUERY_PRIMARY_VARIANT,0,0,0,GD_ACTOR_WORK);
        TEST_ASSERT(scene && primary && scene->ghosts==GD_QUERY_ALL_VARIANTS && scene->include_excluded && scene->layout.node_count>primary->layout.node_count); gd_query_scene_destroy(primary);
        for (i=0;i<scene->layout.node_count;++i) {
            TEST_ASSERT(gd_actor_scene_node_id(scene,i,id,sizeof(id),&identity_work,GD_ACTOR_WORK));
            for (j=0;j<i;++j) { TEST_ASSERT(gd_actor_scene_node_id(scene,j,other,sizeof(other),&identity_work,GD_ACTOR_WORK) && strcmp(id,other)); }
            if (scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && !strcmp(query->variants->variants[scene->nodes[i].variant].value,"alive")) alive=i;
        }
        TEST_ASSERT(alive>=0 && gd_actor_scene_node_id(scene,alive,id,sizeof(id),&identity_work,GD_ACTOR_WORK));
        {
            int effect=-1,axis=find_record(model,"actions/Contact"),prior_axis; const char *unit=model->records[axis].axis.unit,*type=model->records[axis].axis.type; char before[GD_MAX_PATH],after[GD_MAX_PATH];
            for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_EFFECT) { effect=scene->nodes[i].source; break; }
            TEST_ASSERT(effect>=0); prior_axis=query->variants->effects[effect].axis;
            /* Synthetic side-effect axis metadata, distinct from selected Life.
               Restore every field; no parser, authored file or source write. */
            query->variants->effects[effect].axis=axis;
            TEST_ASSERT(gd_actor_scene_node_id(scene,i,before,sizeof(before),&identity_work,GD_ACTOR_WORK));
            model->records[axis].axis.unit="synthetic-unit"; TEST_ASSERT(gd_actor_scene_node_id(scene,i,after,sizeof(after),&identity_work,GD_ACTOR_WORK) && strcmp(before,after)); model->records[axis].axis.unit=unit;
            model->records[axis].axis.type="number"; TEST_ASSERT(gd_actor_scene_node_id(scene,i,after,sizeof(after),&identity_work,GD_ACTOR_WORK) && strcmp(before,after)); model->records[axis].axis.type=type;
            query->variants->effects[effect].axis=prior_axis;
        }
        { int zero=0; strcpy(other,"old"); TEST_ASSERT(!gd_actor_scene_node_id(scene,alive,other,sizeof(other),&zero,0) && !*other && !zero); }
        gd_query_scene_destroy(scene);
        {
            GD_ExplorerProject project={0}; GD_StructureOptions options={0}; GD_StructureResult *result; char start[GD_MAX_PATH]; int kind; StructureMemory memory={0,-1};
            project.allocator=(GD_Allocator){&memory,structure_allocate,structure_release}; project.models[GD_EXPLORER_RULE_DATA]=model; project.rules=rules; project.calls=analysis; project.available=1u<<GD_EXPLORER_ACTORS;
            options.projection="actors"; options.actor_query=query; options.work_limit=GD_GRAPH_ANALYSIS_WORK;
            for (kind=0;kind<GD_STRUCTURE_COUNT;++kind) {
                options.question=gd_structure_name((GD_StructureKind)kind); options.from=options.to=NULL;
                if (kind==GD_STRUCTURE_DECLARATIONS) options.actor_query=NULL; else options.actor_query=query;
                if (kind==GD_STRUCTURE_REACHABILITY || kind==GD_STRUCTURE_DOMINATORS || kind==GD_STRUCTURE_BYPASS || kind==GD_STRUCTURE_RELATED_PATHS) options.from=start;
                if (kind==GD_STRUCTURE_BYPASS) options.to=start;
                result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->view==GD_EXPLORER_ACTORS && !model->error_count);
                if (kind!=GD_STRUCTURE_DECLARATIONS) {
                    TEST_ASSERT(result->actor_scene && !result->inventory && result->layout==&result->actor_scene->layout && result->actor_scene->ghosts==GD_QUERY_ALL_VARIANTS && result->actor_scene->include_excluded);
                    TEST_ASSERT(gd_structure_result_node_id(result,result->layout->nodes[0].key,start,sizeof(start)));
                    TEST_ASSERT(gd_structure_result_record(result,result->layout->nodes[0].key)>=0);
                    for (i=0;i<result->layout->edge_count;++i) TEST_ASSERT(gd_structure_result_edge_record(result,result->layout->edges[i].key)>=0 && gd_graph_relation_name(result->layout->edges[i].relation));
                } else TEST_ASSERT(result->gaps && !result->actor_scene);
                gd_structure_result_destroy(result); TEST_ASSERT(!memory.outstanding);
            }
            options.question="adjacency"; options.actor_query=query; options.from=options.to=NULL;
            for (i=0;i<4;++i) { memory.remaining=i; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(!result && !memory.outstanding && model->error_count==1 && !strcmp(model->errors[0].code,"query.memory")); model->error_count=0; }
            memory.remaining=-1; options.actor_query=NULL; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(!result && model->error_count==1 && !strcmp(model->errors[0].code,"query.actor_scope")); model->error_count=0;
        }
        gd_query_destroy(query);
        request.value_count=2; request.values[0]="dead"; request.values[1]="alive"; query=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(query);
        scene=gd_query_scene_actor_topology(model->allocator,query,GD_ACTOR_WORK); TEST_ASSERT(scene); alive=-1;
        for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && !strcmp(query->variants->variants[scene->nodes[i].variant].value,"alive")) alive=i;
        TEST_ASSERT(alive>=0 && gd_actor_scene_node_id(scene,alive,other,sizeof(other),&identity_work,GD_ACTOR_WORK) && !strcmp(id,other));
        gd_query_scene_destroy(scene); gd_query_destroy(query);
    }
    TEST_ASSERT(!gd_sources_changed(model)); gd_actor_inventory_destroy(inventory); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(actor_inventory_capacity_and_whole_template_maps,
    int mode;
    for (mode=0;mode<2;++mode) {
        GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorInventory *inventory;
        char *body; size_t used=0; int c,p,maps=0;
        TEST_ASSERT(model && setup(mode ? "actor-inventory-wide-map":"actor-inventory-capacity")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
        TEST_ASSERT(write_record(0,"unit","---\naxes: []\n---\n")); body=model->allocator.allocate(model->allocator.context,600000); TEST_ASSERT(body);
        used+=(size_t)snprintf(body+used,600000-used,"```gd-rule\n");
        for (c=0;c<(mode ? 33:126);++c) {
            used+=(size_t)snprintf(body+used,600000-used,"case c%d:\n    target: P0\n    context: ANY\n",c);
            for (p=0;p<(mode ? 2:32);++p) used+=(size_t)snprintf(body+used,600000-used,"    P%d:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\n",p);
            TEST_ASSERT(used<600000);
        }
        TEST_ASSERT(snprintf(body+used,600000-used,"```\n")>0 && write_record(2,"template",body)); model->allocator.release(model->allocator.context,body);
        if (mode) TEST_ASSERT(write_record(2,"caller","```gd-rule\ncase main:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        causes: cases/template with P0=A, P1=B\n        preserve: ALL\n    B:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\n```\n"));
        TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules); TEST_ASSERT(rules && analysis && !model->error_count);
        inventory=gd_actor_inventory_build(model->allocator,analysis,GD_ACTOR_INVENTORY_WORK); TEST_ASSERT(inventory && inventory->work<=GD_ACTOR_INVENTORY_WORK);
        if (!mode) {
            TEST_ASSERT(inventory->limited && inventory->node_count==4093 && inventory->unexamined_cases==2 && inventory->unexamined_participants==rules->participant_count);
            for (c=0;c<124;++c) { TEST_ASSERT(inventory->case_nodes[c]>=0); for (p=0;p<32;++p) TEST_ASSERT(inventory->participant_nodes[rules->cases[c].first_participant+p]>=0); }
            TEST_ASSERT(inventory->case_nodes[124]<0 && inventory->participant_nodes[rules->cases[124].first_participant]<0);
        } else {
            TEST_ASSERT(!inventory->limited && analysis->call_count==1 && analysis->calls[0].binding_count==66);
            for (c=0;c<inventory->link_count;++c) if (inventory->links[c].kind==GD_ACTOR_MAPS_BINDING) ++maps;
            TEST_ASSERT(maps==66);
        }
        TEST_ASSERT(!gd_sources_changed(model)); gd_actor_inventory_destroy(inventory); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(actor_inventory_unknown_scope_has_no_invented_bindings,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorInventory *inventory; int i;
    TEST_ASSERT(model && setup("actor-inventory-unknown")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"unit","---\naxes: [outside/Life]\n---\n"));
    TEST_ASSERT(write_record(2,"empty","```gd-rule\n```\n"));
    TEST_ASSERT(write_record(2,"unknown","```gd-rule\ncase unknown:\n    target: A\n    context: ANY\n    A:\n        noun: things/missing\n        requires: ANY\n        causes: cases/missing\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules); TEST_ASSERT(rules && analysis && has_error(model,"rules.invocation_reference"));
    inventory=gd_actor_inventory_build(model->allocator,analysis,GD_ACTOR_INVENTORY_WORK);
    TEST_ASSERT(inventory && !inventory->limited && inventory->source_incomplete && inventory->incomplete_sources==1 && inventory->unresolved_nouns==1 && inventory->unresolved_axes==1 && inventory->unresolved_calls==1);
    TEST_ASSERT(inventory->node_count==3 && inventory->link_count==1 && inventory->links[0].kind==GD_ACTOR_CONTAINS_BINDING);
    for (i=0;i<inventory->node_count;++i) TEST_ASSERT(inventory->nodes[i].kind!=GD_ACTOR_AXIS_DEFINITION);
    gd_actor_inventory_destroy(inventory); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
