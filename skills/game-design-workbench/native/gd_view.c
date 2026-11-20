/* Native standalone interaction matrix. Source edits remain outside the view. */
#include "gd.h"
#include "gd_matrix_grid.h"
#include "gd_noun_label.h"
#include "gd_native_window.h"
#include "gd_native_canvas.h"
#include "gd_native_ui.h"
#include <stdio.h>
#include <string.h>

#define MATRIX_RGB(r,g,b) ((ImU32)(r)|((ImU32)(g)<<8)|((ImU32)(b)<<16)|0xff000000u)
static const ImU32 matrix_background=MATRIX_RGB(18,24,34),matrix_panel=MATRIX_RGB(29,38,51);
static const ImU32 matrix_foreground=MATRIX_RGB(226,233,240),matrix_muted=MATRIX_RGB(160,179,199);
static const ImU32 matrix_accent=MATRIX_RGB(66,178,167),matrix_selected=MATRIX_RGB(66,109,162);
static const ImU32 matrix_warning=MATRIX_RGB(236,190,102);
typedef struct {float x,y,width,height;} MatrixArea;
static int matrix_hit(ImVec2_c point,MatrixArea area) {
    return point.x>=area.x && point.x<=area.x+area.width && point.y>=area.y && point.y<=area.y+area.height;
}
static void matrix_box(GD_NativeCanvas *canvas,MatrixArea area,ImU32 color,float outline) {
    gd_canvas_rect(canvas,(ImVec2_c){area.x,area.y},(ImVec2_c){area.x+area.width,area.y+area.height},color,0,outline);
}
static int matrix_button(GD_NativeCanvas *canvas,MatrixArea area,const char *label,ImVec2_c mouse,int click) {
    int hover=matrix_hit(mouse,area);
    matrix_box(canvas,area,hover ? matrix_selected:matrix_panel,0);
    gd_canvas_text(canvas,(ImVec2_c){area.x+8,area.y+6},label,16,matrix_foreground);
    return hover && click;
}
/* Decoration uses exact record classification and complete display label.
   No font asset or label-based classification; same policy as legacy frontend. */
static void matrix_label(GD_NativeCanvas *canvas,const GD_Record *record,const char *text,int x,int y,int width,int size,ImU32 color) {
    char prefix[GD_MAX_PATH];ImVec2_c position={(float)x,(float)y};
    if(!gd_canvas_clip(canvas,position,(ImVec2_c){x+width,y+size+5}))return;
    gd_canvas_text(canvas,position,text,(float)size,color);
    if(gd_noun_label_prefix(record,text,prefix,sizeof(prefix))) {
        const char *line=strrchr(prefix,'\n');
        ImVec2_c offset=gd_canvas_measure(line ? line+1:prefix,(float)size);
        ImVec2_c extent=gd_canvas_measure(record->display_name,(float)size);
        float line_y=0;
        for(const char *at=prefix;*at;++at)if(*at=='\n')line_y+=size;
        position.x+=offset.x;position.y+=line_y;
        if(gd_noun_label_style(record)==GD_NOUN_LABEL_BOLD) {
            position.x+=1;gd_canvas_text(canvas,position,record->display_name,(float)size,color);
        } else gd_canvas_line(canvas,(ImVec2_c){position.x,position.y+size+1},
            (ImVec2_c){position.x+extent.x,position.y+size+1},color,1);
    }
    gd_canvas_unclip(canvas);
}
static ImU32 matrix_count_color(GD_CountRange range,int count) {
    float fraction=gd_count_fraction(range,count);
    if(count<=0 || !range.maximum)return matrix_panel;
    return MATRIX_RGB((unsigned char)(32-8*fraction),(unsigned char)(48+64*fraction),(unsigned char)(82+14*fraction));
}
static void matrix_count_legend(GD_NativeCanvas *canvas,GD_CountRange range,int x,int y,int width) {
    char label[96];
    if(width<1)return;
    if(!range.maximum) {
        gd_canvas_text(canvas,(ImVec2_c){x,y},"Declared cases: no populated cells",12,matrix_foreground);return;
    }
    snprintf(label,sizeof(label),"Declared cases min %d",range.minimum);
    gd_canvas_text(canvas,(ImVec2_c){x,y},label,12,matrix_foreground);
    snprintf(label,sizeof(label),"max %d",range.maximum);
    gd_canvas_text(canvas,(ImVec2_c){x+width-gd_canvas_measure(label,12).x,y},label,12,matrix_foreground);
    ImU32 low=matrix_count_color(range,range.minimum),high=matrix_count_color(range,range.maximum);
    ImDrawList_AddRectFilledMultiColor(canvas->draw,(ImVec2_c){x,y+16},(ImVec2_c){x+width,y+25},low,high,high,low);
}
/* Loaded Markdown is UTF-8. Dependency decoder advances whole codepoints;
   fixed buffer/viewport bound work and preserve original bytes/order. */
