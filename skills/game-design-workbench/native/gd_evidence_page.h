#ifndef GD_EVIDENCE_PAGE_H
#define GD_EVIDENCE_PAGE_H
#include <assert.h>
enum { GD_EVIDENCE_PAGE_ROWS=64,GD_EVIDENCE_AXIS_ROWS=16 };
typedef struct { int first,end,total,page,pages; } GD_EvidencePage;
/* Retained-row ordinals only. Native source slots/keys remain separate.
   No allocation. Arithmetic remains valid through INT_MAX retained rows. */
static inline GD_EvidencePage gd_evidence_page(int total,int size,int page) {
    GD_EvidencePage range={0};int remaining;
    assert(total>=0 && size>0 && size<=GD_EVIDENCE_PAGE_ROWS);
    range.total=total;range.pages=total ? 1+(total-1)/size:0;
    if(!range.pages || page<0)page=0;else if(page>=range.pages)page=range.pages-1;
    range.page=page;
    range.first=range.page*size;remaining=total-range.first;
    range.end=range.first+(remaining<size ? remaining:size);return range;
}
/* Call only for matching retained rows, never for unfiltered source slots. */
static inline int gd_evidence_page_take(const GD_EvidencePage *page,int *ordinal) {
    int row;assert(page && ordinal && *ordinal>=0 && *ordinal<page->total);
    row=(*ordinal)++;return row>=page->first && row<page->end;
}
#endif
