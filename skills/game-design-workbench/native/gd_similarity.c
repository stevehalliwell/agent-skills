#include "gd_similarity.h"
#include <math.h>
#include <string.h>
#include <float.h>

typedef struct {
    double distances[GD_MAX_RECORDS][GD_SIMILARITY_LANDMARKS];
    double matrix[GD_SIMILARITY_LANDMARKS][GD_SIMILARITY_LANDMARKS];
    double vectors[GD_SIMILARITY_LANDMARKS][GD_SIMILARITY_LANDMARKS];
    double means[GD_SIMILARITY_LANDMARKS],nearest[GD_MAX_RECORDS];
    int merge[GD_MAX_RECORDS],landmark_rows[GD_SIMILARITY_LANDMARKS];
} GD_SimilarityScratch;
static int spend(int *work,int limit,int count) {
    if (count<0 || *work<0 || *work>limit || count>limit-*work) return 0;
    *work+=count; return 1;
}
static int text_size(const char *text,int *work,int limit) {
    int size=0;
    if (!text) return -1;
    while (size<=GD_MAX_RULE_LINE) {
        if (!spend(work,limit,1)) return -2;
        if (!text[size]) return size;
        ++size;
    }
    return -1;
}
static int compare_text(const char *first,const char *second,int *value,int *work,int limit) {
    int i;
    for (i=0;i<=GD_MAX_RULE_LINE;++i) {
        unsigned char a,b;
        if (!spend(work,limit,1)) return 0;
        a=(unsigned char)first[i]; b=(unsigned char)second[i];
        if (a!=b || !a) { *value=(a>b)-(a<b); return 1; }
    }
    return 0;
}
static const GD_SimilarityCell *cell(const GD_Similarity *result,int entry,int feature) {
    return &result->input.cells[(size_t)entry*result->input.feature_count+feature];
}
int gd_similarity_distance(const GD_Similarity *result,int first,int second,double *components,double *distance,int *work,int limit) {
    int feature; double sum=0,total=0,maximum=0;
    if (!result || !result->input_complete || !distance || !work || limit<0 || limit>GD_SIMILARITY_WORK || first<0 || second<0 || first>=result->input.entry_count || second>=result->input.entry_count) return 0;
    for (feature=0;feature<result->input.feature_count;++feature) {
        const GD_SimilarityFeature *descriptor=&result->input.features[feature];
        const GD_SimilarityCell *a=cell(result,first,feature),*b=cell(result,second,feature);
        if (!spend(work,limit,1) || !a->present || !b->present) return 0;
        if (descriptor->weight>maximum) maximum=descriptor->weight;
    }
    if (!maximum) return 0;
    for (feature=0;feature<result->input.feature_count;++feature) {
        const GD_SimilarityFeature *descriptor=&result->input.features[feature];
        const GD_SimilarityCell *a=cell(result,first,feature),*b=cell(result,second,feature);
        double value=0,weight=descriptor->weight/maximum;
        if (!spend(work,limit,1)) return 0;
        if (descriptor->kind==GD_SIMILARITY_NUMBER) {
            double low=result->low[feature],high=result->high[feature],range=high-low;
            /* Preserve small differences at large offsets. Scale only when the
               finite endpoints' subtraction would overflow. */
            if (high!=low) value=isfinite(range) ? fabs(a->number-b->number)/range:
                fabs(a->number*0.5-b->number*0.5)/(high*0.5-low*0.5);
        } else if (descriptor->kind==GD_SIMILARITY_NAME) {
            int comparison; if (!compare_text(a->name,b->name,&comparison,work,limit)) return 0; value=comparison ? 1:0;
        } else {
            int i=0,j=0,shared=0,combined;
            while (i<a->count && j<b->count) {
                if (!spend(work,limit,1)) return 0;
                if (a->members[i]==b->members[j]) { ++shared; ++i; ++j; }
                else if (a->members[i]<b->members[j]) ++i; else ++j;
            }
            combined=a->count+b->count-shared; value=combined ? 1-(double)shared/combined:0;
        }
        if (components) components[feature]=value;
        sum+=weight*value; total+=weight;
    }
    *distance=sum/total; return 1;
}
static int checked_text(GD_Similarity *result,const char *text,int required) {
    int length=text_size(text,&result->work,result->input.work_limit);
    if (length==-2) result->limited=1;
    return length>=required;
}
static int prepare(GD_Similarity *result,GD_SimilarityScratch *scratch) {
    const GD_SimilarityInput *input=&result->input; int i,j,width; double weights=0;
    for (j=0;j<input->feature_count;++j) {
        const GD_SimilarityFeature *feature=&input->features[j];
        if (feature->kind<GD_SIMILARITY_NUMBER || feature->kind>GD_SIMILARITY_SET || !isfinite(feature->weight) || feature->weight<0 || !checked_text(result,feature->name,1) || (feature->unit && !checked_text(result,feature->unit,0))) return 0;
        if (feature->weight>weights) weights=feature->weight;
        result->low[j]=DBL_MAX; result->high[j]=-DBL_MAX;
    }
    if (!weights) return 0;
    for (i=0;i<input->entry_count;++i) {
        int complete=1;
        if (!checked_text(result,input->ids[i],1)) return 0;
        for (j=0;j<input->feature_count;++j) {
            const GD_SimilarityCell *value=cell(result,i,j); int k;
            if (!spend(&result->work,input->work_limit,1)) { result->limited=1; return 0; }
            if (!value->present) { complete=0; continue; }
            if (input->features[j].kind==GD_SIMILARITY_NUMBER) { if (!isfinite(value->number)) return 0; }
            else if (input->features[j].kind==GD_SIMILARITY_NAME) { if (!checked_text(result,value->name,0)) return 0; }
            else {
                if (value->count<0 || value->count>GD_MAX_PAIRS || (value->count && !value->members)) return 0;
                for (k=0;k<value->count;++k) {
                    if (!spend(&result->work,input->work_limit,1)) { result->limited=1; return 0; }
                    if (value->members[k]<0 || (k && value->members[k]<=value->members[k-1])) return 0;
                }
            }
        }
        /* Sort all IDs, including unplaced rows, to reject duplicate identity. */
        result->order[i]=i; result->placed[i]=(unsigned char)complete;
        if (!complete) ++result->missing_entries;
    }
    for (width=1;width<input->entry_count;width*=2) {
        int start;
        for (start=0;start<input->entry_count;start+=2*width) {
            int a=start,b=start+width,end=b+width,stop=b,position=start;
            if (stop>input->entry_count) stop=input->entry_count;
            if (end>input->entry_count) end=input->entry_count;
            while (a<stop || b<end) {
                int comparison=0;
                if (!spend(&result->work,input->work_limit,1) || (a<stop && b<end && !compare_text(input->ids[result->order[a]],input->ids[result->order[b]],&comparison,&result->work,input->work_limit))) { result->limited=1; return 0; }
                scratch->merge[position++]=a<stop && (b>=end || comparison<=0) ? result->order[a++]:result->order[b++];
            }
        }
        memcpy(result->order,scratch->merge,(size_t)input->entry_count*sizeof(int));
    }
    for (i=0;i<input->entry_count;++i) {
        int entry=result->order[i];
        if (i) {
            int comparison;
            if (!compare_text(input->ids[entry],input->ids[result->order[i-1]],&comparison,&result->work,input->work_limit)) { result->limited=1; return 0; }
            if (!comparison) return 0;
        }
        if (result->placed[entry]) {
            scratch->merge[result->complete_entries++]=entry;
            for (j=0;j<input->feature_count;++j) {
                if (!spend(&result->work,input->work_limit,1)) { result->limited=1; return 0; }
                if (input->features[j].kind==GD_SIMILARITY_NUMBER) {
                    double value=cell(result,entry,j)->number;
                    if (value<result->low[j]) result->low[j]=value;
                    if (value>result->high[j]) result->high[j]=value;
                }
            }
        }
    }
    memcpy(result->order,scratch->merge,(size_t)result->complete_entries*sizeof(int)); return 1;
}
static int fit(GD_Similarity *result,GD_SimilarityScratch *scratch) {
    int i,j,k,count=result->complete_entries,landmarks=count<GD_SIMILARITY_LANDMARKS ? count:GD_SIMILARITY_LANDMARKS,chosen=0,sweep;
    double grand=0,scale=0;
    if (!count) return 1;
    for (i=0;i<count;++i) scratch->nearest[i]=DBL_MAX;
    for (j=0;j<landmarks;++j) {
        int next=-1; double farthest=-1;
        result->landmarks[j]=result->order[chosen]; scratch->landmark_rows[j]=chosen;
        for (i=0;i<count;++i) {
            double distance;
            if (!gd_similarity_distance(result,result->order[i],result->landmarks[j],NULL,&distance,&result->work,result->input.work_limit)) return 0;
            scratch->distances[i][j]=distance;
            if (distance>result->distance_scale) result->distance_scale=distance;
            if (distance<scratch->nearest[i]) scratch->nearest[i]=distance;
        }
        result->landmark_count=j+1;
        for (i=0;i<count;++i) {
            int used=0;
            for (k=0;k<=j;++k) if (result->order[i]==result->landmarks[k]) used=1;
            if (!spend(&result->work,result->input.work_limit,j+2)) return 0;
            if (!used && scratch->nearest[i]>farthest) { farthest=scratch->nearest[i]; next=i; }
        }
        chosen=next;
    }
    /* Normalize before squaring: small but representable distances must not
       disappear merely because their squares underflow. Restore scale below. */
    for (i=0;i<count;++i) for (j=0;j<landmarks;++j) {
        double distance=result->distance_scale ? scratch->distances[i][j]/result->distance_scale:0;
        if (!spend(&result->work,result->input.work_limit,1)) return 0;
        scratch->distances[i][j]=distance*distance;
    }
    for (i=0;i<landmarks;++i) {
        int row=scratch->landmark_rows[i];
        for (j=0;j<landmarks;++j) {
            if (!spend(&result->work,result->input.work_limit,1)) return 0;
            scratch->means[i]+=scratch->distances[row][j]/landmarks;
        }
        grand+=scratch->means[i]/landmarks;
    }
    for (i=0;i<landmarks;++i) for (j=0;j<landmarks;++j) {
        int row=scratch->landmark_rows[i];
        if (!spend(&result->work,result->input.work_limit,1)) return 0;
        scratch->matrix[i][j]=-0.5*(scratch->distances[row][j]-scratch->means[i]-scratch->means[j]+grand);
        scratch->vectors[i][j]=i==j ? 1:0;
        if (fabs(scratch->matrix[i][j])>scale) scale=fabs(scratch->matrix[i][j]);
    }
    /* Cyclic Jacobi: fixed pair order and finite sweep ceiling. */
    for (sweep=0;sweep<64;++sweep) {
        int changed=0;
        for (i=0;i<landmarks;++i) for (j=i+1;j<landmarks;++j) {
            double cross=scratch->matrix[i][j],angle,c,s,first=scratch->matrix[i][i],second=scratch->matrix[j][j];
            if (!spend(&result->work,result->input.work_limit,1)) return 0;
            if (fabs(cross)<=scale*1e-13) continue;
            angle=0.5*atan2(2*cross,second-first); c=cos(angle); s=sin(angle); ++changed;
            scratch->matrix[i][i]=c*c*first-2*s*c*cross+s*s*second;
            scratch->matrix[j][j]=s*s*first+2*s*c*cross+c*c*second;
            scratch->matrix[i][j]=scratch->matrix[j][i]=0;
            for (k=0;k<landmarks;++k) {
                double a,b;
                if (!spend(&result->work,result->input.work_limit,1)) return 0;
                if (k!=i && k!=j) {
                    a=scratch->matrix[k][i]; b=scratch->matrix[k][j];
                    scratch->matrix[k][i]=scratch->matrix[i][k]=c*a-s*b;
                    scratch->matrix[k][j]=scratch->matrix[j][k]=s*a+c*b;
                }
                a=scratch->vectors[k][i]; b=scratch->vectors[k][j];
                scratch->vectors[k][i]=c*a-s*b; scratch->vectors[k][j]=s*a+c*b;
            }
        }
        if (!changed) break;
    }
    if (sweep==64) { result->failure=GD_SIMILARITY_FIT_LIMIT; return 0; }
    for (i=0;i<landmarks;++i) if (scratch->matrix[i][i]<-scale*1e-10) { ++result->negative_eigenvalues; result->negative_eigenvalue_mass-=scratch->matrix[i][i]; }
    for (k=0;k<2;++k) {
        int eigen=-1,pivot=0; double largest=scale*1e-10,sign;
        for (i=0;i<landmarks;++i) if (scratch->matrix[i][i]>largest) { largest=scratch->matrix[i][i]; eigen=i; }
        if (eigen<0) break;
        for (i=1;i<landmarks;++i) if (fabs(scratch->vectors[i][eigen])>fabs(scratch->vectors[pivot][eigen])) pivot=i;
        sign=scratch->vectors[pivot][eigen]<0 ? -1:1;
        for (i=0;i<count;++i) {
            double position=0,mean=0;
            for (j=0;j<landmarks;++j) {
                if (!spend(&result->work,result->input.work_limit,1)) return 0;
                mean+=scratch->distances[i][j]/landmarks;
            }
            for (j=0;j<landmarks;++j) {
                if (!spend(&result->work,result->input.work_limit,1)) return 0;
                position+=scratch->vectors[j][eigen]*(scratch->distances[i][j]-scratch->means[j]-mean+grand);
            }
            position*=(-0.5*sign)*result->distance_scale/sqrt(largest);
            if (!isfinite(position)) { result->failure=GD_SIMILARITY_NUMERIC_FAILURE; return 0; }
            if (k) result->y[result->order[i]]=position; else result->x[result->order[i]]=position;
        }
        scratch->matrix[eigen][eigen]=0;
    }
    if (result->distance_scale) {
        int represented=0;
        for (i=0;i<count;++i) if (result->x[result->order[i]] || result->y[result->order[i]]) represented=1;
        if (!represented) { result->failure=GD_SIMILARITY_NUMERIC_FAILURE; return 0; }
    }
    return 1;
}
static int distortion(GD_Similarity *result) {
    int count=result->complete_entries,total=count*(count-1)/2,samples=total<GD_SIMILARITY_PAIRS ? total:GD_SIMILARITY_PAIRS;
    int sample,first=0,start=0;
    result->distortion_complete=0;
    for (sample=0;sample<samples;++sample) {
        int ordinal=samples>1 ? (int)((int64_t)sample*(total-1)/(samples-1)):0,second,a,b;
        double distance,dx,dy,error;
        while (first<count-1 && ordinal>=start+count-first-1) { start+=count-first-1; ++first; }
        second=first+1+ordinal-start; a=result->order[first]; b=result->order[second];
        if (!gd_similarity_distance(result,a,b,NULL,&distance,&result->work,result->input.work_limit)) return 0;
        if (result->distance_scale) {
            distance/=result->distance_scale;
            dx=result->x[a]/result->distance_scale-result->x[b]/result->distance_scale;
            dy=result->y[a]/result->distance_scale-result->y[b]/result->distance_scale;
        } else dx=dy=0;
        error=sqrt(dx*dx+dy*dy)-distance;
        result->squared_error+=error*error; result->squared_distance+=distance*distance; ++result->distortion_pairs;
    }
    result->distortion_complete=result->distortion_pairs==total;
    return 1;
}
GD_Similarity *gd_similarity_build(GD_Allocator allocator,const GD_SimilarityInput *input) {
    GD_Similarity *result; GD_SimilarityScratch *scratch;
    if (!allocator.allocate || !allocator.release || !input || input->entry_count<0 || input->entry_count>GD_MAX_RECORDS || input->feature_count<1 || input->feature_count>GD_SIMILARITY_FEATURES || input->work_limit<0 || input->work_limit>GD_SIMILARITY_WORK || !input->features || (input->entry_count && (!input->ids || !input->cells))) return NULL;
    result=allocator.allocate(allocator.context,sizeof(*result)); if (!result) return NULL;
    memset(result,0,sizeof(*result)); result->allocator=allocator; result->input=*input;
    scratch=allocator.allocate(allocator.context,sizeof(*scratch)); if (!scratch) { gd_similarity_destroy(result); return NULL; }
    memset(scratch,0,sizeof(*scratch));
    if (!prepare(result,scratch)) {
        if (!result->limited) { allocator.release(allocator.context,scratch); gd_similarity_destroy(result); return NULL; }
    } else {
        result->input_complete=1;
        if (fit(result,scratch) && distortion(result)) result->available=1;
        else result->limited=1;
    }
    if (!result->available) { if (!result->failure) result->failure=GD_SIMILARITY_WORK_LIMIT; memset(result->placed,0,sizeof(result->placed)); memset(result->x,0,sizeof(result->x)); memset(result->y,0,sizeof(result->y)); }
    allocator.release(allocator.context,scratch); return result;
}
size_t gd_similarity_peak_owned_bytes(void) { return sizeof(GD_Similarity)+sizeof(GD_SimilarityScratch); }
void gd_similarity_destroy(GD_Similarity *result) { if (result) result->allocator.release(result->allocator.context,result); }
