#include "gd_diagnostics_ui.h"
#include "raylib.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"

void gd_diagnostics_clear(GD_DiagnosticBrowser *browser) {
    gd_diagnostics_destroy(browser->rejected); browser->rejected=NULL; browser->capture_failed=0; browser->open_next=browser->collapse_next=false;
}
void gd_diagnostics_rejected(GD_DiagnosticBrowser *browser, GD_Allocator allocator, const GD_Model *failed) {
    gd_diagnostics_clear(browser);
    browser->rejected=gd_diagnostics_capture(allocator,failed ? failed->errors:NULL,
        failed ? failed->error_count:0,failed ? failed->errors_omitted:0);
    browser->capture_failed=browser->rejected==NULL; browser->open_next=true;
}
int gd_diagnostics_panel(GD_DiagnosticBrowser *browser, const GD_Model *model,
    const GD_Finding *findings, int count, int omitted, int selected) {
    GD_DiagnosticSet set={model,findings,count,omitted,browser->rejected};
    int total=gd_diagnostic_count(&set), shown=0, navigate=-1, i;
    GD_DiagnosticItem item;
    int partial=omitted || model->errors_omitted || browser->capture_failed || (browser->rejected && browser->rejected->omitted);
    if (browser->open_next || browser->collapse_next) {
        igSetNextItemOpen(browser->open_next,ImGuiCond_Always);
        browser->open_next=browser->collapse_next=false;
    }
    if (!igCollapsingHeader_TreeNodeFlags(TextFormat("Diagnostics (%d retained%s)###domain-diagnostics",total,partial ? " | partial":""),total || partial ? ImGuiTreeNodeFlags_DefaultOpen:0)) return -1;
    igPushID_Str("Domain diagnostics");
    igTextUnformatted("Search code / message / source",NULL); igSetNextItemWidth(-1);
    igInputText("##diagnostic-search",browser->filter,sizeof(browser->filter),0,NULL,NULL);
    igCombo_Str("Kind",&browser->kind,"All\0Findings / questions\0Loaded operation errors\0Rejected reload errors\0",-1);
    igCheckbox("Selected record only",&browser->selected_only);
    igTextWrapped("%d findings | %d loaded errors | %d reload errors",count,model->error_count,browser->rejected ? browser->rejected->count:0);
    if (omitted) igTextWrapped("%d additional domain findings omitted by producer.",omitted);
    if (model->errors_omitted) igTextWrapped("%d additional loaded operation errors omitted.",model->errors_omitted);
    if (browser->rejected && browser->rejected->omitted) igTextWrapped("%d additional rejected-reload errors omitted.",browser->rejected->omitted);
    if (browser->capture_failed) igTextWrapped("Could not retain rejected-reload details. Last-good source remains visible; reload banner has summary.");
    igBeginChild_Str("Diagnostic entries",(ImVec2_c){0,150},0,0);
    for (i=0;i<total;++i) {
        if (!gd_diagnostic_item(&set,i,&item) || !gd_diagnostic_matches(&item,(GD_DiagnosticKind)browser->kind,browser->filter,browser->selected_only ? selected>=0 ? selected:-2:-1)) continue;
        ++shown; igPushID_Int(i);
        igTextWrapped("%s | %s",item.kind==GD_DIAGNOSTIC_FINDING ? "Finding/question":item.kind==GD_DIAGNOSTIC_ERROR ? "Loaded operation error":"Rejected reload",item.code ? item.code:"unknown code");
        igTextWrapped("%s",item.message ? item.message:"Message unavailable");
        if (item.record_reference) igTextWrapped("Record: %s",item.record_reference);
        if (item.field) igTextWrapped("Field: %s",item.field);
        if (item.reference) igTextWrapped("Reference: %s",item.reference);
        if (item.path) igTextWrapped("%s (line %d; 0 = unavailable)",item.path,item.line);
        if (item.truncated) igTextWrapped("Retained text shortened at bounded UTF-8 boundary; inspect source for full context.");
        if (item.record>=0 && igButton(item.kind==GD_DIAGNOSTIC_RELOAD ? "Inspect last-good loaded record":"Inspect loaded record",(ImVec2_c){0,0})) {
            navigate=item.record; browser->collapse_next=true;
        }
        if (item.path && *item.path && igButton("Copy retained source path",(ImVec2_c){0,0})) SetClipboardText(item.path);
        igSeparator(); igPopID();
    }
    if (!shown) igTextWrapped(total ? "No matching diagnostics. Filters do not change analysis or prove completeness.":"No retained diagnostics in supplied domain scope. Not a schema, coverage, learning or experience verdict.");
    igEndChild();
    igText("%d matching / %d retained",shown,total);
    igTextWrapped("Domain findings are not ranked design verdicts. Rejected reloads describe failed candidate input; navigation shows last-good loaded source. No repair or source writes.");
    igPopID(); return navigate;
}
