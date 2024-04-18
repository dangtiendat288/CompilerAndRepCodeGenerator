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

void parse_assign_stmt(){
    struct InstructionNode * iNode = new InstructionNode;
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
    if (lexer.peek(1).token_type == SEMICOLON){        
        iNode->assign_inst.operand1_index = location[token.lexeme];
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
        token = lexer.GetToken(); 
}

void parse_input_stmt(){
    InstructionNode* iNode = new InstructionNode;
    iNode->type = IN;
    //get variable's name
    token = lexer.GetToken();
    iNode->input_inst.var_index = location[token.lexeme];
    current->next = iNode;      
    current = iNode;
    //parse semicolon
    token = lexer.GetToken();
    //get the next beginning of statements
    token = lexer.GetToken();
}

void parse_output_stmt(){
    InstructionNode* iNode = new InstructionNode;
    iNode->type = OUT;
    //get the variable to print
    token = lexer.GetToken();

    iNode->output_inst.var_index = location[token.lexeme];
    current->next = iNode;      
    current = iNode;
    //parse the semicolon
    token = lexer.GetToken();
    //get the next beginning of statements
    token = lexer.GetToken();
}

void parse_statement_list(){
    try{        
        //Get the beginning lexeme of statements
        token = lexer.GetToken();
        //This while loop stops when token.type is none of the statements's beginning
        while(token.token_type == ID || token.token_type == INPUT || token.token_type == OUTPUT){
            switch (token.token_type)
            {
            case ID:
                parse_assign_stmt();
                break;
            case INPUT:
                parse_input_stmt();
                break;
            case OUTPUT:
                parse_output_stmt();            
            default:
                break;
            }            
        } 
        //end
    } catch (exception& e) {
        cout << "Syntax error." << endl;
    }
}

// Function to print the linked list
void printLinkedList(InstructionNode* head) {
    InstructionNode* current = head;

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

    //parse RBRACE (already parsed by parse_statement_list)
    // token = lexer.GetToken();
    // cout << "RBRACE" << token.lexeme << endl;
    // cout << "RBRACE" << token.token_type << endl;

    //Inputs
    parse_num_list();

    //printLinkedList(head);
    
    return head;
}