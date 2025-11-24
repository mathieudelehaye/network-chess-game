grammar SimpleChessGame;

// Entry rule: a chess game is a list of moves or comments
game
    : (comment | strike)* EOF
    ;

// Comment parsing
comment
    : COMMENT
    ;

// Strike parsing
strike
    : COORD '-' COORD
    ;

// Lexer rules
COORD   : [a-h][1-8] ;
COMMENT : '//' ~[\r\n]* -> skip ;
WS      : [ \t\r\n]+    -> skip ;