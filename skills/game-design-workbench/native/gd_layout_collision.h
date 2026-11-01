/* Private shared Free graph footprint projection. No topology/identity changes.
   Snapshot sweep intervals, a rotating starting row and a strict candidate cap
   avoid an exhaustive all-pairs frame. Residual overlap is revisited next step. */
#ifndef GD_LAYOUT_COLLISION_H
#define GD_LAYOUT_COLLISION_H
#include <float.h>
#define GD_LAYOUT_CLEARANCE 8.0
#define GD_LAYOUT_COLLISION_PASSES 16
#define GD_LAYOUT_SEPARATION_SHIFT 20.0f
#define GD_LAYOUT_CONTACT_EPSILON 0.0001
#define GD_LAYOUT_CONTACT_RELAXATION 1.15
int gd_layout_set_footprint(GD_LayoutNode *node,float left,float top,float right,float bottom) {
    if (!node || !isfinite(left) || !isfinite(top) || !isfinite(right) || !isfinite(bottom) ||
        fabsf(left)>1048576 || fabsf(top)>1048576 || fabsf(right)>1048576 || fabsf(bottom)>1048576 ||
        left>=right || top>=bottom) return 0;
    node->footprint=(GD_LayoutBounds){left,top,right,bottom}; return 1;
}
static int collision_compare(const void *left,const void *right) {
    const GD_LayoutCollisionItem *a=left,*b=right;
    if (a->minimum!=b->minimum) return a->minimum<b->minimum ? -1:1;
    return (a->node>b->node)-(a->node<b->node);
}
/* In-place heapsort: fixed scratch, worst-case O(n log n), no libc sort
   allocation or input-dependent quadratic broad phase. Node breaks all ties. */
