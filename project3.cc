#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include "compiler.h"
#include "lexer.h"
#include <map>
#include <iostream>
#include <vector>
using namespace std;

LexicalAnalyzer lexer;
Token token;
map<string, int> location;

InstructionNode* head = nullptr;
InstructionNode* current = nullptr;
InstructionNode* endOfFor = nullptr;

struct InstructionNode* parse_statement_list();
void printLinkedList(InstructionNode* head);

void parse_id_list(){
    try {     
        token = lexer.GetToken();
        if(token.token_type == ID){
            location[token.lexeme] = next_available;
            mem[next_available] = 0;
            next_available++;
        }
        if (lexer.peek(1).token_type == COMMA){
            //parse COMMA
            lexer.GetToken();
            parse_id_list();            
        } else {
            return;
        }

    } catch (exception& e) {
        cout << "Syntax error in parse_id_list!" << endl;
    }
}

void parse_num_list(){
    try {        
        //get the NUM token 
        token = lexer.GetToken();
        
        if(token.token_type = NUM){
            inputs.push_back(stoi(token.lexeme));
        }
        if(lexer.peek(1).token_type == NUM){
            parse_num_list();
        } else {
            return;
        }
    } catch (exception& e) {
        cout << "Syntax error in parse_num_list! " << e.what() << endl;
    }
}

struct InstructionNode* parse_assign_stmt(){
    try{
        struct InstructionNode * iNode = new InstructionNode;
        iNode->next = nullptr;
        iNode->type = ASSIGN;
        iNode->assign_inst.left_hand_side_index = location[token.lexeme];
        //parse EQUAL
        token = lexer.GetToken();
        //parse primary or expr (first operand)
        token = lexer.GetToken();
        
        // if this is a constant, then allocate memory for it
        // a constant is not in variable list at the beginning
        // it would not be in the location map
        if (location.find(token.lexeme) == location.end()) {
            location[token.lexeme] = next_available;
            mem[next_available] = stoi(token.lexeme);
            next_available++;
        }
        
        // assign the first operand
        iNode->assign_inst.operand1_index = location[token.lexeme];

        if (lexer.peek(1).token_type == SEMICOLON){        
            iNode->assign_inst.op = OPERATOR_NONE;
        } else {   
            token = lexer.GetToken(); //parse the operator
            switch(token.token_type){
                case PLUS:
                    iNode->assign_inst.op = OPERATOR_PLUS;
                    break;
                case MINUS:
                    iNode->assign_inst.op = OPERATOR_MINUS;
                    break;
                case MULT:
                    iNode->assign_inst.op = OPERATOR_MULT;
                    break;
                case DIV:
                    iNode->assign_inst.op = OPERATOR_DIV;
                    break;
            }
            token = lexer.GetToken(); //parse second operand
            // if this is a constant, then allocate memory for it
            // a constant is not in variable list at the beginning
            // it would not be in the location map
            if (location.find(token.lexeme) == location.end()) {
                location[token.lexeme] = next_available;
                mem[next_available] = stoi(token.lexeme);
                next_available++;
            }
            iNode->assign_inst.operand2_index = location[token.lexeme]; 
        }

        current->next = iNode;      
        current = iNode;
        lexer.GetToken(); //parse the semicolon
        
        return current;

    } catch (exception e) {
        // cout << e.what() << endl;    
    }
}

struct InstructionNode* parse_input_stmt(){
    InstructionNode* iNode = new InstructionNode;
    iNode->type = IN;
    iNode->next = nullptr;
    //get variable's name
    token = lexer.GetToken();
    iNode->input_inst.var_index = location[token.lexeme];
    current->next = iNode;      
    current = iNode;
    //parse semicolon
    token = lexer.GetToken();
    //get the next beginning of statements
    return current;
}

struct InstructionNode* parse_output_stmt(){
    InstructionNode* iNode = new InstructionNode;
    iNode->type = OUT;
    iNode->next = nullptr;
    //get the variable to print
    token = lexer.GetToken();

    iNode->output_inst.var_index = location[token.lexeme];
    current->next = iNode;      
    current = iNode;
    //parse the semicolon
    token = lexer.GetToken();
    //get the next beginning of statements
    return current;
}

