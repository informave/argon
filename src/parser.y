//
// parser.y - Grammar definition
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
// 
// GPL 3.0 License
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


%start_symbol start

%token_prefix ARGON_TOK_

%token_type {informave::argon::Token*}

%default_type {Node*}

%extra_argument { informave::argon::ParseTree *tree }

%include {
         #include "argon/ast.hh"
         #include "argon/token.hh"
         #include <iostream>  

         using namespace informave::argon;

         //#define YYNOERRORRECOVERY 1
     
         #define CREATE_NODE(type)  type *node = tree->newNode<type>()
         #define ADD_TOKEN(n, t)     n->addChild( tree->newTokenNode(t) )
}  


%syntax_error {  
              tree->raiseSyntaxError();
} 

//%left PLUS MINUS.   
//%left DIVIDE TIMES.  
   
//%left LITERAL SEP ID.
//%left CONNECTION TYPE DBCSTR PROGRAM TASK AS TEMPLATE LP RP LB RB BEGIN END.




////////////// Caller arguments ////////////////////////////

%type callArgs { NodeList* }
%type callArgList { NodeList* }

callArgs(A) ::= LP callArgList(B) RP. { A = B; }
callArgs(A) ::= LP RP .               { A = tree->newNodeList(); }
callArgs(A) ::= .                     { A = tree->newNodeList(); }


callArgList(A) ::= callArgList(B) COMMA callArgItem(C). {
            A = B;
            assert(A);
            A->push_back(C);
}

callArgList(A) ::= callArgItem(B). {
               A = tree->newNodeList();
               A->push_back(B);
}

callArgItem(A) ::= ID(B). {
               CREATE_NODE(IdNode);
               node->init(Identifier(B->data()));
               A = node;
}

callArgItem(A) ::= LITERAL(B). {
               CREATE_NODE(LiteralNode);
               node->init(B->data());
               A = node;
}

callArgItem(A) ::= COLUMN(B). {
               CREATE_NODE(ColumnNode);
               node->init(B->data());
               A = node;
}

/// other


arglist ::= LP arglistx RP.
arglistx ::= arglistx argitem COMMA.
arglistx ::= arglistx argitem.
arglistx ::= .

argitem ::= ID.
argitem ::= LITERAL.


start ::= setuplist PROGRAM inslist.

setuplist ::= setuplist conn.
setuplist ::= setuplist decl.
setuplist ::= .

decl ::= DECLARE ID(A) declArgList AS otype SEP. { 
     std::cout << "Declare: " << A->data() << std::endl; 
}

otype ::= TABLE arglist declBody.
otype ::= VIEW arglist declBody.
otype ::= PROCEDURE arglist declBody.
otype ::= SQL arglist declBody.

declBody ::= BEGIN bodyExpr END.
declBody ::= .


declArgList ::= LP declArgItems RP.
declArgList ::= LP  RP.
declArgList ::= .
declArgItems ::= declArgItems COMMA declArgItem.
declArgItems ::= declArgItem. { std::cout << "decl args" << std::endl; }
declArgItem ::= ID.



/*
declArgList ::= LP declArgItems declArgItem RP. // (item1, item2, item3)
declArgList ::= .

declArgItems ::= declArgItems declArgItem COMMA.
declArgItems ::= .

declArgItem ::= ID.
*/


////////////// Connection ////////////////////////////

%type connspec { ConnSpec* }

conn ::= CONNECTION(Y) ID(A) connspec(B) SEP(Z). {
     CREATE_NODE(ConnNode);
     node->init(Identifier(A->data()), B);
     tree->addChild(node);

     ADD_TOKEN(node, Y);
     ADD_TOKEN(node, Z);
}


connspec(A) ::= TYPE LITERAL(B) DBCSTR LITERAL(C). {
            CREATE_NODE(ConnSpec);
            node->init(B->data(), C->data());
            A = node;
}


connspec(A) ::= . {
            CREATE_NODE(ConnSpec);
            A = node;
}


////////////// Instructions ////////////////////////////


inslist ::= inslist ins.
inslist ::= .


//ins ::= error.

ins ::= task.

%type log { LogNode* }

%type logArgs { NodeList* }

log(X) ::= LOG(Y) logArgs(A) SEP(Z). {
    CREATE_NODE(LogNode);
    //node->addChild(LogLevelNode());
    assert(A && A->size() > 0);
    node->addChilds(A); // copy nodelist to node

    ADD_TOKEN(node, Y);
    ADD_TOKEN(node, Z);

    X = node;
}

%type logArg { Node* }

logArgs(A) ::= logArgs(B) logArg(C). {
   A = B;
   assert(A);
   //if(A == 0) A = new NodeList();
   A->push_back(C);
}

logArgs(A) ::= . { A = tree->newNodeList(); }

logArg(A) ::= ID(B). {
   CREATE_NODE(IdNode);
   node->init(Identifier(B->data()));
   A = node;
}

logArg(A) ::= LITERAL(B). {
   CREATE_NODE(LiteralNode);
   node->init(B->data());
   A = node;
}



////////////// TASK Instruction ////////////////////////////

//%type task { Task* }

%type taskbody { NodeList* }

task ::= TASK(Y) ID(A) LP taskargs RP AS TEMPLATE tmplargs taskbody(B) SEP(Z).
{
   CREATE_NODE(TaskNode);
   node->init(A->data());
   tree->addChild(node);

   assert(B);
   node->addChilds(B);

   ADD_TOKEN(node, Y);
   ADD_TOKEN(node, Z);

   //std::cout << node->getSourceInfo() << std::endl;
   //std::cout << Z->getSourceInfo() << std::endl;



/*
   node->addArgs(Args);
   node->addBody();
   node->setTemplate(T->data());
   node->setTemplateArgs(TA);
*/
}



%type bodyExprList { NodeList* }



taskbody(A) ::= BEGIN bodyExprList(B) END. { A = B; }
taskbody(A) ::= . { A = tree->newNodeList(); }



bodyExprList(A) ::= bodyExprList(B) bodyExpr(C). {
             A = B;
             assert(A); 
             if(C)
                        A->push_back(C);
            else
               std::cout << "ingnoring NULL node" << std::endl;
}

bodyExprList(A) ::= . { A = tree->newNodeList(); }

bodyExpr(A) ::= colAssignExpr. { A = 0; }

bodyExpr(A) ::= log(C). { A = C; }


%type taskExecExpr { TaskExecNode* }

bodyExpr(A) ::= taskExecExpr(B). { A = B; }


taskExecExpr(A) ::= EXEC(Y) TASK ID(B) callArgs(C) SEP(Z). { 
                CREATE_NODE(TaskExecNode);
                node->init(B->data());
                node->addChilds(C);
                ADD_TOKEN(node, Y);
                ADD_TOKEN(node, Z);
                A = node;
}





colAssignExpr ::= COLUMN ASSIGNOP value SEP.

value ::= COLUMN.
value ::= NULL.
value ::= LITERAL.
value ::= NUMBER.
value ::= ID.


taskargs ::= taskargs ID COMMA.
taskargs ::= taskargs ID.
taskargs ::= .

tmplargs ::= LB tmplargsx RB.
tmplargs ::= .
tmplargsx ::= tmplargsx tmplarg COMMA.
tmplargsx ::= tmplargsx tmplarg.
tmplargsx ::= .

tmplarg ::= ID.
tmplarg ::= ID LP tmplArgParams RP.
tmplarg ::= otype.

tmplArgParams ::= tmplArgParams tmplArgParam.
tmplArgParams ::= .

tmplArgParam ::= ID.


