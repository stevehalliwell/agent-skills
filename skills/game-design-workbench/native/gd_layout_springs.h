/* Shared Free spring targets. Uses prepared routing's sorted unordered pairs;
   active parallel/reverse edges count as one neighbor. No per-frame allocation. */
#ifndef GD_LAYOUT_SPRINGS_H
#define GD_LAYOUT_SPRINGS_H
#include <math.h>
#include <string.h>
enum { SPRING_NEIGHBOR_CAP=64,SPRING_NEIGHBOR_AREA_SHARE=8 };
static void spring_neighbors_prepare(GD_Layout *layout) {
    int begin;
    memset(layout->spring_neighbors,0,(size_t)layout->node_count*sizeof(layout->spring_neighbors[0]));
    for(begin=0;begin<layout->edge_count;){
        const GD_LayoutLane *pair=&layout->lanes[begin];int end=begin,active=0;
        while(end<layout->edge_count && layout->lanes[end].a==pair->a && layout->lanes[end].b==pair->b){if(layout->edges[layout->lanes[end].edge].active)active=1;++end;}
        if(active && pair->a!=pair->b){++layout->spring_neighbors[pair->a];++layout->spring_neighbors[pair->b];}
        begin=end;
    }
}
static double spring_radius(const GD_LayoutNode *node,int neighbors) {
    double width=(double)node->footprint.right-node->footprint.left,height=(double)node->footprint.bottom-node->footprint.top;
    /* Each extra distinct neighbor adds one eighth of base exclusion area.
       Radius grows with its square root; cap fan-out at 64 for bounded demand.
       Existing adapter spacing remains a floor, not an added second gap. */
    double degree=fmax(1,fmin(SPRING_NEIGHBOR_CAP,neighbors));
    return 0.5*hypot(width,height)*sqrt(1+(degree-1)/SPRING_NEIGHBOR_AREA_SHARE);
}
static float spring_target(const GD_Layout *layout,int first,int second,double clearance,float minimum) {
    double radial=spring_radius(&layout->nodes[first],layout->spring_neighbors[first])+spring_radius(&layout->nodes[second],layout->spring_neighbors[second])+clearance;
    return (float)fmax(minimum,radial);
}
#endif