struct InstructionNode* parse_if_stmt(){
    InstructionNode* iNode = new InstructionNode;
    iNode->type = CJMP;
    iNode->next = nullptr;

    //get the first operand
    token = lexer.GetToken();
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    //assign first operand
    iNode->cjmp_inst.operand1_index = location[token.lexeme];

    //get the relop
    token = lexer.GetToken();
    switch(token.token_type){
        case GREATER:
            iNode->cjmp_inst.condition_op = CONDITION_GREATER;
            break;
        case LESS:
            iNode->cjmp_inst.condition_op = CONDITION_LESS;
            break;
        case NOTEQUAL:
            iNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            break;
    }

    //get the second operand
    token = lexer.GetToken();
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    //assign second operand
    iNode->cjmp_inst.operand2_index = location[token.lexeme];

    current->next = iNode;      
    current = iNode;
    //parse the LBRACE
    token = lexer.GetToken();    
    //parse_body
    parse_statement_list();
    
    //current node is the end of the if's body 
        
    //Declare a do nothing node
    InstructionNode* noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;

    current->next = noopNode;
    current = noopNode;
    iNode->cjmp_inst.target = noopNode;
    
    //parse the RBRACE
    token = lexer.GetToken();  
    // printLinkedList(head);
    
    return iNode;
};

struct InstructionNode* parse_while_stmt(){
    InstructionNode* iNode = new InstructionNode;
    iNode->type = CJMP;
    iNode->next = nullptr;

    //get the first operand
    token = lexer.GetToken();
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    //assign first operand
    iNode->cjmp_inst.operand1_index = location[token.lexeme];

    //get the relop
    token = lexer.GetToken();
    switch(token.token_type){
        case GREATER:
            iNode->cjmp_inst.condition_op = CONDITION_GREATER;
            break;
        case LESS:
            iNode->cjmp_inst.condition_op = CONDITION_LESS;
            break;
        case NOTEQUAL:
            iNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            break;
    }

    //get the second operand
    token = lexer.GetToken();
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    //assign second operand
    iNode->cjmp_inst.operand2_index = location[token.lexeme];

    current->next = iNode;      
    current = iNode;
    //parse the LBRACE
    token = lexer.GetToken();    
    //parse_body
    parse_statement_list();
    //current node is the end of the if's body 
    
    //Declare a do nothing node
    InstructionNode* noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;

    //jmp to NOOP node when the while loop breaks
    iNode->cjmp_inst.target = noopNode;
    
    InstructionNode* jmpNode = new InstructionNode;
    jmpNode->next = noopNode;
    jmpNode->type = JMP;
    jmpNode->jmp_inst.target = iNode;        

    //the node next to the body is the jmpNode
    current->next = jmpNode;
    current = noopNode;
    
    //parse the RBRACE
    token = lexer.GetToken();  
    // printLinkedList(head);
    
    return iNode;
}

struct InstructionNode* parse_for_stmt(){    
    //parse LPAREN
    lexer.GetToken();

    //parse the LHS token
    token = lexer.GetToken();

    //parse the first assignment statement
    struct InstructionNode * assignNode1 = new InstructionNode;
    assignNode1->next = nullptr;
    assignNode1->type = ASSIGN;
    assignNode1->assign_inst.left_hand_side_index = location[token.lexeme];
    //parse EQUAL
    token = lexer.GetToken();
    //parse primary or expr (first operand)
    token = lexer.GetToken();
    
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    
    // assign the first operand
    assignNode1->assign_inst.operand1_index = location[token.lexeme];

    if (lexer.peek(1).token_type == SEMICOLON){        
        assignNode1->assign_inst.op = OPERATOR_NONE;
    } else {   
        token = lexer.GetToken(); //parse the operator
        switch(token.token_type){
            case PLUS:
                assignNode1->assign_inst.op = OPERATOR_PLUS;
                break;
            case MINUS:
                assignNode1->assign_inst.op = OPERATOR_MINUS;
                break;
            case MULT:
                assignNode1->assign_inst.op = OPERATOR_MULT;
                break;
            case DIV:
                assignNode1->assign_inst.op = OPERATOR_DIV;
                break;
        }
        token = lexer.GetToken(); //parse second operand
        // if this is a constant, then allocate memory for it
        // a constant is not in variable list at the beginning
        // it would not be in the location map
        if (location.find(token.lexeme) == location.end()) {
            location[token.lexeme] = next_available;
            mem[next_available] = stoi(token.lexeme);
            next_available++;
        }
        assignNode1->assign_inst.operand2_index = location[token.lexeme]; 
    }
    lexer.GetToken(); //parse the semicolon
    //end of assignNode1