static void matrix_body(GD_NativeCanvas *canvas,const char *text,int x,float y,int width,int top,int bottom) {
    char line[1024];int used=0;const int size=16,height=21;
    while(*text && y<bottom) {
        unsigned int codepoint=0;int bytes=igImTextCharFromUtf8(&codepoint,text,NULL);
        if(bytes<=0 || bytes>4)break;
        if(codepoint=='\r'){text+=bytes;continue;}
        float measured=width+1;
        if(used+bytes<(int)sizeof(line)) {
            memcpy(line+used,text,(size_t)bytes);line[used+bytes]=0;measured=gd_canvas_measure(line,size).x;
        }
        if(codepoint=='\n' || measured>width || used+bytes>=(int)sizeof(line)) {
            line[used]=0;
            if(y+height>=top)gd_canvas_text(canvas,(ImVec2_c){x,y},line,size,matrix_foreground);
            y+=height;used=0;
            if(codepoint=='\n'){text+=bytes;continue;}
        }
        memcpy(line+used,text,(size_t)bytes);used+=bytes;text+=bytes;
    }
    line[used]=0;
    if(y+height>=top && y<bottom)gd_canvas_text(canvas,(ImVec2_c){x,y},line,size,matrix_foreground);
}
static int matrix_axes(const GD_Model *model,int *nouns,int *verbs,int *verb_count) {
    int count=0;*verb_count=0;
    for(int i=0;i<model->record_count;++i) {
        if(model->records[i].role==GD_NOUN)nouns[count++]=i;
        if(model->records[i].role==GD_VERB)verbs[(*verb_count)++]=i;
    }
    return count;
}
/* Visibility is explicit for owned hidden qualification. Production gd_view
   always uses visible/no-activation; this frontend owns no editable drafts. */
