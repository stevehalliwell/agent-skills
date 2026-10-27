#include "gd_diagnostics.h"
#include <string.h>
#include <limits.h>

static size_t retained_length(const char *text, size_t limit, int *truncated) {
    size_t length=0;
    if (!text) return 0;
    while (length<limit && text[length]) ++length;
    if (text[length]) {
        *truncated=1;
        while (length && ((unsigned char)text[length]&0xc0)==0x80) --length;
    }
    return length;
}
GD_DiagnosticErrors *gd_diagnostics_capture(GD_Allocator allocator, const GD_Error *errors, int count, int omitted) {
    GD_Error fallback={"view.reload","Cannot prepare replacement snapshot/layout",NULL,0};
    GD_DiagnosticErrors *result;
    size_t lengths[GD_MAX_ERRORS][3], bytes=0;
    int shortened[GD_MAX_ERRORS]={0}, path_shortened[GD_MAX_ERRORS]={0}, i, j, retained;
    char *cursor;
    if (!allocator.allocate || !allocator.release || count<0 || omitted<0 || (count && !errors)) return NULL;
    if (!count) { errors=&fallback; count=1; }
    retained=count>GD_MAX_ERRORS ? GD_MAX_ERRORS:count;
    /* count/omitted originate from bounded producers; refuse arithmetic overflow. */
    if (omitted>INT_MAX-(count-retained)) return NULL;
    for (i=0;i<retained;++i) {
        const char *parts[]={errors[i].code,errors[i].message,errors[i].path};
        for (j=0;j<3;++j) {
            lengths[i][j]=retained_length(parts[j],j==0 ? 128:GD_MAX_PATH-1,j==2 ? &path_shortened[i]:&shortened[i]);
            if (path_shortened[i]) shortened[i]=1;
            if (parts[j]) bytes+=lengths[i][j]+1;
        }
    }
    result=allocator.allocate(allocator.context,sizeof(*result)+bytes);
    if (!result) return NULL;
    memset(result,0,sizeof(*result)); result->allocator=allocator;
    result->count=retained; result->omitted=omitted+(count-retained); cursor=result->text;
    for (i=0;i<retained;++i) {
        const char *parts[]={errors[i].code,errors[i].message,errors[i].path};
        const char **destinations[]={&result->errors[i].code,&result->errors[i].message,&result->errors[i].path};
        result->errors[i].line=errors[i].line; result->truncated[i]=(unsigned char)shortened[i];
        result->path_truncated[i]=(unsigned char)path_shortened[i];
        for (j=0;j<3;++j) if (parts[j]) {
            *destinations[j]=cursor; memcpy(cursor,parts[j],lengths[i][j]);
            cursor[lengths[i][j]]=0; cursor+=lengths[i][j]+1;
        }
    }
    return result;
}
void gd_diagnostics_destroy(GD_DiagnosticErrors *errors) {
    if (errors) errors->allocator.release(errors->allocator.context,errors);
}
int gd_diagnostic_count(const GD_DiagnosticSet *set) {
    if (!set || !set->model || set->finding_count<0 || set->finding_count>GD_MAX_ERRORS || (set->finding_count && !set->findings)) return 0;
    if (set->model->error_count<0 || set->model->error_count>GD_MAX_ERRORS ||
        (set->rejected && (set->rejected->count<0 || set->rejected->count>GD_MAX_ERRORS))) return 0;
    return set->finding_count+set->model->error_count+(set->rejected ? set->rejected->count:0);
}
static int record_at_path(const GD_Model *model, const char *path) {
    int i, found=-1;
    if (!path) return -1;
    for (i=0;i<model->record_count;++i) if (model->records[i].source_path && !strcmp(path,model->records[i].source_path)) {
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
int gd_diagnostic_item(const GD_DiagnosticSet *set, int index, GD_DiagnosticItem *item) {
    const GD_Model *model;
    if (!item || index<0 || index>=gd_diagnostic_count(set)) return 0;
    memset(item,0,sizeof(*item)); item->record=-1; model=set->model;
    if (index<set->finding_count) {
        const GD_Finding *finding=&set->findings[index];
        item->kind=GD_DIAGNOSTIC_FINDING; item->code=finding->code; item->message=finding->message;
        item->field=finding->field; item->reference=finding->reference; item->line=finding->line;
        if (finding->record>=0 && finding->record<model->record_count) item->record=finding->record;
    } else {
        const GD_Error *error;
        index-=set->finding_count;
        if (index<model->error_count) { item->kind=GD_DIAGNOSTIC_ERROR; error=&model->errors[index]; }
        else {
            index-=model->error_count; item->kind=GD_DIAGNOSTIC_RELOAD;
            error=&set->rejected->errors[index]; item->truncated=set->rejected->truncated[index];
        }
        item->code=error->code; item->message=error->message; item->path=error->path; item->line=error->line;
        if (item->kind!=GD_DIAGNOSTIC_RELOAD || !set->rejected->path_truncated[index]) item->record=record_at_path(model,item->path);
    }
    if (item->record>=0) {
        item->record_reference=model->records[item->record].reference;
        if (!item->path) item->path=model->records[item->record].source_path;
    }
    return 1;
}
static unsigned char lower_ascii(unsigned char c) { return c>='A' && c<='Z' ? (unsigned char)(c+'a'-'A'):c; }
static int contains(const char *value, const char *text) {
    size_t i,j;
    if (!value) return 0;
    for (i=0;value[i];++i) {
        for (j=0;text[j] && value[i+j] && lower_ascii((unsigned char)text[j])==lower_ascii((unsigned char)value[i+j]);++j) {}
        if (!text[j]) return 1;
    }
    return 0;
}
int gd_diagnostic_matches(const GD_DiagnosticItem *item, GD_DiagnosticKind kind, const char *text, int record) {
    if (!item || (kind!=GD_DIAGNOSTIC_ALL && item->kind!=kind) || (record!=-1 && (record<0 || item->record!=record))) return 0;
    if (!text || !*text) return 1;
    return contains(item->code,text) || contains(item->message,text) || contains(item->path,text) ||
        contains(item->record_reference,text) || contains(item->field,text) || contains(item->reference,text);
}