    //add assignNode1 to the list of nodes
    current->next = assignNode1;      
    current = assignNode1;

    //start of whileNode
    InstructionNode* whileNode = new InstructionNode;
    whileNode->type = CJMP;
    whileNode->next = nullptr;

    //get the first operand
    token = lexer.GetToken();
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    //assign first operand
    whileNode->cjmp_inst.operand1_index = location[token.lexeme];

    //get the relop
    token = lexer.GetToken();
    switch(token.token_type){
        case GREATER:
            whileNode->cjmp_inst.condition_op = CONDITION_GREATER;
            break;
        case LESS:
            whileNode->cjmp_inst.condition_op = CONDITION_LESS;
            break;
        case NOTEQUAL:
            whileNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            break;
    }

    //get the second operand
    token = lexer.GetToken();
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    //assign second operand
    whileNode->cjmp_inst.operand2_index = location[token.lexeme];
    //end of parsing loop condition

    //add this CJMP node to the list of nodes
    current->next = whileNode;      
    current = whileNode;

    //parse SEMICOLON
    lexer.GetToken();
    
    //start of assignNode2
    
    //parse LHS token
    token = lexer.GetToken();

    struct InstructionNode * assignNode2 = new InstructionNode;
    assignNode2->next = nullptr;
    assignNode2->type = ASSIGN;
    assignNode2->assign_inst.left_hand_side_index = location[token.lexeme];
    //parse EQUAL
    token = lexer.GetToken();
    //parse primary or expr (first operand)
    token = lexer.GetToken();
    
