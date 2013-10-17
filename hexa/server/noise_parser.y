%{
extern int yylex();
void yyerror(const char* s) { printf("ERROR: %s\n", s); }
%}

%token <string> TIDENTIFIER TDOUBLE TINTEGER
%token <token> TASSIGN TLPAREN TRPAREN TCOMMA
%token <token> TPLUS TMINUS TMUL TDIV

%type <ident> ident
%type <expr> numeric expr
%type <exprvec> call_args

%left TPLUS TMINUS
%left TMUL TDIV

%start expr

%%

numeric: TINTEGER { $$ = new num_int(std::stol($1)); }
       | TDOUBLE  { $$ = new num_double(std::stod($1)); }
       ;

ident: TIDENTIFIER { $$ = new identifier($1); }

call_args: /*blank*/ { $$ = new expression_list; }
         | expr { $$ = new expression_list{$1}; }
         | call_args TCOMMA expr { $1->emplace_back(std::move($3)); }
         ;

expr: ident TLPAREN call_args TRPAREN { $$ = new call($1, $3); }
    | ident { $<ident>$ = $1; }
    | numeric
    | TLPAREN expr TRPAREN { $$ = $2; }
    ;