static int native_matrix_view(GD_Model *initial,int frames,const char *screenshot,GD_WindowVisibility visibility) {
    GD_Model *model=initial;GD_NativeWindow window={0};GD_NativeCanvas canvas={0};
    int nouns[GD_MAX_RECORDS],verbs[GD_MAX_RECORDS],verb_count,noun_count=matrix_axes(model,nouns,verbs,&verb_count);
    int row=0,column=0,first_row=0,first_column=0,case_number=0,linked=-1,frame=0,stale=0,result=0;
    float scroll=0;double next_check=0;char reload_message[256]="";
    if(model->cell_count) {
        for(int i=0;i<noun_count;++i)if(nouns[i]==model->cells[0].noun)row=i;
        for(int i=0;i<verb_count;++i)if(verbs[i]==model->cells[0].verb)column=i;
    }
    if(!gd_native_window_open(&window,"Game Design Workbench — interaction matrix",1280,800,960,600,visibility)) {
        fprintf(stderr,"gd: cannot initialize native matrix window: %s\n",window.error);return 2;
    }
    while(!frames || frame<frames) {
        int ready=gd_native_window_begin(&window);
        if(ready<0){fprintf(stderr,"gd: native matrix frame failed: %s\n",window.error);result=2;break;}
        if(window.close_requested)break;
        if(!ready)continue;
        if(igIsKeyPressed_Bool(ImGuiKey_Escape,false))break;
        ImGuiIO *io=igGetIO_Nil();int width=(int)io->DisplaySize.x,height=(int)io->DisplaySize.y;
        int split=width*57/100,grid_left=210,grid_top=164,row_height=38,column_width=148;
        int visible_rows=(height-grid_top-52)/row_height,visible_columns=(split-grid_left-16)/column_width;
        int old_row=row,old_column=column,old_case=case_number,cell,case_record=-1;
        ImVec2_c mouse=io->MousePos;float wheel=io->MouseWheel;int click=igIsMouseClicked_Bool(ImGuiMouseButton_Left,false);
        char label[1024];
        if(gd_native_window_seconds(&window)>=next_check) {
            if(gd_sources_changed(model))stale=1;next_check=gd_native_window_seconds(&window)+1;
        }
        if(igIsKeyPressed_Bool(ImGuiKey_F5,false)) {
            GD_Model *fresh=gd_create(model->allocator);char noun_reference[GD_MAX_PATH]="",verb_reference[GD_MAX_PATH]="";
            if(noun_count)snprintf(noun_reference,sizeof(noun_reference),"%s",model->records[nouns[row]].reference);
            if(verb_count)snprintf(verb_reference,sizeof(verb_reference),"%s",model->records[verbs[column]].reference);
            if(fresh && gd_load(fresh,model->collections,model->collection_count)) {
                GD_Model *old=model;model=fresh;noun_count=matrix_axes(model,nouns,verbs,&verb_count);row=column=0;
                for(int i=0;i<noun_count;++i)if(!strcmp(noun_reference,model->records[nouns[i]].reference))row=i;
                for(int i=0;i<verb_count;++i)if(!strcmp(verb_reference,model->records[verbs[i]].reference))column=i;
                if(old!=initial)gd_destroy(old);
                stale=0;reload_message[0]=0;case_number=0;linked=-1;scroll=0;
            } else {
                stale=1;snprintf(reload_message,sizeof(reload_message),"%s",fresh && fresh->error_count ? fresh->errors[0].message:"Cannot allocate reload storage");
                fprintf(stderr,"gd: refresh failed; retaining stale view: %s\n",reload_message);gd_destroy(fresh);
            }
        }
        if(noun_count && verb_count) {
            if(igIsKeyPressed_Bool(ImGuiKey_DownArrow,true) && row+1<noun_count)++row;
            if(igIsKeyPressed_Bool(ImGuiKey_UpArrow,true) && row>0)--row;
            if(igIsKeyPressed_Bool(ImGuiKey_RightArrow,true) && column+1<verb_count)++column;
            if(igIsKeyPressed_Bool(ImGuiKey_LeftArrow,true) && column>0)--column;
        }
        if(wheel && mouse.x<split) {
            if(io->KeyShift)first_column-=(int)wheel;else first_row-=(int)wheel;
        } else if(wheel)scroll-=wheel*42;
        if(first_row<0)first_row=0;if(first_column<0)first_column=0;
        if(first_row>noun_count-1)first_row=noun_count ? noun_count-1:0;
        if(first_column>verb_count-1)first_column=verb_count ? verb_count-1:0;
        if(!frame || old_row!=row || old_column!=column) {
            if(row<first_row)first_row=row;if(row>=first_row+visible_rows)first_row=row-visible_rows+1;
            if(column<first_column)first_column=column;if(column>=first_column+visible_columns)first_column=column-visible_columns+1;
            case_number=0;linked=-1;scroll=0;
        }
        if(scroll<0)scroll=0;
        GD_CountRange counts={0};for(int i=0;i<model->cell_count;++i)gd_count_range_add(&counts,model->cells[i].pair_count);
        if(!gd_canvas_begin(&canvas,(GD_CanvasCamera){{0,0},{0,0},1})){result=2;break;}
        gd_canvas_text(&canvas,(ImVec2_c){20,18},"Interaction matrix",26,matrix_foreground);
        snprintf(label,sizeof(label),"%d source records | %d declared cells | %d unplaced cases",model->record_count,model->cell_count,model->unplaced_cases);
        gd_canvas_text(&canvas,(ImVec2_c){20,54},label,16,matrix_muted);
        gd_canvas_text(&canvas,(ImVec2_c){20,82},"Arrows: select | Wheel: pan rows | Shift+wheel: columns | F5: reload",16,matrix_muted);
        matrix_count_legend(&canvas,counts,20,104,split-40);
        if(stale) {
            matrix_box(&canvas,(MatrixArea){0,height-34,width,34},MATRIX_RGB(76,57,31),0);
            if(reload_message[0])snprintf(label,sizeof(label),"STALE — refresh failed: %s",reload_message);
            else snprintf(label,sizeof(label),"STALE — source changed. F5 reloads; displayed data is the last loaded snapshot.");
            matrix_label(&canvas,NULL,label,16,height-26,width-32,16,matrix_warning);
        } else gd_canvas_text(&canvas,(ImVec2_c){20,height-26},"Active = accepted + defined case. Other cases stay visible. Blank is not a defect.",16,matrix_muted);
        for(int j=first_column;j<verb_count && j<first_column+visible_columns;++j) {
            int x=grid_left+(j-first_column)*column_width;
            matrix_label(&canvas,&model->records[verbs[j]],model->records[verbs[j]].display_name,x+6,grid_top-25,column_width-10,16,matrix_foreground);
        }
        for(int i=first_row;i<noun_count && i<first_row+visible_rows;++i) {
            int y=grid_top+(i-first_row)*row_height;
            snprintf(label,sizeof(label),"%s (%d)",model->records[nouns[i]].display_name,model->connectivity[nouns[i]].partners);
            matrix_label(&canvas,&model->records[nouns[i]],label,20,y+10,grid_left-30,16,matrix_foreground);
            for(int j=first_column;j<verb_count && j<first_column+visible_columns;++j) {
                MatrixArea area={grid_left+(j-first_column)*column_width,y,column_width-3,row_height-3};
                int index=gd_matrix_grid_cell(model,nouns[i],verbs[j]);
                matrix_box(&canvas,area,matrix_count_color(counts,index>=0 ? model->cells[index].pair_count:0),0);
                if(row==i && column==j)matrix_box(&canvas,area,matrix_warning,2);
                if(index>=0)snprintf(label,sizeof(label),"%d cases%s",model->cells[index].pair_count,model->cells[index].active ? " *":"");
                else snprintf(label,sizeof(label),"-");
                gd_canvas_text(&canvas,(ImVec2_c){area.x+10,y+10},label,16,matrix_foreground);
                if(matrix_hit(mouse,area) && click){row=i;column=j;case_number=0;linked=-1;scroll=0;}
            }
        }
        gd_canvas_line(&canvas,(ImVec2_c){split,102},(ImVec2_c){split,height-42},matrix_muted,1);
        cell=noun_count && verb_count ? gd_matrix_grid_cell(model,nouns[row],verbs[column]):-1;
        if(cell>=0) {
            const GD_Cell *selected=&model->cells[cell];int x=split+18,detail_width=width-x-18,body_top=294;const GD_Record *record;
            if(igIsKeyPressed_Bool(ImGuiKey_PageDown,true))++case_number;
            if(igIsKeyPressed_Bool(ImGuiKey_PageUp,true))--case_number;
            if(case_number<0)case_number=selected->pair_count-1;case_number%=selected->pair_count;
            if(old_case!=case_number){linked=-1;scroll=0;}
            case_record=model->pairs[selected->first_pair+case_number].interaction;record=&model->records[case_record];
            snprintf(label,sizeof(label),"%s x %s",model->records[selected->noun].display_name,model->records[selected->verb].display_name);
            matrix_label(&canvas,&model->records[selected->noun],label,x,110,detail_width,20,matrix_foreground);
            snprintf(label,sizeof(label),"Case %d/%d: %s",case_number+1,selected->pair_count,record->display_name);
            matrix_label(&canvas,record,label,x,144,detail_width,17,matrix_foreground);
            snprintf(label,sizeof(label),"status: %s | coverage: %s",record->status ? record->status:"unset",record->coverage ? record->coverage:"unset");
            matrix_label(&canvas,NULL,label,x,172,detail_width,16,matrix_muted);
            if(matrix_button(&canvas,(MatrixArea){x,201,120,30},"Previous",mouse,click) && selected->pair_count>1){--case_number;if(case_number<0)case_number+=selected->pair_count;linked=-1;scroll=0;}
            if(matrix_button(&canvas,(MatrixArea){x+128,201,120,30},"Next",mouse,click) && selected->pair_count>1){case_number=(case_number+1)%selected->pair_count;linked=-1;scroll=0;}
            if(linked>=0 && matrix_button(&canvas,(MatrixArea){x+256,201,135,30},"Back to case",mouse,click)){linked=-1;scroll=0;}
            if(linked>=0)record=&model->records[linked];
            matrix_label(&canvas,NULL,record->reference,x,243,detail_width,17,matrix_accent);
            matrix_label(&canvas,NULL,record->source_path,x,268,detail_width,14,matrix_muted);
            if(igIsKeyPressed_Bool(ImGuiKey_C,false))igSetClipboardText(record->source_path);
            if(gd_canvas_clip(&canvas,(ImVec2_c){x,body_top},(ImVec2_c){x+detail_width,height-62})) {
                float y=body_top-scroll;
                for(int i=0;i<model->edge_count;++i) {
                    GD_Edge *edge=&model->edges[i];int source=linked>=0 ? linked:case_record;
                    if(edge->source!=source)continue;
                    if(y+24>=body_top && y<height-62) {
                        snprintf(label,sizeof(label),"%s: %s%s",gd_field_name(edge->field),edge->reference,edge->target<0 ? " [not uniquely loaded]":"");
                        gd_canvas_text(&canvas,(ImVec2_c){x,y},label,15,edge->target>=0 ? matrix_accent:matrix_muted);
                        if(edge->target>=0 && matrix_hit(mouse,(MatrixArea){x,y,detail_width,22}) && click){linked=edge->target;scroll=0;break;}
                    }
                    y+=24;
                }
                matrix_body(&canvas,record->body,x,y+14,detail_width-6,body_top,height-62);gd_canvas_unclip(&canvas);
            }
            gd_canvas_text(&canvas,(ImVec2_c){x,height-52},"PageUp/Down: cases | Wheel: detail | C: copy source path",14,matrix_muted);
        } else {
            gd_canvas_text(&canvas,(ImVec2_c){split+18,120},noun_count && verb_count ? "No declared case in this cell.":"No noun/verb axes in the supplied folders.",17,matrix_foreground);
            gd_canvas_text(&canvas,(ImVec2_c){split+18,150},"No rule, exclusion, or defect inferred.",16,matrix_muted);
        }
        if(!gd_canvas_end(&canvas) || !gd_native_window_end(&window,18/255.0f,24/255.0f,34/255.0f,1)) {
            fprintf(stderr,"gd: native matrix render failed: %s\n",window.error);result=2;break;
        }
        ++frame;
        if(screenshot && frame==frames && !gd_native_ui_capture_png(screenshot)) {
            fprintf(stderr,"gd: screenshot export failed: %s\n",gd_native_ui_error());result=2;
        }
    }
    if(!gd_native_window_close(&window)){fprintf(stderr,"gd: native matrix close failed: %s\n",window.error);result=2;}
    if(model!=initial)gd_destroy(model);
    return result;
}
int gd_view(GD_Model *model,int frames,const char *screenshot) {
    return native_matrix_view(model,frames,screenshot,GD_WINDOW_VISIBLE);
}
#undef MATRIX_RGB
