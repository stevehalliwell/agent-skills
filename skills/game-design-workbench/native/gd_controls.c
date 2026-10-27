#include "gd.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SAFE_INTEGER 9007199254740991.0
enum { OP_NUMBER=1, OP_REFERENCE, OP_ADD, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE,
       OP_NEGATE, OP_POSITIVE, OP_MIN, OP_MAX, OP_CLAMP, OP_ABS, OP_FLOOR, OP_CEIL, OP_ROUND, OP_PAREN, OP_THIS, OP_UNKNOWN };

/* Decimal grammar shared by metadata, formula literals, and overrides. */
static const char *number_end(const char *p, const char *finish) {
    const char *start = p;
    if (p<finish && (*p=='+' || *p=='-')) ++p;
    if (p==finish || (!isdigit((unsigned char)*p) && *p!='.')) return start;
    if (*p=='.') { ++p; if (p==finish || !isdigit((unsigned char)*p)) return start; }
    else {
        while (p<finish && isdigit((unsigned char)*p)) ++p;
        if (p<finish && *p=='.') ++p;
    }
    while (p<finish && isdigit((unsigned char)*p)) ++p;
    if (p<finish && (*p=='e' || *p=='E')) {
        ++p; if (p<finish && (*p=='+' || *p=='-')) ++p;
        if (p==finish || !isdigit((unsigned char)*p)) return start;
        while (p<finish && isdigit((unsigned char)*p)) ++p;
    }
    return p;
}
/* Avoid accepting fractional decimal text rounded to an integer by strtod. */
static int exact_integer_text(const char *text) {
    const char *p = text;
    int fraction = 0, exponent = 0, negative = 0, last_nonzero = 0, digits = 0, decimal = 0;
    if (*p == '+' || *p == '-') ++p;
    while (*p && *p != 'e' && *p != 'E') {
        if (*p == '.') decimal = 1;
        else { ++digits; if (*p != '0') last_nonzero = digits; if (decimal) ++fraction; }
        ++p;
    }
    if (*p) {
        ++p; if (*p == '+' || *p == '-') negative = *p++ == '-';
        while (*p) { if (exponent < 100000) exponent = exponent*10+(*p-'0'); ++p; }
        if (negative) exponent = -exponent;
    }
    return !last_nonzero || exponent - fraction + digits - last_nonzero >= 0;
}
int gd_control_number(const char *text, double *value, int integer) {
    char *end;
    double number;
    if (!text || !*text || *number_end(text,text+strlen(text))) return 0;
    errno = 0; number = strtod(text,&end);
    if (*end || !isfinite(number) || (errno == ERANGE && number == 0)) return 0;
    if (integer && (fabs(number) > SAFE_INTEGER || floor(number) != number || !exact_integer_text(text))) return 0;
    *value = number; return 1;
}
static int unit_term(GD_Unit *unit, const char *symbol, size_t length, int power) {
    int i, j;
    if (!length || length > 32) return 0;
    if (!power) return 1;
    for (i = 0; i < unit->count; ++i) {
        if (strlen(unit->terms[i].symbol) == length && !memcmp(unit->terms[i].symbol,symbol,length)) {
            int combined = unit->terms[i].power + power;
            if (combined < -32 || combined > 32) return 0;
            unit->terms[i].power = combined;
            if (!combined) { for (j=i+1;j<unit->count;++j) unit->terms[j-1]=unit->terms[j]; --unit->count; }
            return 1;
        }
    }
    if (unit->count == GD_MAX_UNIT_TERMS || power < -32 || power > 32) return 0;
    i = unit->count++;
    memcpy(unit->terms[i].symbol,symbol,length); unit->terms[i].symbol[length]=0; unit->terms[i].power=power;
    for (;i>0 && strcmp(unit->terms[i-1].symbol,unit->terms[i].symbol)>0;--i) {
        GD_UnitTerm temporary = unit->terms[i-1]; unit->terms[i-1]=unit->terms[i]; unit->terms[i]=temporary;
    }
    return 1;
}
int gd_control_unit(const char *text, size_t length, GD_Unit *unit) {
    size_t i=0;
    int sign=1;
    memset(unit,0,sizeof(*unit));
    if (!text || !length || length>256) return 0;
    while (i<length) {
        size_t start;
        int power=1, negative=0;
        while (i<length && isspace((unsigned char)text[i])) ++i;
        start=i;
        if (i<length && text[i]=='1') ++i;
        else {
            if (i==length || !(isalpha((unsigned char)text[i]) || text[i]=='_') || (unsigned char)text[i]>=128) return 0;
            while (i<length && (isalnum((unsigned char)text[i]) || text[i]=='_') && (unsigned char)text[i]<128) ++i;
        }
        {
            size_t size=i-start;
            while (i<length && isspace((unsigned char)text[i])) ++i;
            if (i<length && text[i]=='^') {
                ++i; if (i<length && (text[i]=='+' || text[i]=='-')) negative=text[i++]=='-';
                if (i==length || !isdigit((unsigned char)text[i])) return 0;
                power=0;
                while (i<length && isdigit((unsigned char)text[i])) { power=power*10+(text[i++]-'0'); if (power>32) return 0; }
                if (negative) power=-power;
            }
            if (!(size==1 && text[start]=='1') && !unit_term(unit,text+start,size,power*sign)) return 0;
        }
        while (i<length && isspace((unsigned char)text[i])) ++i;
        if (i==length) return 1;
        if (text[i]!='*' && text[i]!='/') return 0;
        sign=text[i++]=='/' ? -1:1;
    }
    return 0;
}
int gd_expression_has_unknown(const GD_ControlToken *tokens, int count) {
    int i;
    if (!tokens || count<1 || count>GD_MAX_EXPRESSION_TOKENS) return 1;
    for (i=0;i<count;++i) if (tokens[i].operation==OP_UNKNOWN) return 1;
    return 0;
}
int gd_expression_is_starting_value(const GD_ControlToken *tokens, int count) {
    return tokens && count==1 && tokens[0].operation==OP_THIS;
}
int gd_expressions_equal(const GD_ControlToken *a, int count_a, const GD_ControlToken *b, int count_b) {
    int i;
    if (!a || !b || count_a<1 || count_a>GD_MAX_EXPRESSION_TOKENS || count_a!=count_b) return 0;
    for (i=0;i<count_a;++i) {
        GD_Unit first,second;
        if (a[i].operation!=b[i].operation || a[i].operation==OP_UNKNOWN) return 0;
        if (a[i].operation==OP_REFERENCE && a[i].target!=b[i].target) return 0;
        if (a[i].operation!=OP_NUMBER) continue;
        if (a[i].number!=b[i].number || a[i].rounded_fraction!=b[i].rounded_fraction) return 0;
        if (!gd_control_unit(a[i].unit ? a[i].unit:"1",a[i].unit ? (size_t)a[i].unit_length:1,&first) ||
            !gd_control_unit(b[i].unit ? b[i].unit:"1",b[i].unit ? (size_t)b[i].unit_length:1,&second) || !gd_units_equal(&first,&second)) return 0;
    }
    return 1;
}
int gd_units_equal(const GD_Unit *a, const GD_Unit *b) {
    int i;
    if (a->count!=b->count) return 0;
    for (i=0;i<a->count;++i) if (a->terms[i].power!=b->terms[i].power || strcmp(a->terms[i].symbol,b->terms[i].symbol)) return 0;
    return 1;
}
static int find_slice(const GD_Model *model, const char *text, size_t length) {
    int low=0, high=model->record_count;
    while (low<high) {
        int middle=low+(high-low)/2;
        const char *reference=model->records[middle].reference;
        /* Model sorts by role before canonical reference. Mixed rule/axis/control
           loads must search the control partition, not assume one global name sort. */
        int comparison=model->records[middle].role-GD_CONTROL;
        if (!comparison) {
            comparison=strncmp(reference,text,length);
            if (!comparison && reference[length]) comparison=1;
        }
        if (comparison<0) low=middle+1; else high=middle;
    }
    if (low==model->record_count || strlen(model->records[low].reference)!=length || memcmp(model->records[low].reference,text,length)) return -1;
    if (model->records[low].role!=GD_CONTROL || (low+1<model->record_count && !strcmp(model->records[low].reference,model->records[low+1].reference))) return -1;
    return low;
}
int gd_control_find(const GD_Model *model, const char *reference) {
    return reference ? find_slice(model,reference,strlen(reference)):-1;
}
static void control_error(GD_Model *model, int record, int field, const char *code, const char *message) {
    gd_error(model,code,message,model->records[record].source_path,model->records[record].control.lines[field]);
}
static int expression_error(GD_Model *model, GD_Expression *expression, GD_ExpressionSite site, const char *code, const char *message) {
    if (expression) expression->count=0;
    if (site.mode!=GD_EXPRESSION_CONTROL)
        code=!strcmp(code,"controls.expression_limit") ? "rules.quantity_limit":!strcmp(code,"controls.reference_syntax") ? "rules.reference_syntax":
             !strcmp(code,"controls.reference") ? "rules.quantity_reference":"rules.quantity_syntax";
    gd_error(model,code,message,site.record>=0 && site.record<model->record_count ? model->records[site.record].source_path:NULL,site.line);
    return 0;
}
static int expression_reference(const char *text, size_t size) {
    size_t i, part=0; int slash=0;
    for (i=0;i<size;++i) {
        unsigned c=(unsigned char)text[i];
        if (c=='/') { if (slash || !part) return 0; slash=1; part=0; continue; }
        if (!((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || (part && (c=='-' || c=='_'))) || ++part>128) return 0;
    }
    return slash && part;
}
static int emit(GD_Expression *controls, int source, int operation, int target, double number, const char *unit, int length, int offset) {
    GD_ControlToken *token;
    if (controls->count==controls->capacity || controls->count==GD_MAX_EXPRESSION_TOKENS) return 0;
    token=&controls->tokens[controls->count++]; memset(token,0,sizeof(*token));
    token->operation=operation; token->target=target; token->source=source; token->number=number;
    token->unit=unit; token->unit_length=length; token->offset=offset; return 1;
}
static int precedence(int operation) {
    return operation==OP_NEGATE || operation==OP_POSITIVE ? 3 : operation==OP_MULTIPLY || operation==OP_DIVIDE ? 2:1;
}
static int function_code(const char *name, size_t length) {
    static const char *names[]={"min","max","clamp","abs","floor","ceil","round"};
    int i;
    for (i=0;i<7;++i) if (strlen(names[i])==length && !memcmp(name,names[i],length)) return OP_MIN+i;
    return 0;
}
static int arity(int operation) { return operation==OP_CLAMP ? 3:operation==OP_MIN || operation==OP_MAX ? 2:1; }
/* ponytail: bounded shunting-yard bytecode, not an AST/code evaluator. The
   dependency solver below is iterative too; formula chains never use C stack. */
int gd_compile_expression(GD_Model *model, GD_Expression *controls, const char *text, size_t length, GD_ExpressionSite site) {
    const char *start=text ? text:"", *p=start, *finish=start+(text ? length:0);
    struct { int operation, function, arguments; } stack[GD_MAX_EXPRESSION_STACK];
    int depth=0, operand=1, record=site.record;
    if (!controls || !controls->tokens || controls->capacity<0 || controls->capacity>GD_MAX_EXPRESSION_TOKENS || !text || !length || length>4096) goto invalid;
    controls->count=0;
    while (p<finish) {
        int operation=0, function=0;
        while (p<finish && isspace((unsigned char)*p)) ++p;
        if (p==finish) break;
        if (operand && (*p=='+' || *p=='-')) { operation=*p++=='+' ? OP_POSITIVE:OP_NEGATE; goto push; }
        if (operand && (isdigit((unsigned char)*p) || *p=='.')) {
            const char *end=number_end(p,finish), *unit=NULL;
            char number[4097]; size_t size=(size_t)(end-p); int unit_length=0;
            double value;
            if (!size || size>=sizeof(number) || end>finish) goto invalid;
            memcpy(number,p,size); number[size]=0;
            if (!gd_control_number(number,&value,0)) goto invalid;
            p=end;
            if (p<finish && *p=='[') { unit=++p; while (p<finish && *p!=']') ++p; if (p==finish) goto invalid; unit_length=(int)(p-unit); ++p; }
            if (!emit(controls,record,OP_NUMBER,-1,value,unit,unit_length,(int)(end-start-size))) goto limit;
            controls->tokens[controls->count-1].rounded_fraction=floor(value)==value && !exact_integer_text(number);
            operand=0; continue;
        }
        if (p+1<finish && p[0]=='$' && p[1]=='{')
            return expression_error(model,controls,site,"controls.reference_syntax","Use {ALIAS/NAME} for control lookups; the dollar-prefixed syntax is not supported");
        if (operand && *p=='{') {
            const char *reference=p+1; int target, loaded=0, i;
            p=reference; while (p<finish && *p!='}') ++p;
            if (p==finish || p==reference || p-reference>257 || !expression_reference(reference,(size_t)(p-reference))) goto invalid;
            target=find_slice(model,reference,(size_t)(p-reference));
            for (i=0;i<model->collection_count;++i) if (model->collections[i].role==GD_CONTROL) loaded=1;
            if (target<0 && (site.mode==GD_EXPRESSION_CONTROL || loaded))
                return expression_error(model,controls,site,"controls.reference","Formula requires a unique loaded control reference");
            if (!emit(controls,record,OP_REFERENCE,target>=0 ? target:GD_EXPRESSION_UNLOADED_REFERENCE,0,reference,(int)(p-reference),(int)(reference-start-1))) goto limit;
            ++p; operand=0; continue;
        }
        if (operand && isalpha((unsigned char)*p)) {
            const char *name=p;
            while (p<finish && isalpha((unsigned char)*p)) ++p;
            if ((site.mode==GD_EXPRESSION_ASSIGNMENT && p-name==4 && !memcmp(name,"this",4)) ||
                (site.mode!=GD_EXPRESSION_CONTROL && p-name==7 && !memcmp(name,"UNKNOWN",7))) {
                if (!emit(controls,record,p-name==4 ? OP_THIS:OP_UNKNOWN,-1,0,NULL,0,(int)(name-start))) goto limit;
                operand=0; continue;
            }
            function=function_code(name,(size_t)(p-name));
            while (p<finish && isspace((unsigned char)*p)) ++p;
            if (!function || p==finish || *p!='(') goto invalid;
        }
        if (*p=='(' && operand) {
            ++p; if (depth==GD_MAX_EXPRESSION_STACK) goto limit;
            stack[depth].operation=OP_PAREN; stack[depth].function=function; stack[depth++].arguments=1; continue;
        }
        if ((*p==')' || *p==',') && !operand) {
            while (depth && stack[depth-1].operation!=OP_PAREN) {
                if (!emit(controls,record,stack[--depth].operation,-1,0,NULL,0,(int)(p-start))) goto limit;
            }
            if (!depth) goto invalid;
            if (*p==',') {
                if (!stack[depth-1].function || ++stack[depth-1].arguments>3) goto invalid;
                ++p; operand=1; continue;
            }
            --depth;
            if (stack[depth].function && (stack[depth].arguments!=arity(stack[depth].function) ||
                !emit(controls,record,stack[depth].function,-1,0,NULL,0,(int)(p-start)))) goto invalid;
            ++p; continue;
        }
        if (!operand) {
            if (*p=='+') operation=OP_ADD;
            else if (*p=='-') operation=OP_SUBTRACT;
            else if (*p=='*') operation=OP_MULTIPLY;
            else if (*p=='/') operation=OP_DIVIDE;
            if (!operation) goto invalid;
            while (depth && stack[depth-1].operation!=OP_PAREN && precedence(stack[depth-1].operation)>=precedence(operation)) {
                if (!emit(controls,record,stack[--depth].operation,-1,0,NULL,0,(int)(p-start))) goto limit;
            }
            ++p; operand=1;
        } else goto invalid;
    push:
        if (depth==GD_MAX_EXPRESSION_STACK) goto limit;
        stack[depth].operation=operation; stack[depth].function=0; stack[depth++].arguments=0;
    }
    if (operand) goto invalid;
    while (depth) {
        if (stack[depth-1].operation==OP_PAREN) goto invalid;
        if (!emit(controls,record,stack[--depth].operation,-1,0,NULL,0,(int)(p-start))) goto limit;
    }
    return 1;
limit:
    return expression_error(model,controls,site,"controls.expression_limit","Formula exceeds 4096 bytes, 256 tokens, 128 stack entries, or available shared-token capacity");
invalid:
    return expression_error(model,controls,site,"controls.expression","Invalid formula syntax, function arity, or literal; only documented arithmetic is supported");
}
static int compile(GD_Model *model, GD_Controls *controls, int record) {
    int remaining=GD_MAX_CONTROL_TOKENS-controls->token_count, valid;
    GD_Expression expression={controls->tokens+controls->token_count,0,remaining<GD_MAX_EXPRESSION_TOKENS ? remaining:GD_MAX_EXPRESSION_TOKENS};
    GD_ExpressionSite site={record,model->records[record].control.lines[2],GD_EXPRESSION_CONTROL};
    controls->first[record]=controls->token_count;
    valid=gd_compile_expression(model,&expression,model->records[record].control.expression,strlen(model->records[record].control.expression),site);
    controls->count[record]=expression.count; controls->token_count+=expression.count; return valid;
}
static int numeric_type(const GD_ControlDefinition *definition) { return definition->type && (!strcmp(definition->type,"number") || !strcmp(definition->type,"integer")); }
static int validate_value(GD_Model *model, int record, GD_ControlValue *value) {
    const GD_ControlDefinition *definition=&model->records[record].control;
    int i;
    if (numeric_type(definition)) {
        double minimum,maximum;
        GD_Unit declared;
        if (value->kind!=GD_LITERAL_NUMBER || !isfinite(value->number) ||
            (!strcmp(definition->type,"integer") && (fabs(value->number)>SAFE_INTEGER || floor(value->number)!=value->number))) goto type;
        if (!definition->unit || !gd_control_unit(definition->unit,strlen(definition->unit),&declared) || !gd_units_equal(&declared,&value->unit)) {
            control_error(model,record,3,"controls.unit","Numeric control requires explicit compatible units; use 1 for dimensionless and explicit scale conversions"); return 0;
        }
        if ((definition->minimum && !gd_control_number(definition->minimum,&minimum,0)) ||
            (definition->maximum && !gd_control_number(definition->maximum,&maximum,0)) ||
            (definition->minimum && definition->maximum && minimum>maximum) ||
            (definition->minimum && value->number<minimum) || (definition->maximum && value->number>maximum)) {
            control_error(model,record,5,"controls.range","Value violates declared bounds, or bounds are invalid"); return 0;
        }
        return 1;
    }
    if (definition->minimum || definition->maximum || (definition->unit && *definition->unit && strcmp(definition->unit,"1"))) goto type;
    if (definition->type && !strcmp(definition->type,"boolean") && value->kind==GD_LITERAL_BOOLEAN) return 1;
    if (definition->type && !strcmp(definition->type,"enum") && value->kind==GD_LITERAL_STRING) {
        const char *allowed=definition->allowed;
        for (i=0;i<definition->allowed_count;++i,allowed+=strlen(allowed)+1) if (!strcmp(value->string,allowed)) return 1;
        control_error(model,record,4,"controls.enum","Enum value must match an authored allowed value exactly"); return 0;
    }
type:
    control_error(model,record,1,"controls.type","Control requires its declared scalar type; integer values/results must be exact and JSON-safe, without implicit rounding"); return 0;
}
static int literal(GD_Model *model, int record, const char *override, GD_ControlValue *value) {
    const GD_ControlDefinition *definition=&model->records[record].control;
    const char *text=override ? override:definition->value;
    memset(value,0,sizeof(*value));
    if (!definition->type || !text) goto invalid;
    value->kind=override ? numeric_type(definition) ? GD_LITERAL_NUMBER : !strcmp(definition->type,"boolean") ? GD_LITERAL_BOOLEAN:GD_LITERAL_STRING : definition->kind;
    {
        GD_LiteralKind expected=numeric_type(definition) ? GD_LITERAL_NUMBER : !strcmp(definition->type,"boolean") ? GD_LITERAL_BOOLEAN : !strcmp(definition->type,"enum") ? GD_LITERAL_STRING:GD_LITERAL_NULL;
        if (!expected || value->kind!=expected) {
            control_error(model,record,1,"controls.type","Literal must match the declared control type; no numeric boolean or string coercion"); return 0;
        }
    }
    if (value->kind==GD_LITERAL_NUMBER) {
        if (!gd_control_number(text,&value->number,!strcmp(definition->type,"integer"))) goto invalid;
        if (!definition->unit || !gd_control_unit(definition->unit,strlen(definition->unit),&value->unit)) {
            control_error(model,record,3,"controls.unit","Invalid numeric unit; explicit 1 denotes dimensionless"); return 0;
        }
    } else if (value->kind==GD_LITERAL_BOOLEAN) {
        if (!strcmp(text,"true") || (!override && (!strcmp(text,"True") || !strcmp(text,"TRUE")))) value->number=1;
        else if (strcmp(text,"false") && (override || (strcmp(text,"False") && strcmp(text,"FALSE")))) goto invalid;
    } else if (value->kind==GD_LITERAL_STRING) value->string=text;
    else goto invalid;
    return validate_value(model,record,value);
invalid:
    control_error(model,record,1,"controls.value","Missing or invalid typed literal/override; no coercion, unsafe integers, nonfinite values, or underflow to zero"); return 0;
}
GD_ExpressionState gd_expression_value(const GD_ControlToken *tokens, int count, const GD_Controls *controls,
                                      const GD_ExpressionInput *starting, int integer, GD_ExpressionInput *result) {
    GD_ExpressionInput stack[GD_MAX_EXPRESSION_STACK];
    int i,depth=0;
    if (!result) return GD_EXPRESSION_ARITHMETIC_ERROR;
    memset(result,0,sizeof(*result));
    if (!tokens || count<1 || count>GD_MAX_EXPRESSION_TOKENS) return GD_EXPRESSION_ARITHMETIC_ERROR;
    for (i=0;i<count;++i) {
        const GD_ControlToken *token=&tokens[i];
        int operation=token->operation, arguments, known;
        GD_ExpressionInput *a,*b,*c;
        if (operation==OP_NUMBER || operation==OP_REFERENCE || operation==OP_THIS || operation==OP_UNKNOWN) {
            if (depth==GD_MAX_EXPRESSION_STACK) goto invalid;
            a=&stack[depth++]; memset(a,0,sizeof(*a));
            if (operation==OP_REFERENCE) {
                if (controls && token->target>=0 && token->target<GD_MAX_RECORDS && controls->state[token->target]==1) {
                    a->value=controls->values[token->target]; a->known=1; a->unit_known=a->value.kind==GD_LITERAL_NUMBER;
                }
            } else if (operation==OP_THIS || operation==OP_UNKNOWN) {
                if (starting) *a=*starting;
                if (operation==OP_UNKNOWN) { a->known=0; a->value.number=0; }
            } else {
                a->value.kind=GD_LITERAL_NUMBER; a->value.number=token->number; a->known=a->unit_known=1;
                if (!isfinite(token->number) || (integer && (token->rounded_fraction || fabs(token->number)>SAFE_INTEGER))) goto invalid;
                if (token->unit && !gd_control_unit(token->unit,(size_t)token->unit_length,&a->value.unit)) goto unit_error;
            }
            if (a->known && a->value.kind==GD_LITERAL_NUMBER && (!isfinite(a->value.number) || (integer && fabs(a->value.number)>SAFE_INTEGER))) goto invalid;
            continue;
        }
        if (operation<OP_ADD || operation>OP_ROUND) goto invalid;
        arguments=operation>=OP_MIN ? arity(operation) : operation==OP_NEGATE || operation==OP_POSITIVE ? 1:2;
        if (depth<arguments) goto invalid;
        a=&stack[depth-arguments]; b=arguments>=2 ? a+1:NULL; c=arguments==3 ? a+2:NULL;
        if ((a->value.kind && a->value.kind!=GD_LITERAL_NUMBER) || (b && b->value.kind && b->value.kind!=GD_LITERAL_NUMBER) ||
            (c && c->value.kind && c->value.kind!=GD_LITERAL_NUMBER)) goto invalid;
        known=a->known && (!b || b->known) && (!c || c->known);
        if (operation==OP_MULTIPLY || operation==OP_DIVIDE) {
            int j;
            if (operation==OP_DIVIDE && b->known && b->value.number==0) goto invalid;
            if (known) {
                double left=a->value.number;
                a->value.number=operation==OP_MULTIPLY ? left*b->value.number:left/b->value.number;
                if (left!=0 && b->value.number!=0 && a->value.number==0) goto invalid;
            }
            if (a->unit_known && b->unit_known) {
                for (j=0;j<b->value.unit.count;++j)
                    if (!unit_term(&a->value.unit,b->value.unit.terms[j].symbol,strlen(b->value.unit.terms[j].symbol),
                                   b->value.unit.terms[j].power*(operation==OP_DIVIDE ? -1:1))) goto unit_error;
            } else a->unit_known=0;
        } else {
            if ((b && a->unit_known && b->unit_known && !gd_units_equal(&a->value.unit,&b->value.unit)) ||
                (c && a->unit_known && c->unit_known && !gd_units_equal(&a->value.unit,&c->value.unit)) ||
                (c && b->unit_known && c->unit_known && !gd_units_equal(&b->value.unit,&c->value.unit))) goto unit_error;
            if (operation==OP_CLAMP && b->known && c->known && b->value.number>c->value.number) goto invalid;
            if (known) {
                if (operation==OP_ADD) a->value.number+=b->value.number;
                else if (operation==OP_SUBTRACT) a->value.number-=b->value.number;
                else if (operation==OP_NEGATE) a->value.number=-a->value.number;
                else if (operation==OP_MIN) { if (b->value.number<a->value.number) a->value.number=b->value.number; }
                else if (operation==OP_MAX) { if (b->value.number>a->value.number) a->value.number=b->value.number; }
                else if (operation==OP_CLAMP) {
                    if (a->value.number<b->value.number) a->value.number=b->value.number;
                    if (a->value.number>c->value.number) a->value.number=c->value.number;
                } else if (operation==OP_ABS) a->value.number=fabs(a->value.number);
                else if (operation==OP_FLOOR) a->value.number=floor(a->value.number);
                else if (operation==OP_CEIL) a->value.number=ceil(a->value.number);
                else if (operation==OP_ROUND && a->value.number!=0) {
                    double magnitude=fabs(a->value.number), rounded=floor(magnitude);
                    if (magnitude-rounded>=0.5) rounded+=1;
                    a->value.number=a->value.number<0 ? -rounded:rounded;
                }
            }
            if (b) a->unit_known=a->unit_known && b->unit_known && (!c || c->unit_known);
        }
        a->known=known; a->value.kind=GD_LITERAL_NUMBER;
        if (known && (!isfinite(a->value.number) || (integer && fabs(a->value.number)>SAFE_INTEGER))) goto invalid;
        depth-=arguments-1;
    }
    if (depth!=1) goto invalid;
    *result=stack[0]; return result->known ? GD_EXPRESSION_VALUE:GD_EXPRESSION_UNKNOWN;
unit_error:
    return GD_EXPRESSION_UNIT_ERROR;
invalid:
    return GD_EXPRESSION_ARITHMETIC_ERROR;
}
static int evaluate(GD_Model *model, GD_Controls *controls, int record) {
    GD_Expression expression={controls->tokens+controls->first[record],controls->count[record],controls->count[record]};
    GD_ExpressionInput value;
    GD_ExpressionState state=gd_expression_value(expression.tokens,expression.count,controls,NULL,!strcmp(model->records[record].control.type,"integer"),&value);
    if (state==GD_EXPRESSION_UNKNOWN) {
        control_error(model,record,2,"controls.dependency","A required control could not be resolved"); return 0;
    }
    if (state==GD_EXPRESSION_UNIT_ERROR) {
        control_error(model,record,2,"controls.unit","Formula unit mismatch or unit complexity limit; scales/symbols must match explicitly"); return 0;
    }
    if (state==GD_EXPRESSION_ARITHMETIC_ERROR) {
        control_error(model,record,2,"controls.arithmetic","Invalid/nonfinite arithmetic, underflow to zero, nonnumeric operand, zero division, inverted clamp bounds, or unsafe integer intermediate"); return 0;
    }
    if (!validate_value(model,record,&value.value)) return 0;
    controls->values[record]=value.value; return 1;
}
int gd_control_find_name(const GD_Model *model, const char *name) {
    int i, found=-1;
    if (!name) return -1;
    if (strchr(name,'/')) return gd_control_find(model,name);
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_CONTROL && !strcmp(model->records[i].name,name)) {
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
int gd_select_controls(GD_Model *model, const GD_ControlFilter *filter, GD_ControlSelection *selection) {
    int i,j,all=0,valid=1;
    unsigned char included[GD_MAX_RECORDS]={0},excluded[GD_MAX_RECORDS]={0};
    if (!filter || !selection || filter->status_count<0 || filter->status_count>GD_MAX_CONTROL_FILTERS ||
        filter->include_count<0 || filter->include_count>GD_MAX_CONTROL_FILTERS || filter->exclude_count<0 || filter->exclude_count>GD_MAX_CONTROL_FILTERS ||
        filter->root < -1 || filter->root>=model->record_count || (filter->root>=0 && model->records[filter->root].role!=GD_CONTROL)) {
        gd_error(model,"controls.selection","Invalid bounded control filter or root",NULL,0); return 0;
    }
    memset(selection,0,sizeof(*selection));
    for (i=0;i<filter->status_count;++i) {
        if (!filter->statuses[i] || !*filter->statuses[i]) valid=0;
        else if (!strcmp(filter->statuses[i],"all")) all=1;
    }
    if (!valid || (all && filter->status_count!=1)) {
        gd_error(model,"controls.selection","Use nonempty status names or --status all alone",NULL,0); return 0;
    }
    for (i=0;i<filter->include_count+filter->exclude_count;++i) {
        int include=i<filter->include_count;
        int record=gd_control_find_name(model,include ? filter->include[i]:filter->exclude[i-filter->include_count]);
        if (record<0) { gd_error(model,"controls.selection","Include/exclude names must identify unique loaded controls",NULL,0); valid=0; }
        else if (include) included[record]=1; else excluded[record]=1;
    }
    if (!valid) return 0;
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_CONTROL) {
        const char *status=model->records[i].status;
        int match=all || (!filter->status_count && (!filter->accepted_default || filter->include_count || filter->root>=0));
        if (!filter->status_count && !match) match=status && !strcmp(status,"accepted");
        for (j=0;j<filter->status_count && !match;++j) if (status && !strcmp(status,filter->statuses[j])) match=1;
        selection->allowed[i]=(unsigned char)match;
        selection->roots[i]=(unsigned char)(match && (!filter->include_count || included[i]) && !excluded[i] && (filter->root<0 || i==filter->root));
    }
    return 1;
}
GD_Controls *gd_resolve_control_set(GD_Model *model, const GD_ControlSelection *selection, const GD_ControlOverride *overrides, int count) {
    GD_Controls *controls;
    const char *replacement[GD_MAX_RECORDS]={0};
    int queue[GD_MAX_RECORDS], head[GD_MAX_RECORDS], remaining[GD_MAX_RECORDS]={0}, next[GD_MAX_CONTROL_TOKENS];
    int i,j,read=0,write=0;
    if (count<0 || count>GD_MAX_RECORDS || (count && !overrides)) {
        gd_error(model,"controls.arguments","Invalid control root or override collection",NULL,0); return NULL;
    }
    controls=model->allocator.allocate(model->allocator.context,sizeof(*controls));
    if (!controls) { gd_error(model,"input.memory","Cannot allocate control analysis",NULL,0); return NULL; }
    memset(controls,0,sizeof(*controls)); controls->allocator=model->allocator;
    for (i=0;i<count;++i) {
        int target=gd_control_find(model,overrides[i].reference);
        if (target<0 || !overrides[i].value || replacement[target]) { gd_error(model,"controls.override","Override requires a unique existing control and may not repeat a key",NULL,0); continue; }
        replacement[target]=overrides[i].value; controls->overridden[target]=1;
    }
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_CONTROL && (!selection || selection->roots[i])) {
        controls->selected[i]=controls->requested[i]=1; queue[write++]=i;
    }
    while (read<write) {
        int record=queue[read++];
        const GD_ControlDefinition *definition=&model->records[record].control;
        ++controls->selected_count;
        if (selection && !selection->allowed[record]) {
            gd_error(model,"controls.status","Required control is outside the selected status policy; include its status explicitly or revise the dependency",model->records[record].source_path,0);
            controls->state[record]=3; continue;
        }
        if (replacement[record] || !definition->expression || !*definition->expression) {
            controls->state[record]=literal(model,record,replacement[record],&controls->values[record]) ? 1:3;
        } else if (!numeric_type(definition) || definition->kind!=GD_LITERAL_NULL) {
            control_error(model,record,2,"controls.definition","Formula requires a numeric control and null literal value; no competing authorities"); controls->state[record]=3;
        } else {
            controls->state[record]=compile(model,controls,record) ? 2:3;
            if (controls->state[record]!=2) continue;
            for (j=controls->first[record];j<controls->first[record]+controls->count[record];++j) {
                GD_ControlToken *token=&controls->tokens[j];
                if (token->operation==OP_REFERENCE && !controls->selected[token->target]) { controls->selected[token->target]=1; queue[write++]=token->target; }
            }
        }
    }
    for (i=0;i<model->record_count;++i) head[i]=-1;
    for (i=0;i<model->record_count;++i) if (controls->state[i]==2) {
        for (j=controls->first[i];j<controls->first[i]+controls->count[i];++j) {
            GD_ControlToken *token=&controls->tokens[j];
            if (token->operation==OP_REFERENCE) { next[j]=head[token->target]; head[token->target]=j; ++remaining[i]; }
        }
    }
    read=write=0;
    for (i=0;i<model->record_count;++i) if (controls->selected[i] && !remaining[i]) queue[write++]=i;
    while (read<write) {
        int record=queue[read++];
        if (controls->state[record]==2) controls->state[record]=evaluate(model,controls,record) ? 1:3;
        if (controls->state[record]==1) ++controls->resolved_count;
        for (j=head[record];j>=0;j=next[j]) if (!--remaining[controls->tokens[j].source]) queue[write++]=controls->tokens[j].source;
    }
    for (i=0;i<model->record_count;++i) if (controls->state[i]==2) {
        controls->state[i]=3; control_error(model,i,2,"controls.cycle","Formula lies in a dependency cycle or is blocked by an upstream cycle");
    }
    controls->complete=!model->error_count && !model->errors_omitted && controls->selected_count==controls->resolved_count;
    return controls;
}
GD_Controls *gd_resolve_controls(GD_Model *model, int root, const GD_ControlOverride *overrides, int count) {
    GD_ControlSelection selection;
    int i;
    if (root < -1 || root>=model->record_count || (root>=0 && model->records[root].role!=GD_CONTROL)) {
        gd_error(model,"controls.arguments","Invalid control root",NULL,0); return NULL;
    }
    if (root<0) return gd_resolve_control_set(model,NULL,overrides,count);
    memset(&selection,0,sizeof(selection)); selection.roots[root]=1;
    for (i=0;i<model->record_count;++i) selection.allowed[i]=(unsigned char)(model->records[i].role==GD_CONTROL);
    return gd_resolve_control_set(model,&selection,overrides,count);
}
void gd_controls_destroy(GD_Controls *controls) { if (controls) controls->allocator.release(controls->allocator.context,controls); }