    // if this is a constant, then allocate memory for it
    // a constant is not in variable list at the beginning
    // it would not be in the location map
    if (location.find(token.lexeme) == location.end()) {
        location[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        next_available++;
    }
    
    // assign the first operand
    assignNode2->assign_inst.operand1_index = location[token.lexeme];

    if (lexer.peek(1).token_type == SEMICOLON){        
        assignNode2->assign_inst.op = OPERATOR_NONE;
    } else {   
        token = lexer.GetToken(); //parse the operator
        switch(token.token_type){
            case PLUS:
                assignNode2->assign_inst.op = OPERATOR_PLUS;
                break;
            case MINUS:
                assignNode2->assign_inst.op = OPERATOR_MINUS;
                break;
            case MULT:
                assignNode2->assign_inst.op = OPERATOR_MULT;
                break;
            case DIV:
                assignNode2->assign_inst.op = OPERATOR_DIV;
                break;
        }
        token = lexer.GetToken(); //parse second operand
        // if this is a constant, then allocate memory for it
        // a constant is not in variable list at the beginning
        // it would not be in the location map
        if (location.find(token.lexeme) == location.end()) {
            location[token.lexeme] = next_available;
            mem[next_available] = stoi(token.lexeme);
            next_available++;
        }
        assignNode2->assign_inst.operand2_index = location[token.lexeme]; 
    }
    lexer.GetToken(); //parse the semicolon
    //end of assignNode2

    //parse the body
    //parse the RPAREN
    lexer.GetToken();
    //parse the LBRACE
    lexer.GetToken();

    parse_statement_list();
    //current is pointing to the end of the body
    current->next = assignNode2;
    current = assignNode2;

    //Declare a do nothing node
    InstructionNode* noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;

    //jmp to NOOP node when the while loop breaks
    whileNode->cjmp_inst.target = noopNode;
    
    InstructionNode* jmpNode = new InstructionNode;
    jmpNode->next = noopNode;
    jmpNode->type = JMP;
    jmpNode->jmp_inst.target = whileNode;        

    //the node next to the body is the jmpNode
    current->next = jmpNode;
    current = noopNode;

    endOfFor = current;

    //parse the RBRACE
    token = lexer.GetToken();  
    // printLinkedList(head);
    // cout << endl;
    
    return assignNode1;
}

struct InstructionNode* parse_stmt(){
    //Get the beginning lexeme of statements
    token = lexer.GetToken();
    //This while loop stops when token.type is none of the statements's beginning
    //while(token.token_type == ID || token.token_type == INPUT || token.token_type == OUTPUT){
    if(token.token_type == FOR || token.token_type == WHILE || token.token_type == IF || token.token_type == ID || token.token_type == INPUT || token.token_type == OUTPUT){
        switch (token.token_type)
        {
        case FOR: 
            return parse_for_stmt();
            break;    
        case WHILE: 
            return parse_while_stmt();
            break;
        case IF:
            return parse_if_stmt();
            break;    
        case ID:
            return parse_assign_stmt();
            break;
        case INPUT:
            return parse_input_stmt();
            break;
        case OUTPUT:
            return parse_output_stmt();            
        default:
            break;
        }            
    }
}

struct InstructionNode* parse_statement_list(){
    try{
        struct InstructionNode* instl1;
        struct InstructionNode* instl2;

        Token nextToken0 = lexer.peek(1);

        instl1 = parse_stmt();
        Token nextToken = lexer.peek(1);
        if(nextToken.token_type == FOR || nextToken.token_type == WHILE || nextToken.token_type == IF ||nextToken.token_type == ID || nextToken.token_type == INPUT || nextToken.token_type == OUTPUT){
            instl2 = parse_statement_list();
            
            if(nextToken0.token_type == FOR){
                endOfFor->next = instl2;
                endOfFor = nullptr;
            }

            if(instl1->type == CJMP){
                //append instl2 to the last node of instl1
                InstructionNode* lastNode = instl1;

                //Get the last NOOP node of the if's body to return
                int ifCtr = 0;
                while(lastNode != nullptr && (lastNode->type != NOOP || ifCtr != 1)){
                    if(lastNode->type == CJMP){
                        ifCtr++;
                    }
                    if(lastNode->type == NOOP){
                        ifCtr--;
                    }
                    
                    lastNode = lastNode->next;
                }
                lastNode->next = instl2;
                return instl1;
            }
                        
            instl1->next = instl2;
        } 
        return instl1;            
         
    } catch (exception& e) {
        cout << "Syntax error." << e.what() << endl;
    }
}

// Function to print the linked list
void printLinkedList(InstructionNode* head) {
    InstructionNode* current = nullptr;
    current = head;

    while (current != nullptr) {
        // Print the contents of the current node based on its type
        switch (current->type) {
            case ASSIGN:
                std::cout << "Assign: lhs=" << current->assign_inst.left_hand_side_index
                          << ", op1=" << current->assign_inst.operand1_index
                          << ", op2=" << current->assign_inst.operand2_index
                          << ", op=" << current->assign_inst.op << std::endl;
                break;
            case IN:
                std::cout << "Input: var=" << current->input_inst.var_index << std::endl;
                break;
            case OUT:
                std::cout << "Output: var=" << current->output_inst.var_index << std::endl;
                break;
            case CJMP:
                std::cout << "CJump: op=" << current->cjmp_inst.condition_op
                          << ", op1=" << current->cjmp_inst.operand1_index
                          << ", op2=" << current->cjmp_inst.operand2_index
                          << ", target=" << current->cjmp_inst.target << std::endl;
                break;
            case JMP:
                std::cout << "Jump: target=" << current->jmp_inst.target << std::endl;
                break;
            case NOOP:
                std::cout << "NOOP" << std::endl;
                break;
        }

        // Move to the next node
        current = current->next;
    }
}

struct InstructionNode * parse_generate_intermediate_representation()
{
    parse_id_list();

    head = new InstructionNode;
    head->type = NOOP;
    head->next = nullptr;

    current = head;
    
    //parse SEMICOLON
    token = lexer.GetToken();
    //parse LBRACE
    token = lexer.GetToken();
    
    parse_statement_list();

    //end of list is nullptr
    current->next = nullptr;

    //parse RBRACE
    token = lexer.GetToken();
    // cout << "RBRACE" << token.lexeme << endl;
    // cout << "RBRACE" << token.token_type << endl;

    //Inputs
    parse_num_list();
    // printLinkedList(head);
    
    return head;
}