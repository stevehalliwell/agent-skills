#include "gd_noun_grid.h"
#include "gd_noun_label.h"
TEST(noun_labels_preserve_canonical_identity,
    GD_Model *model;int actor,resource,unknown;
    TEST_ASSERT(setup("noun-labels"));collections[1].role=GD_AXIS;collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"hero","---\nis_actor: true\n---\n"));
    TEST_ASSERT(write_record(0,"lives","---\nis_actor: false\n---\n"));
    TEST_ASSERT(write_record(0,"unknown","---\naxes: []\n---\n"));
    model=model_create();TEST_ASSERT(model && gd_load(model,collections,3));
    actor=find_record(model,"things/hero");resource=find_record(model,"things/lives");unknown=find_record(model,"things/unknown");TEST_ASSERT(actor>=0 && resource>=0 && unknown>=0);
    TEST_ASSERT(!strcmp(model->records[actor].name,"hero") && !strcmp(model->records[actor].reference,"things/hero"));
    TEST_ASSERT(!strcmp(model->records[actor].display_name,"hero - A") && !strcmp(model->records[resource].display_name,"lives - N") && !strcmp(model->records[unknown].display_name,"unknown - U"));
    TEST_ASSERT(gd_noun_label_style(&model->records[actor])==GD_NOUN_LABEL_UNDERLINE && gd_noun_label_style(&model->records[resource])==GD_NOUN_LABEL_BOLD && gd_noun_label_style(&model->records[unknown])==GD_NOUN_LABEL_REGULAR && gd_noun_label_style(NULL)==GD_NOUN_LABEL_REGULAR);
    {char prefix[64];GD_Record unrelated={.display_name="not-a-noun - A",.is_actor_line=1,.is_actor=1};
        TEST_ASSERT(gd_noun_label_style(&unrelated)==GD_NOUN_LABEL_REGULAR);
        TEST_ASSERT(gd_noun_label_prefix(&model->records[actor],"binding Ω\nhero - A\nstate",prefix,sizeof(prefix)) && !strcmp(prefix,"binding Ω\n"));
        TEST_ASSERT(!gd_noun_label_prefix(&model->records[actor],"[Unresolved noun] things/hero",prefix,sizeof(prefix)) && !*prefix);
        TEST_ASSERT(!gd_noun_label_prefix(&model->records[actor],"prefix hero - A",prefix,2) && !*prefix);
        TEST_ASSERT(!gd_noun_label_prefix(NULL,"hero - A",prefix,sizeof(prefix)) && !gd_noun_label_prefix(&model->records[actor],NULL,prefix,sizeof(prefix)));
    }
    TEST_ASSERT(!gd_sources_changed(model));gd_destroy(model);
    /* Related snapshots still know original noun role; unrelated metadata is not guessed. */
    collections[0].role=GD_RELATED;collections[0].noun_collection=1;model=model_create();TEST_ASSERT(model && gd_load(model,collections,3));actor=find_record(model,"things/hero");
    TEST_ASSERT(actor>=0 && model->records[actor].role==GD_RELATED && model->records[actor].is_noun && model->records[actor].is_actor && !strcmp(model->records[actor].display_name,"hero - A") && gd_noun_label_style(&model->records[actor])==GD_NOUN_LABEL_UNDERLINE);
    gd_destroy(model);
)
TEST(noun_grid_case_counts_diagonal_limits_and_evidence,
    GD_Model *model=model_create();GD_Rules *rules;GD_NounGrid *grid,*limited;int a,b,c,d,pair,i,budget,expected,source;
    TEST_ASSERT(model && setup("noun-grid"));collections[1].role=GD_AXIS;collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"a","---\nis_actor: true\n---\n"));
    TEST_ASSERT(write_record(0,"b","---\nis_actor: false\n---\n"));
    TEST_ASSERT(write_record(0,"c","---\naxes: []\n---\n"));
    TEST_ASSERT(write_record(0,"isolate","---\naxes: []\n---\n"));
    TEST_ASSERT(write_record(2,"shared","```gd-rule\ncase one:\n    target: A\n    context: ANY\n    A:\n        noun: things/a\n        requires: ANY\n    A2:\n        noun: things/a\n        requires: ANY\n    B:\n        noun: things/b\n        requires: ANY\n    B2:\n        noun: things/b\n        requires: ANY\n    C:\n        noun: things/c\n        requires: ANY\ncase two:\n    target: A\n    context: ANY\n    A:\n        noun: things/a\n        requires: ANY\n    B:\n        noun: things/b\n        requires: ANY\ncase three:\n    target: A\n    context: ANY\n    A:\n        noun: things/a\n        requires: ANY\n    B:\n        noun: things/b\n        requires: ANY\ncase alone_a:\n    target: A\n    context: ANY\n    A:\n        noun: things/a\n        requires: ANY\ncase alone_d:\n    target: D\n    context: ANY\n    D:\n        noun: things/isolate\n        requires: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3));model->collections[2].role=GD_CASE;rules=gd_read_rules(model);TEST_ASSERT(rules && !model->error_count);
    a=find_record(model,"things/a");b=find_record(model,"things/b");c=find_record(model,"things/c");d=find_record(model,"things/isolate");source=find_record(model,"cases/shared");TEST_ASSERT(a>=0 && b>=0 && c>=0 && d>=0 && source>=0);
    grid=gd_noun_grid_build(rules,GD_NOUN_GRID_WORK);TEST_ASSERT(grid && !grid->limited && grid->noun_count==4 && grid->pair_count==5);
    pair=gd_noun_grid_pair(grid,a,b);TEST_ASSERT(pair>=0 && pair==gd_noun_grid_pair(grid,b,a) && grid->pairs[pair].count==3);
    TEST_ASSERT(grid->pairs[gd_noun_grid_pair(grid,a,a)].count==1 && grid->pairs[gd_noun_grid_pair(grid,b,b)].count==1 && gd_noun_grid_pair(grid,c,c)<0);
    TEST_ASSERT(gd_noun_grid_pair(grid,a,d)<0 && gd_noun_grid_pair(grid,b,d)<0 && gd_noun_grid_pair(grid,d,d)<0);
    expected=0;for(i=0;i<rules->case_count;++i)expected+=gd_noun_grid_case(grid,pair,i);TEST_ASSERT(expected==3);
    {GD_Layout *layout=model->allocator.allocate(model->allocator.context,sizeof(*layout));int loop=0;TEST_ASSERT(layout && gd_noun_grid_layout(grid,layout) && layout->node_count==4 && layout->edge_count==5 && layout->mode==GD_LAYOUT_FORCE && layout->spring_gap+GD_NOUN_GRAPH_CARD_HEIGHT==GD_NOUN_GRAPH_MIN_REST_LENGTH);for(i=0;i<layout->edge_count;++i){const GD_LayoutEdge *edge=&layout->edges[i];TEST_ASSERT(edge->key==i && layout->nodes[edge->a].key==grid->pairs[i].a && layout->nodes[edge->b].key==grid->pairs[i].b);if(edge->a==edge->b){GD_LayoutRoute route;gd_layout_route(layout,i,&route);TEST_ASSERT(route.count>2);++loop;}}TEST_ASSERT(loop==2 && !layout->nodes[grid->slot[d]].degree);gd_layout_step(layout,-1);model->allocator.release(model->allocator.context,layout);}
    {char path[GD_MAX_PATH];FILE *file;snprintf(path,sizeof(path),"%s/noun-grid.json",root);file=fopen(path,"wb");TEST_ASSERT(file && gd_noun_grid_write(file,grid));TEST_ASSERT(!fclose(file));}
    for(budget=0;budget<=grid->work;++budget){int j;limited=gd_noun_grid_build(rules,budget);TEST_ASSERT(limited && limited->work<=budget && limited->noun_count==grid->noun_count);for(i=0;i<limited->pair_count;++i){expected=0;for(j=0;j<rules->case_count;++j)expected+=gd_noun_grid_case(limited,i,j);TEST_ASSERT(expected==limited->pairs[i].count);}gd_noun_grid_destroy(limited);}
    {GD_Allocator original=rules->allocator;for(i=0;i<2;++i){RouteFailure failure={original,i};rules->allocator=(GD_Allocator){&failure,route_allocate,route_release};limited=gd_noun_grid_build(rules,GD_NOUN_GRID_WORK);rules->allocator=original;TEST_ASSERT(!limited);}}
    model->collections[2].role=GD_MDA_MECHANIC;limited=gd_noun_grid_build(rules,GD_NOUN_GRID_WORK);TEST_ASSERT(limited && limited->noun_count==4 && !limited->pair_count);gd_noun_grid_destroy(limited);model->collections[2].role=GD_CASE;
    TEST_ASSERT(!gd_noun_grid_build(NULL,1) && !gd_noun_grid_build(rules,-1) && !gd_noun_grid_build(rules,GD_NOUN_GRID_WORK+1));
    gd_noun_grid_destroy(grid);
    /* Synthetic native-capacity fixture: 300 explicit noun types in one case. */
    for(i=0;i<300;++i){model->records[i].role=GD_NOUN;rules->participants[i].noun_record=i;rules->participants[i].noun_resolution=GD_RULE_RESOLVED;}
    model->records[300].role=GD_RULE;model->records[300].collection=2;model->record_count=301;rules->participant_count=300;rules->sources[0].record=300;rules->case_count=1;rules->cases[0].source=0;rules->cases[0].first_participant=0;rules->cases[0].participant_count=300;
    grid=gd_noun_grid_build(rules,GD_NOUN_GRID_WORK);TEST_ASSERT(grid && grid->limited && grid->incomplete && grid->pair_count==GD_MAX_PAIRS && grid->noun_count==300);
    gd_noun_grid_destroy(grid);gd_rules_destroy(rules);gd_destroy(model);
)
