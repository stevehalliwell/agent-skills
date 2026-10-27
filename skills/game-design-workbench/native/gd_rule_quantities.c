#include "gd.h"
#include <math.h>
#include <string.h>

/* Numeric assumptions use the declared axis unit, like typed control overrides.
   Never read a starting quantity from an initial/configuration control. */
static int fact_number(const char *text, double *number, int integer) {
    size_t length=0;
    if (!text) return 0;
    while (length<128 && text[length]) ++length;
    return length<128 && gd_control_number(text,number,integer);
}
GD_ExpressionInput gd_rule_starting_quantity(const GD_Rules *rules, int participant, int axis,
                                            const GD_RulePredicateFact *facts, int count) {
    GD_ExpressionInput result;
    int i, found=0, conflict=0, integer;
    memset(&result,0,sizeof(result));
    if (!rules || !rules->model || participant<0 || participant>=rules->participant_count || axis<0 || axis>=rules->model->record_count ||
        count<0 || count>GD_MAX_RULE_CONDITION_ITEMS || (count && !facts) || !rules->axes[axis].valid ||
        (rules->axes[axis].type!=GD_AXIS_INTEGER && rules->axes[axis].type!=GD_AXIS_NUMBER)) return result;
    result.value.kind=GD_LITERAL_NUMBER;
    integer=rules->axes[axis].type==GD_AXIS_INTEGER;
    if (rules->axes[axis].unit_known)
        result.unit_known=gd_control_unit(rules->model->records[axis].axis.unit,strlen(rules->model->records[axis].axis.unit),&result.value.unit);
    for (i=0;i<count;++i) if (facts[i].participant==participant && facts[i].axis_record==axis && facts[i].truth==GD_RULE_TRUTH_TRUE) {
        double number;
        if (!fact_number(facts[i].value,&number,integer)) { conflict=1; continue; }
        if (found && number!=result.value.number) conflict=1;
        result.value.number=number; found=1;
    }
    for (i=0;i<count && found;++i) if (facts[i].participant==participant && facts[i].axis_record==axis && facts[i].truth!=GD_RULE_TRUTH_TRUE) {
        double number;
        if (!fact_number(facts[i].value,&number,integer) || number==result.value.number) conflict=1;
    }
    result.known=found && !conflict;
    return result;
}
GD_RuleBounds gd_rule_quantity_bounds(const GD_Rules *rules, int axis, double number) {
    const GD_ValueDefinition *definition; double bound; int integer;
    if (!rules || !rules->model || axis<0 || axis>=rules->model->record_count || !rules->axes[axis].valid || !isfinite(number) ||
        (rules->axes[axis].type!=GD_AXIS_INTEGER && rules->axes[axis].type!=GD_AXIS_NUMBER)) return GD_BOUNDS_UNKNOWN;
    definition=&rules->model->records[axis].axis; integer=rules->axes[axis].type==GD_AXIS_INTEGER;
    if ((definition->minimum && gd_control_number(definition->minimum,&bound,integer) && number<bound) ||
        (definition->maximum && gd_control_number(definition->maximum,&bound,integer) && number>bound)) return GD_BOUNDS_OUTSIDE;
    return definition->minimum || definition->maximum ? GD_BOUNDS_WITHIN:GD_BOUNDS_UNDECLARED;
}
GD_RuleQuantityValue gd_rule_quantity_value(const GD_Rules *rules, int index,
                                           const GD_RulePredicateFact *facts, int count) {
    GD_RuleQuantityValue result={GD_EXPRESSION_UNKNOWN,0,0,0,GD_BOUNDS_UNKNOWN};
    const GD_RuleQuantity *quantity;
    GD_ExpressionInput starting,value;
    int integer=0;
    if (!rules || !rules->model || index<0 || index>=rules->quantity_count || count<0 || count>GD_MAX_RULE_CONDITION_ITEMS || (count && !facts)) return result;
    quantity=&rules->quantities[index];
    if (quantity->state!=GD_CONDITION_PARSED || quantity->first_token<0 || quantity->token_count<1 || quantity->token_count>GD_MAX_EXPRESSION_TOKENS ||
        quantity->first_token>rules->quantity_token_count-quantity->token_count) return result;
    starting=gd_rule_starting_quantity(rules,quantity->participant,quantity->axis_record,facts,count);
    if (quantity->axis_record>=0) integer=quantity->mode==GD_EXPRESSION_ASSIGNMENT && rules->axes[quantity->axis_record].type==GD_AXIS_INTEGER;
    result.state=gd_expression_value(rules->quantity_tokens+quantity->first_token,quantity->token_count,rules->controls,&starting,integer,&value);
    result.number=value.value.number; result.known=value.known;
    result.checks_complete=starting.unit_known && value.unit_known;
    if (result.state==GD_EXPRESSION_UNIT_ERROR || result.state==GD_EXPRESSION_ARITHMETIC_ERROR) return result;
    if (value.value.kind && value.value.kind!=GD_LITERAL_NUMBER) { result.state=GD_EXPRESSION_ARITHMETIC_ERROR; result.known=0; return result; }
    if (starting.unit_known && value.unit_known && !gd_units_equal(&starting.value.unit,&value.value.unit)) result.state=GD_EXPRESSION_UNIT_ERROR;
    if (result.known && (!isfinite(result.number) || (integer && (fabs(result.number)>9007199254740991.0 || floor(result.number)!=result.number))))
        result.state=GD_EXPRESSION_ARITHMETIC_ERROR;
    if (result.known && quantity->mode==GD_EXPRESSION_ASSIGNMENT) result.bounds=gd_rule_quantity_bounds(rules,quantity->axis_record,result.number);
    return result;
}
GD_RuleQuantityValue gd_rule_quantity_delta(const GD_Rules *rules, int index, int *recognized) {
    GD_RuleQuantityValue result={GD_EXPRESSION_UNKNOWN,0,0,0,GD_BOUNDS_UNKNOWN};
    GD_ExpressionInput starting,value;
    const GD_RuleQuantity *quantity;
    int integer;
    if (!recognized) return result;
    *recognized=0;
    if (!rules || !rules->model || index<0 || index>=rules->quantity_count) return result;
    quantity=&rules->quantities[index];
    if (quantity->mode!=GD_EXPRESSION_ASSIGNMENT || quantity->state!=GD_CONDITION_PARSED || quantity->axis_record<0 ||
        quantity->axis_record>=rules->model->record_count || quantity->first_token<0 || quantity->token_count<1 ||
        quantity->token_count>GD_MAX_EXPRESSION_TOKENS || quantity->first_token>rules->quantity_token_count-quantity->token_count) return result;
    starting=gd_rule_starting_quantity(rules,quantity->participant,quantity->axis_record,NULL,0);
    if (starting.value.kind!=GD_LITERAL_NUMBER) return result;
    integer=rules->axes[quantity->axis_record].type==GD_AXIS_INTEGER;
    result.state=gd_expression_delta(rules->quantity_tokens+quantity->first_token,quantity->token_count,rules->controls,&starting,integer,recognized,&value);
    if (!*recognized || result.state==GD_EXPRESSION_UNIT_ERROR || result.state==GD_EXPRESSION_ARITHMETIC_ERROR) return result;
    result.checks_complete=quantity->bindings_complete && starting.unit_known && value.unit_known;
    if (starting.unit_known && value.unit_known && !gd_units_equal(&starting.value.unit,&value.value.unit)) {
        result.state=GD_EXPRESSION_UNIT_ERROR; result.checks_complete=0; return result;
    }
    result.number=value.value.number; result.known=value.known && result.checks_complete;
    if (!result.known) result.state=GD_EXPRESSION_UNKNOWN;
    return result;
}
static void quantity_finding(GD_Rules *rules, const GD_RuleQuantity *quantity, const char *code, const char *message) {
    GD_Finding *finding;
    if (rules->finding_count==GD_MAX_ERRORS) { ++rules->findings_omitted; return; }
    finding=&rules->findings[rules->finding_count++];
    finding->record=quantity->record; finding->line=quantity->line; finding->field="quantity";
    finding->code=code; finding->message=message;
}
static int quantity_charge(GD_Model *model, GD_Rules *rules, size_t cost, int record, int line) {
    if (rules->quantity_limited) return 0;
    if (cost>(size_t)(GD_RULE_QUANTITY_WORK_LIMIT-rules->quantity_work)) {
        rules->quantity_limited=1; rules->quantity_bindings_complete=0;
        gd_error(model,"rules.quantity_work_limit","Quantity work budget exceeded; remaining programs/checks unavailable",model->records[record].source_path,line);
        return 0;
    }
    rules->quantity_work+=(int)cost; return 1;
}
static int compile_quantity(GD_Model *model, GD_Rules *rules, int participant, int axis, int record, int line,
                            const char *text, size_t length, GD_ExpressionMode mode) {
    GD_RuleQuantity *quantity;
    GD_Expression expression;
    GD_ExpressionSite site={record,line,mode};
    int index=rules->quantity_count, remaining=GD_MAX_RULE_QUANTITY_TOKENS-rules->quantity_token_count, i, errors=model->error_count;
    if (index==GD_MAX_RULE_QUANTITIES) {
        if (!rules->quantity_limited) gd_error(model,"rules.quantity_capacity","16384 quantity-program limit exceeded",model->records[record].source_path,line);
        rules->quantity_limited=1; rules->quantity_bindings_complete=0; return -1;
    }
    quantity=&rules->quantities[rules->quantity_count++]; memset(quantity,0,sizeof(*quantity));
    quantity->text=text; quantity->length=length; quantity->participant=participant; quantity->axis_record=axis;
    quantity->record=record; quantity->line=line; quantity->mode=mode; quantity->first_token=rules->quantity_token_count;
    if (!quantity_charge(model,rules,(length+1)*32,record,line)) { quantity->state=GD_CONDITION_LIMITED; return index; }
    expression.tokens=rules->quantity_tokens+rules->quantity_token_count; expression.count=0;
    expression.capacity=remaining<GD_MAX_EXPRESSION_TOKENS ? remaining:GD_MAX_EXPRESSION_TOKENS;
    if (!gd_compile_expression(model,&expression,text,length,site)) {
        quantity->state=remaining<GD_MAX_EXPRESSION_TOKENS && model->error_count>errors &&
            !strcmp(model->errors[model->error_count-1].code,"rules.quantity_limit") ? GD_CONDITION_LIMITED:GD_CONDITION_INVALID;
        rules->quantity_bindings_complete=0;
        if (quantity->state==GD_CONDITION_LIMITED) rules->quantity_limited=1;
        return index;
    }
    quantity->state=GD_CONDITION_PARSED; quantity->token_count=expression.count;
    rules->quantity_token_count+=expression.count;
    quantity->bindings_complete=axis>=0 && rules->axes[axis].valid && rules->axes[axis].unit_known;
    for (i=quantity->first_token;i<rules->quantity_token_count;++i) {
        const GD_ControlToken *token=&rules->quantity_tokens[i];
        if (token->target==GD_EXPRESSION_UNLOADED_REFERENCE) quantity->bindings_complete=0;
    }
    return index;
}
void gd_resolve_rule_quantities(GD_Model *model, GD_Rules *rules) {
    GD_ControlSelection selection;
    int c,p,i,k,roots=0;
    memset(&selection,0,sizeof(selection)); rules->quantity_bindings_complete=1;
    for (c=0;c<rules->case_count;++c) {
        const GD_RuleCase *item=&rules->cases[c];
        int record=rules->sources[item->source].record;
        for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
            GD_RuleParticipant *participant=&rules->participants[p];
            for (i=participant->first_assignment;i<participant->first_assignment+participant->assignment_count;++i) {
                GD_RuleAssignment *assignment=&rules->assignments[i];
                assignment->quantity=compile_quantity(model,rules,p,assignment->target.resolution==GD_RULE_RESOLVED ? assignment->target.axis_record:-1,
                    record,assignment->line,assignment->expression,strlen(assignment->expression),GD_EXPRESSION_ASSIGNMENT);
            }
            for (k=0;k<2;++k) {
                GD_RuleCondition *condition=k ? &participant->skip_condition:&participant->required_condition;
                int before=rules->quantity_count, tokens_before=rules->quantity_token_count;
                GD_RuleConditionState failed=GD_CONDITION_UNSPECIFIED;
                for (i=condition->first_token;i<condition->first_token+condition->token_count;++i) {
                    GD_RuleConditionToken *token=&rules->condition_tokens[i];
                    if (token->operation!=GD_RULE_OP_COMPARE) continue;
                    token->quantity=compile_quantity(model,rules,p,token->target.resolution==GD_RULE_RESOLVED ? token->target.axis_record:-1,
                        record,participant->lines[k ? 2:1],token->text+token->rhs_offset,token->length-(size_t)token->rhs_offset,GD_EXPRESSION_PREDICATE);
                    if (token->quantity<0 || !rules->quantities[token->quantity].bindings_complete) condition->bindings_complete=0;
                    if (token->quantity<0 || rules->quantities[token->quantity].state==GD_CONDITION_LIMITED) failed=GD_CONDITION_LIMITED;
                    else if (rules->quantities[token->quantity].state==GD_CONDITION_INVALID && failed!=GD_CONDITION_LIMITED) failed=GD_CONDITION_INVALID;
                }
                if (failed!=GD_CONDITION_UNSPECIFIED) {
                    /* A malformed/limited numeric atom invalidates its complete
                       Boolean field, not just that operand. Keep raw text/errors. */
                    condition->state=failed; condition->token_count=0; condition->bindings_complete=0;
                    rules->quantity_count=before; rules->quantity_token_count=tokens_before;
                }
                if (!condition->bindings_complete && condition->state!=GD_CONDITION_UNSPECIFIED) rules->condition_bindings_complete=0;
            }
        }
    }
    for (i=0;i<model->record_count;++i) selection.allowed[i]=model->records[i].role==GD_CONTROL;
    for (i=0;i<rules->quantity_token_count;++i) if (rules->quantity_tokens[i].target>=0) {
        selection.roots[rules->quantity_tokens[i].target]=1; roots=1;
    }
    for (i=0;i<rules->state_effect_count;++i) if (rules->state_effects[i].control_record>=0) {
        selection.roots[rules->state_effects[i].control_record]=1; roots=1;
    }
    for (p=0;p<rules->participant_count;++p) for (k=0;k<2;++k) {
        const GD_RuleCondition *condition=k ? &rules->participants[p].skip_condition:&rules->participants[p].required_condition;
        for (i=condition->first_token;i<condition->first_token+condition->token_count;++i) {
            const GD_RuleConditionToken *token=&rules->condition_tokens[i];
            if (token->operation==GD_RULE_OP_CONTROL && token->control_record>=0) { selection.roots[token->control_record]=1; roots=1; }
        }
    }
    if (roots) rules->controls=gd_resolve_control_set(model,&selection,NULL,0);
    for (i=0;i<rules->quantity_count;++i) {
        GD_RuleQuantity *quantity=&rules->quantities[i];
        GD_RuleQuantityValue *value=&quantity->inspection;
        int token;
        for (token=quantity->first_token;token<quantity->first_token+quantity->token_count;++token) {
            int dependency=rules->quantity_tokens[token].target;
            if (dependency>=0 && (!rules->controls || rules->controls->state[dependency]!=1 || rules->controls->values[dependency].kind!=GD_LITERAL_NUMBER)) quantity->bindings_complete=0;
        }
        if (quantity->state!=GD_CONDITION_PARSED || !quantity_charge(model,rules,(size_t)(quantity->token_count+1)*GD_RULE_NUMERIC_TOKEN_WORK,quantity->record,quantity->line)) {
            rules->quantity_bindings_complete=0; continue;
        }
        *value=gd_rule_quantity_value(rules,i,NULL,0);
        if (!quantity->bindings_complete || !value->checks_complete) rules->quantity_bindings_complete=0;
        if (value->state==GD_EXPRESSION_UNIT_ERROR || value->state==GD_EXPRESSION_ARITHMETIC_ERROR) {
            quantity->bindings_complete=0; rules->quantity_bindings_complete=0;
            gd_error(model,value->state==GD_EXPRESSION_UNIT_ERROR ? "rules.quantity_unit":"rules.quantity_arithmetic",
                value->state==GD_EXPRESSION_UNIT_ERROR ? "Quantity expression must match the target axis unit; no implicit conversion":
                "Quantity expression violates numeric type/safe arithmetic constraints",model->records[quantity->record].source_path,quantity->line);
        } else if (!quantity->bindings_complete || !value->checks_complete)
            quantity_finding(rules,quantity,"rules.quantity_undetermined","Target/unit/control input remains unknown; no starting value inferred");
        if (value->bounds==GD_BOUNDS_OUTSIDE)
            quantity_finding(rules,quantity,"rules.quantity_bounds","Proposed result is outside authored bounds; retained without clamp, wrap or gameplay rejection");
    }
    /* Remove slots belonging to whole rolled-back Boolean fields. Published
       programs remain contiguous; compact only before returning the snapshot. */
    {
        int write=0;
        for (p=0;p<rules->participant_count;++p) for (k=0;k<2;++k) {
            GD_RuleCondition *condition=k ? &rules->participants[p].skip_condition:&rules->participants[p].required_condition;
            for (i=condition->first_token;i<condition->first_token+condition->token_count;++i) {
                const GD_RuleConditionToken *token=&rules->condition_tokens[i];
                if (token->operation==GD_RULE_OP_COMPARE && (token->quantity<0 || !rules->quantities[token->quantity].bindings_complete)) {
                    condition->bindings_complete=0; rules->condition_bindings_complete=0;
                }
            }
            if (condition->token_count && condition->first_token!=write)
                memmove(rules->condition_tokens+write,rules->condition_tokens+condition->first_token,(size_t)condition->token_count*sizeof(*rules->condition_tokens));
            condition->first_token=write; write+=condition->token_count;
        }
        rules->condition_token_count=write;
    }
}
