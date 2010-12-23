//
// parser.y - Grammar definition
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
// 
// Lesser GPL 3.0 License
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


%token_prefix ARGON_TOK_

%token_type {informave::argon::Token*}

%default_type {Node*}

%extra_argument { informave::argon::ParseTree *tree }

%syntax_error {  
	tree->raiseSyntaxError();
}


%include {
	#include "argon/ast.hh"
	#include "argon/token.hh"
	#include <iostream>  

	using namespace informave::argon;

	//#define YYNOERRORRECOVERY 1
     
	#define CREATE_NODE(type)  type *node = tree->newNode<type>()
   #define ADD_TOKEN(n, t)     n->addChild( tree->newTokenNode(t) )
}  



//..............................................................................
////////////////////////////////////////////////////////////////////////// START

%start_symbol start

start ::= setuplist PROGRAM inslist.

setuplist ::= setuplist connDecl.
setuplist ::= setuplist decl.
setuplist ::= .



//..............................................................................
///////////////////////////////////////////////////////// Connection declaration

%type connspec { ConnSpec* }

connDecl ::= CONNECTION(Y) ID(A) connspec(B) SEP(Z). {
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



//..............................................................................
///////////////////////////////////////////////////////////// OBJECT Declaration

%type objType { ObjectNode* }
%type declBody { NodeList* }

decl ::= DECLARE(Y) ID(A) declArgList(C) AS objType(B) declBody(D) SEP(Z). { 
	  ObjectNode *node = B;
	  if(node)
	  {
			node->init(Identifier(A->data()));

		  	node->addChild(C);
			node->addChilds(D);

	   	tree->addChild(node);
	   	ADD_TOKEN(node, Y);
	   	ADD_TOKEN(node, Z);
		}
}


objType(A) ::= tableObj(B).       { A = B; }
objType(A) ::= viewObj(B).        { A = B; }
objType(A) ::= procedureObj(B).   { A = B; }
objType(A) ::= sqlObj(B).         { A = B; }


declBody(A) ::= BEGIN bodyExprList(B) END.  { A = B; }
declBody(A) ::= .                           { A = tree->newNodeList(); }




//..............................................................................
////////////////////////////////////////////////////////////////////////// Table

%type tableObj { ObjectNode* }

tableObj(A) ::= TABLE callArgs(B). {
     CREATE_NODE(TableNode);
	  node->addChild(B);
	  A = node;
}


//..............................................................................
/////////////////////////////////////////////////////////////////////////// View

%type viewObj { ObjectNode* }

viewObj(A) ::= VIEW callArgs. { A = 0; }



//..............................................................................
////////////////////////////////////////////////////////////////////// Procedure

%type procedureObj { ObjectNode* }

procedureObj(A) ::= PROCEDURE callArgs. { A = 0; }



//..............................................................................
///////////////////////////////////////////////////////////////////// SQL Object

%type sqlObj { ObjectNode* }

sqlObj(A) ::= SQL callArgs. { A = 0; }






//..............................................................................
//////////////////////////////////////////////////////////////////// Instruction

inslist ::= inslist ins.
inslist ::= .

ins ::= task.
// ins ::= function.


//..............................................................................
/////////////////////////////////////////////////////////////////////////// TASK

%type taskBody { NodeList* }
%type bodyExprList { NodeList* }

task ::= TASK(Y) ID(A) declArgList(E) AS TEMPLATE(C) tmplArgs(D) taskBody(B) SEP(Z).
{
   CREATE_NODE(TaskNode);
   node->init(A->data(), C->data());

	node->addChild(E);
	node->addChild(D);

   assert(B);
   node->addChilds(B);

   tree->addChild(node);
   ADD_TOKEN(node, Y);
   ADD_TOKEN(node, Z);
}


taskBody(A) ::= BEGIN bodyExprList(B) END. { A = B; }

taskBody(A) ::= . { A = tree->newNodeList(); }


bodyExprList(A) ::= bodyExprList(B) bodyExpr(C). {
	A = B;
   assert(A); 
   if(C)
		A->push_back(C);
   else
		std::cout << "ingnoring NULL node" << std::endl;
}

bodyExprList(A) ::= . { A = tree->newNodeList(); }


bodyExpr(A) ::= colAssignCmd(B). { A = B; }
bodyExpr(A) ::= sqlExecCmd(B).   { A = B; }
bodyExpr(A) ::= logCmd(B).       { A = B; }
bodyExpr(A) ::= taskExecCmd(B).  { A = B; }


//..............................................................................
///////////////////////////////////////////////////////////// Template Arguments

%type tmplArgs { TmplArgumentsNode* }
%type tmplArgList { NodeList* }

tmplArgs(A) ::= LB tmplArgList(B)  RB. {
				CREATE_NODE(TmplArgumentsNode);
				node->addChilds(B);
				A = node;
}

tmplArgs(A) ::= . {
				CREATE_NODE(TmplArgumentsNode);
				A = node;				
}

tmplArgList(A) ::= tmplArgList(B) COMMA tmplArgItem(C). {
					A = B;
					assert(A);
					A->push_back(C);
}

tmplArgList(A) ::= tmplArgItem(B). {
					A = tree->newNodeList();
					A->push_back(B);
}


tmplArgItem(A) ::= id(B). {
               A = B;
}

tmplArgItem(A) ::= id(B) LP callArgList(C) RP. { // @bug is callArgList ok?
               CREATE_NODE(IdCallNode);
					node->addChild(B);
					node->addChild(C);
               A = node;
}

tmplArgItem(A) ::= objType(B). {
					A = B;
}




//..............................................................................
/////////////////////////////////////////////////////////////////////// EXEC SQL

%type sqlExecCmd { SqlExecNode* }
%type sqlExecParams { ArgumentsNode* }

sqlExecCmd(A) ::= EXEC(Y) SQL LITERAL(B) ON ID(C) sqlExecParams(D) SEP(Z). {
					CREATE_NODE(SqlExecNode);
					node->init(B->data(), C->data());
					node->addChild(D);
					ADD_TOKEN(node, Y);
					ADD_TOKEN(node, Z);
					A = node;
}

sqlExecParams(A) ::= WITH PARAMS callArgs(B). { A = B; }

sqlExecParams(A) ::= . {
					  CREATE_NODE(ArgumentsNode);
					  A = node;
}



//..............................................................................
//////////////////////////////////////////////////////////////////////////// LOG

%type logCmd { LogNode* }
%type logArgs { NodeList* }
%type logArg { Node* }

logCmd(X) ::= LOG(Y) logArgs(A) SEP(Z). {
    CREATE_NODE(LogNode);
    //node->addChild(LogLevelNode());
    assert(A && A->size() > 0);
    node->addChilds(A);
    ADD_TOKEN(node, Y);
    ADD_TOKEN(node, Z);
    X = node;
}

logArgs(A) ::= logArgs(B) logArg(C). {
   A = B;
   assert(A);
   A->push_back(C);
}

logArgs(A) ::= . { A = tree->newNodeList(); }

logArg(A) ::= id(B).      { A = B; }
logArg(A) ::= literal(B). { A = B; }
logArg(A) ::= column(B).  { A = B; }


//..............................................................................
////////////////////////////////////////////////////////////////////// EXEC TASK

%type taskExecCmd { TaskExecNode* }


taskExecCmd(A) ::= EXEC(Y) TASK ID(B) callArgs(C) SEP(Z). { 
                CREATE_NODE(TaskExecNode);
                node->init(B->data());
                node->addChild(C);
                ADD_TOKEN(node, Y);
                ADD_TOKEN(node, Z);
                A = node;
}


//..............................................................................
///////////////////////////////////////////////////////////////////// Assign Cmd

%type colAssignCmd { ColumnAssignNode* }

colAssignCmd(A) ::= column(B) ASSIGNOP value SEP. {
	CREATE_NODE(ColumnAssignNode);
	node->addChild(B);
	A = node;
}

value ::= column.
value ::= NULL.
value ::= LITERAL.
value ::= NUMBER.
value ::= ID.





//..............................................................................
////////////////////////////////////////////////////// Argument list declaration

%type declArgList { ArgumentsSpecNode* }
%type declArgItems { NodeList* }


declArgList(A) ::= LP declArgItems(B) RP.   { 
					CREATE_NODE(ArgumentsSpecNode);
					A = node;
					A->addChilds(B); 
}

declArgList(A) ::= LP  RP. { 
					CREATE_NODE(ArgumentsSpecNode);
					A = node;
}

declArgList(A) ::= . {
					CREATE_NODE(ArgumentsSpecNode);
					A = node;
}

declArgItems(A) ::= declArgItems(B) COMMA declArgItem(C). {
					 A = B;
					 A->push_back(C);
}

declArgItems(A) ::= declArgItem(B). {
					 A = tree->newNodeList();
					 A->push_back(B);
}

declArgItem(A) ::= id(B). { A = B; }


//..............................................................................
///////////////////////////////////////////////////////////////////// Identifier

%type id { IdNode* }
id(A) ::= ID(B). {
               CREATE_NODE(IdNode);
               node->init(Identifier(B->data()));
					ADD_TOKEN(node, B);
               A = node;
}


//..............................................................................
//////////////////////////////////////////////////////////////////////// Literal

%type literal { LiteralNode* }
literal(A) ::= LITERAL(B). {
               CREATE_NODE(LiteralNode);
               node->init(B->data());
					ADD_TOKEN(node, B);
               A = node;
}

//..............................................................................
///////////////////////////////////////////////////////////////////////// Column

// %type column { ColumnNode* }
column(A) ::= COLUMN(B). {
               CREATE_NODE(ColumnNode);
               node->init(B->data());
					ADD_TOKEN(node, B);
               A = node;
}

column(A) ::= COLUMN_NUM(B). {
               CREATE_NODE(ColumnNumNode);
               node->init(B->data());
					ADD_TOKEN(node, B);
               A = node;
}




//..............................................................................
///////////////////////////////////////////////////////////// Call Argument list

%type callArgs { ArgumentsNode* }
%type callArgList { ArgumentsNode* }

callArgs(A) ::= LP callArgList(B) RP. { A = B; }

callArgs(A) ::= LP RP . {
				CREATE_NODE(ArgumentsNode);
				A = node;
}

callArgs(A) ::= . {
				CREATE_NODE(ArgumentsNode);
				A = node;
}

callArgList(A) ::= callArgList(B) COMMA callArgItem(C). {
            A = B;
            assert(A);
            A->addChild(C);
}

callArgList(A) ::= callArgItem(B). {
					CREATE_NODE(ArgumentsNode);
					node->addChild(B);
					A = node;
}

callArgItem(A) ::= id(B).      { A = B; }
callArgItem(A) ::= literal(B). { A = B; }
callArgItem(A) ::= column(B).  { A = B; }

