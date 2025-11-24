grammar PGN;

// Entry point - can parse either a full game or just a single move
parse
    : pgn_game EOF          # FullGame
    | san_move EOF          # SingleMove
    ;

// Full PGN game structure
pgn_game
    : tag_section movetext_section
    ;

// Tag section (metadata)
tag_section
    : tag_pair*
    ;

tag_pair
    : LEFT_BRACKET tag_name tag_value RIGHT_BRACKET
    ;

tag_name
    : SYMBOL
    ;

tag_value
    : STRING
    ;

// Movetext section (the actual game moves)
movetext_section
    : element_sequence game_termination
    ;

element_sequence
    : (element | recursive_variation)*
    ;

element
    : move_number_indication
    | san_move
    | comment
    ;

move_number_indication
    : INTEGER PERIOD PERIOD? PERIOD?  // Handles "1.", "1...", etc.
    ;

// Standard Algebraic Notation move
san_move
    : CASTLE_KINGSIDE       # CastleKingside
    | CASTLE_QUEENSIDE      # CastleQueenside
    | PAWN_MOVE             # PawnMove
    | PIECE_MOVE            # PieceMove
    ;

recursive_variation
    : LEFT_PAREN element_sequence RIGHT_PAREN
    ;

comment
    : BRACE_COMMENT
    ;

game_termination
    : WHITE_WINS
    | BLACK_WINS
    | DRAWN_GAME
    | ASTERISK
    | /* empty */
    ;

// Lexer rules
WHITE_WINS      : '1-0' ;
BLACK_WINS      : '0-1' ;
DRAWN_GAME      : '1/2-1/2' ;
ASTERISK        : '*' ;

// Castling
CASTLE_KINGSIDE  : 'O-O' ('+'|'#')? ;
CASTLE_QUEENSIDE : 'O-O-O' ('+'|'#')? ;

// Piece moves: [Piece][from_file]?[from_rank]?[x]?[to_square][=Promotion]?[+#]?
// Examples: Nf3, Nxe5, N1c3, Qh4+, e8=Q#
PIECE_MOVE
    : [KQRBN] [a-h]? [1-8]? 'x'? [a-h][1-8] ('=' [QRBN])? [+#]?
    ;

// Pawn moves: [from_file]?[x]?[to_square][=Promotion]?[+#]?
// Examples: e4, exd5, e8=Q, d5+
PAWN_MOVE
    : [a-h]? 'x'? [a-h][1-8] ('=' [QRBN])? [+#]?
    ;

LEFT_BRACKET    : '[' ;
RIGHT_BRACKET   : ']' ;
LEFT_BRACE      : '{' ;
RIGHT_BRACE     : '}' ;
LEFT_PAREN      : '(' ;
RIGHT_PAREN     : ')' ;

PERIOD          : '.' ;

STRING
    : '"' (~["\r\n] | '\\"')* '"'
    ;

INTEGER
    : [0-9]+
    ;

SYMBOL
    : [a-zA-Z][a-zA-Z0-9_]*
    ;

BRACE_COMMENT
    : '{' ~[}]* '}'
    ;

REST_OF_LINE_COMMENT
    : ';' ~[\r\n]* -> skip
    ;

NEWLINE
    : [\r\n]+ -> skip
    ;

SPACES
    : [ \t]+ -> skip
    ;