static void collision_sift(GD_LayoutCollisionItem *items,int root,int count) {
    while (root*2+1<count) {
        int child=root*2+1; GD_LayoutCollisionItem swap;
        if (child+1<count && collision_compare(&items[child],&items[child+1])<0) ++child;
        if (collision_compare(&items[root],&items[child])>=0) break;
        swap=items[root]; items[root]=items[child]; items[child]=swap; root=child;
    }
}
static void collision_sort(GD_LayoutCollisionItem *items,int count) {
    int i;
    for (i=count/2;i>0;--i) collision_sift(items,i-1,count);
    for (i=count-1;i>0;--i) {
        GD_LayoutCollisionItem swap=items[0]; items[0]=items[i]; items[i]=swap;
        collision_sift(items,0,i);
    }
}
static float collision_spring_length(const GD_LayoutNode *a,const GD_LayoutNode *b,float dx,float dy,float distance) {
    double x=DBL_MAX,y=DBL_MAX;
    if (dx>0) x=((double)a->footprint.right-b->footprint.left+GD_LAYOUT_CLEARANCE)*distance/dx;
    else if (dx<0) x=((double)b->footprint.right-a->footprint.left+GD_LAYOUT_CLEARANCE)*distance/-dx;
    if (dy>0) y=((double)a->footprint.bottom-b->footprint.top+GD_LAYOUT_CLEARANCE)*distance/dy;
    else if (dy<0) y=((double)b->footprint.bottom-a->footprint.top+GD_LAYOUT_CLEARANCE)*distance/-dy;
    return (float)fmax(110,fmin(x,y));
}
static int collision_pair(GD_Layout *layout,int first,int second,int held) {
    GD_LayoutNode *a=&layout->nodes[first],*b=&layout->nodes[second];
    double left=(double)a->x+a->footprint.right-b->x-b->footprint.left+GD_LAYOUT_CLEARANCE;
    double right=(double)b->x+b->footprint.right-a->x-a->footprint.left+GD_LAYOUT_CLEARANCE;
    double up=(double)a->y+a->footprint.bottom-b->y-b->footprint.top+GD_LAYOUT_CLEARANCE;
    double down=(double)b->y+b->footprint.bottom-a->y-a->footprint.top+GD_LAYOUT_CLEARANCE;
    double x,y,dx=0,dy=0,weight_a=first==held ? 0:second==held ? 1:0.5,weight_b=1-weight_a;
    float ax,ay,bx,by; double amount,move_a,move_b,spare;
    float *remaining_a=&layout->collision_remaining[first],*remaining_b=&layout->collision_remaining[second];
    if (left<=GD_LAYOUT_CONTACT_EPSILON || right<=GD_LAYOUT_CONTACT_EPSILON || up<=GD_LAYOUT_CONTACT_EPSILON || down<=GD_LAYOUT_CONTACT_EPSILON) return 0;
    x=left<right || (left==right && first<second) ? -left:right;
    y=up<down || (up==down && first<second) ? -up:down;
    /* Small over-relaxation leaves room for neighboring corrections instead of
       repeatedly returning a touching pair to compression under edge springs. */
    if (fabs(x)<=fabs(y)) dx=x*GD_LAYOUT_CONTACT_RELAXATION; else dy=y*GD_LAYOUT_CONTACT_RELAXATION;
    amount=fmin(fabs(dx ? dx:dy),(double)*remaining_a+*remaining_b);
    if (amount<=GD_LAYOUT_CONTACT_EPSILON) { layout->collision_limited=1; return 0; }
    if (amount<fabs(dx ? dx:dy)) layout->collision_limited=1;
    move_a=fmin(*remaining_a,amount*weight_a); move_b=fmin(*remaining_b,amount*weight_b);
    spare=amount-move_a-move_b;
    if (spare>0) { double extra=fmin(*remaining_a-move_a,spare); move_a+=extra; move_b+=spare-extra; }
    ax=(float)(a->x+(dx<0 ? -move_a:dx>0 ? move_a:0)); ay=(float)(a->y+(dy<0 ? -move_a:dy>0 ? move_a:0));
    bx=(float)(b->x+(dx<0 ? move_b:dx>0 ? -move_b:0)); by=(float)(b->y+(dy<0 ? move_b:dy>0 ? -move_b:0));
    if (!isfinite(ax)||!isfinite(ay)||!isfinite(bx)||!isfinite(by)) { layout->collision_limited=1; return 0; }
    if (ax==a->x && ay==a->y && bx==b->x && by==b->y) return 0;
    a->x=ax; a->y=ay; b->x=bx; b->y=by;
    *remaining_a=fmaxf(0,*remaining_a-(float)move_a); *remaining_b=fmaxf(0,*remaining_b-(float)move_b);
    /* Remove compression velocity on corrected axis; springs remain responsible
       for overall grouping. Never move pointer-held node. */
    if (dx) { if (first!=held) a->vx=0; if (second!=held) b->vx=0; }
    else { if (first!=held) a->vy=0; if (second!=held) b->vy=0; }
    return 1;
}
static void collision_pass(GD_Layout *layout,int held) {
    int i,row,start,count=layout->node_count;
    if (count<2) return;
    for (i=0;i<count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i]; GD_LayoutCollisionItem *item=&layout->collision_order[i];
        if (!isfinite(node->x)||!isfinite(node->y)) { layout->collision_limited=1; return; }
        item->minimum=(double)node->x+node->footprint.left-GD_LAYOUT_CLEARANCE/2;
        item->maximum=(double)node->x+node->footprint.right+GD_LAYOUT_CLEARANCE/2; item->node=i;
    }
    collision_sort(layout->collision_order,count);
    start=(int)(layout->collision_cursor%(unsigned)count);
    for (row=0;row<count;++row) {
        int index=(start+row)%count,j; const GD_LayoutCollisionItem *a=&layout->collision_order[index];
        for (j=index+1;j<count;++j) {
            const GD_LayoutCollisionItem *b=&layout->collision_order[j];
            if (layout->collision_checks==GD_LAYOUT_COLLISION_WORK) {
                layout->collision_limited=1; layout->collision_cursor=(unsigned)((index+1)%count); return;
            }
            ++layout->collision_checks;
            if (b->minimum>=a->maximum) break;
            layout->collision_moves+=(unsigned)collision_pair(layout,a->node,b->node,held);
        }
    }
    /* A complete sweep keeps a fixed order near rest. Only exhausted work
       rotates rows; rotating every frame creates perpetual contact jitter. */
    layout->collision_cursor=0;
}
static void collision_separate(GD_Layout *layout,int held) {
    int pass,i;
    for (i=0;i<layout->node_count;++i) layout->collision_remaining[i]=i==held ? 0:GD_LAYOUT_SEPARATION_SHIFT;
    layout->collision_checks=layout->collision_moves=0; layout->collision_limited=0;
    for (pass=0;pass<GD_LAYOUT_COLLISION_PASSES;++pass) {
        unsigned before=layout->collision_moves;
        collision_pass(layout,held);
        if (layout->collision_checks==GD_LAYOUT_COLLISION_WORK) { layout->collision_limited=1; break; }
        if (layout->collision_moves==before) break;
    }
    if (pass==GD_LAYOUT_COLLISION_PASSES) layout->collision_limited=1;
}
#undef GD_LAYOUT_CLEARANCE
#undef GD_LAYOUT_COLLISION_PASSES
#undef GD_LAYOUT_SEPARATION_SHIFT
#undef GD_LAYOUT_CONTACT_EPSILON
#undef GD_LAYOUT_CONTACT_RELAXATION
#endif
