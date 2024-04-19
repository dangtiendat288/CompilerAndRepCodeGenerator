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
        cout << e.what() << endl;    
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
    
    //Get the last node to return
    // while(current->next != nullptr){
    //     current = current->next;
    // }

    return iNode;
};

struct InstructionNode* parse_stmt(){
    //Get the beginning lexeme of statements
    token = lexer.GetToken();
    //This while loop stops when token.type is none of the statements's beginning
    //while(token.token_type == ID || token.token_type == INPUT || token.token_type == OUTPUT){
    if(token.token_type == IF || token.token_type == ID || token.token_type == INPUT || token.token_type == OUTPUT){
        switch (token.token_type)
        {
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

        instl1 = parse_stmt();
        Token nextToken = lexer.peek(1);
        if(nextToken.token_type == IF ||nextToken.token_type == ID || nextToken.token_type == INPUT || nextToken.token_type == OUTPUT){
            instl2 = parse_statement_list();
            
            if(instl1->type == CJMP){
                //append instl2 to the last node of instl1
                InstructionNode* lastNode = instl1;
                while(lastNode->type != 1000){  //1000 is the NOOP type
                    lastNode = lastNode->next;
                }
                lastNode->next = instl2;
                return instl1;
            }
                        
            instl1->next = instl2;
        } 
        return instl1;            
         
    } catch (exception& e) {
        cout << "Syntax error." << endl;
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