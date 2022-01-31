module ApplyMove where

import Checkers.Types
import Moves

-- Tait Wiley, 10064664


-- main function of ApplyMove, takes a move and a gamestate, and returns a new
-- gamestate with that move applied
-- uses makeSmove for applying a simple move, and jMove for making a jump move
-- type CheckersEngine = Move -> GameState -> GameState
apply_move:: CheckersEngine
apply_move move gs
    |   inSimpleMoves move (moves gs) && jumpMoves gs == [] = makeSMove move gs
    |   inJumpMoves move (moves gs) = jMove move gs
    |   otherwise = gs{message = "Illegal move!!"}



----------------------------------------------------------------------------------------------
-- Code for applying a jump move



-- kMove, takes a jump move and a gamestate, returns a gamestate with the jump move applied
-- uses makeJmove
jMove :: Move -> GameState -> GameState
jMove move gs
    |   status gs == Turn Red = ((makeJMove move gs){status = Turn Black})
    |   status gs == Turn Black = ((makeJMove move gs){status = Turn Red})

-- makeJMove, takes a jump move and a gamestate, returns a gamestate with the jump move applied (and all subsequent jumps)
-- checks what piece the jump move is being applied for
-- deletes the start position Coord and ads the end position Coord to the appriopriate piecestate
-- adds the move to the history
-- for each jump, calls itself again recursively
makeJMove :: Move -> GameState -> GameState
makeJMove [a] gs = gs
makeJMove (start:(next:rest)) gs
       | _status gs == Red && member start (redKings gs)
            = makeJMove (next:rest)
                    (gs{blackKings = deletePiece (jumped start next) (blackKings gs)
                        , blackPieces = deletePiece (jumped start next) (blackPieces gs)
                        , redKings = (toCoord next):(deletePiece (toCoord start) (redKings gs))
                        , message = ""
                        , history = addJHistory [start,next] (history gs)})  

       | _status gs == Black && member start (blackKings gs)
            = makeJMove (next:rest)
                    (gs{redKings = deletePiece (jumped start next) (redKings gs)
                        , redPieces = deletePiece (jumped start next) (redPieces gs)
                        , blackKings = (toCoord next):(deletePiece (toCoord start) (blackKings gs))
                        , message = ""
                        , history = addJHistory [start,next] (history gs)})  
        
        | _status gs == Red && member start (redPieces gs) = redPJump (start:(next:rest)) gs
            
        | _status gs == Black && member start (blackPieces gs) = blackPJump (start:(next:rest)) gs

-- redPJump, takes a move and a gamestate
-- returns a gamestate with the red pawn jump applied
-- seperate function as it needs to take into account if the pawn is kinged after its jump
redPJump :: Move -> GameState -> GameState
redPJump (start:(next:rest)) gs

    |   kinged gs (toCoord next) = makeJMove (next:rest)
                    (gs{blackKings = deletePiece (jumped start next) (blackKings gs)
                        , blackPieces = deletePiece (jumped start next) (blackPieces gs)
                        , redPieces = deletePiece (toCoord start) (redPieces gs)                        
                        , redKings = (toCoord next):(deletePiece (toCoord start) (redKings gs))
                        , message = ""
                        , history = addJHistory [start,next] (history gs)})          
    |   otherwise = makeJMove (next:rest)
            (gs{blackKings = deletePiece (jumped start next) (blackKings gs)
                , blackPieces = deletePiece (jumped start next) (blackPieces gs)
                , redPieces= (toCoord next):(deletePiece (toCoord start) (redPieces gs))
                , message = ""
                , history = addJHistory [start,next] (history gs)})  

-- blackPJump, takes a move and a gamestate
-- returns a gamestate with the black pawn jump applied
-- seperate function as it needs to take into account if the pawn is kinged after its jump
blackPJump :: Move -> GameState -> GameState
blackPJump (start:(next:rest)) gs

    |   kinged gs (toCoord next) = makeJMove (next:rest)
                    (gs{redKings = deletePiece (jumped start next) (redKings gs)
                        , redPieces = deletePiece (jumped start next) (redPieces gs)
                        , blackPieces = deletePiece (toCoord start) (blackPieces gs)  
                        , blackKings = (toCoord next):(deletePiece (toCoord start) (blackKings gs))
                        , message = ""
                        , history = addJHistory [start,next] (history gs)})          
    |   otherwise = makeJMove (next:rest)
            (gs{redKings = deletePiece (jumped start next) (redKings gs)
                , redPieces = deletePiece (jumped start next) (redPieces gs)
                , blackPieces= (toCoord next):(deletePiece (toCoord start) (blackPieces gs))
                , message = ""
                , history = addJHistory [start,next] (history gs)})  

-- jumped, takes two Coords, and returns the Coord in between them (the jumped square)
jumped :: PorK Coord -> PorK Coord -> Coord
jumped start end = halfWay (toCoord start) (toCoord end)

-- halfWay, takes 2 Coords, (assumed to be a space away diagonal from eachother) and returns the Coord inbetween them
-- used by jumped
halfWay :: Coord -> Coord -> Coord
halfWay (x,y) (w,z)
    |   x > w && y > z = ((x-1), (y-1))
    |   x > w && y < z = ((x-1), (y+1))
    |   x < w && y > z = ((x+1), (y-1))
    |   x < w && y < z = ((x+1), (y+1))



------------------------------------------------------------------------------------------------------------
-- Code for applying a simple move



-- makeSMove, takes a simple move and a gameState, and returns a gamestate with the move applied
-- checks what piece the move is being applied for
-- deletes the start position Coord and ads the end position Coord to the appriopriate piecestate
-- adds the move to the history
makeSMove :: Move -> GameState -> GameState
makeSMove [start,end] gs
    | _status gs == Red && member start (redKings gs)
        = gs{redKings = replace start end (redKings gs)
            , status = changePlayer gs
            , message = ""
            , history = addHistory [start,end] (history gs)}
    | _status gs == Black && member start (blackKings gs)
        = gs{blackKings = replace start end (blackKings gs)
            , status = changePlayer gs
            , message = ""
            , history = addHistory [start,end] (history gs)}
    | _status gs == Red && member start (redPieces gs) = makeRPMove [start, end] gs    
    | _status gs == Black && member start (blackPieces gs) = makeBPMove [start, end] gs

-- redPMove, takes a move and a gamestate
-- returns a gamestate with the red pawn move applied
-- seperate function as it needs to take into account if the pawn is kinged after its move
makeRPMove :: Move -> GameState -> GameState
makeRPMove [start, end] gs
    |   kinged gs (toCoord end)
        = gs{redPieces = deletePiece (toCoord start) (redPieces gs)
            , redKings = insertPiece (toCoord end) (redKings gs)
            , status = changePlayer gs
            , message = ""
            , history = addHistory [start,end] (history gs)}  
    |   otherwise
        = gs{redPieces = replace start end (redPieces gs)
            , status = changePlayer gs
            , message = ""
            , history = addHistory [start,end] (history gs)}  

-- blackPMove, takes a move and a gamestate
-- returns a gamestate with the black pawn move applied
-- seperate function as it needs to take into account if the pawn is kinged after its move
makeBPMove :: Move -> GameState -> GameState
makeBPMove [start, end] gs
    |   kinged gs (toCoord end)
        = gs{blackPieces = deletePiece (toCoord start) (blackPieces gs)
            , blackKings = insertPiece (toCoord end) (blackKings gs)
            , status = changePlayer gs
            , message = ""
            , history = addHistory [start,end] (history gs)}  
    |   otherwise
        = gs{blackPieces = replace start end (blackPieces gs)
            , status = changePlayer gs
            , message = ""
            , history = addHistory [start,end] (history gs)}  



-----------------------------------------------------------------------------------------------------
-- General functions used by the main functions above



-- changePlayer, takes a gamestate and returns a status (of the opposit player)
changePlayer :: GameState -> Status
changePlayer gs
    |   status gs == Turn Red = Turn Black
    |   status gs == Turn Black = Turn Red

-- member, takes a Coord and a piecestate, returns whether the piece on that Coord belongs to that piecestate
member :: PorK Coord -> PieceState -> Bool
member piece [] = False
member piece (x:xs)
    |   toCoord piece == x = True
    |   otherwise = member piece xs

-- uses deletePiece and insertPiece to move one piece in a given pieceState
replace :: PorK Coord -> PorK Coord -> PieceState -> PieceState
replace start end ps = insertPiece (toCoord end) (deletePiece (toCoord start) ps)

-- Deletes one piece in a given pieceState
deletePiece :: Coord -> PieceState -> PieceState
deletePiece piece [] = []
deletePiece piece (x:xs)
    |   piece == x = xs
    |   otherwise = x : deletePiece piece xs

-- inserts a piece into a given piecestate
insertPiece :: Coord -> PieceState -> PieceState
insertPiece piece [] = [piece]
insertPiece piece ps = piece : ps

-- Adds a move to a list of moves (the history)
addHistory :: Move -> [Move] -> [Move]
addHistory [] history = history
addHistory move history = move : history

-- special history adding function for jump moves
-- checks if the first coordinate of the given move is the last coordinate of the most recent move
-- if it is, no need to create a new move, and the last coord of the move is appended to the first element of the history
-- otherwise, a new move in the history is started
addJHistory :: Move -> [Move] -> [Move]
addJHistory [] history = history
addJHistory move [] = [move]
addJHistory (y:ys) (x:xs)                           
    |   y == getLast x = ([x ++ ys] ++ xs)
    |   otherwise = (y:ys):x:xs

-- getting the last PorK Coord of a Move
getLast :: Move -> PorK Coord
getLast [x] = x 
getLast (_:xs) = getLast xs 

-- Checks if a move exists in the tuple returned by moves
inMoves :: Move -> ([Move], [Move]) -> Bool
inMoves move (simple ,jump)
    |   inMove move simple || inMove move jump = True   
    |   otherwise = False

-- checks if a move is in a list of moves
inMove :: Move -> [Move] -> Bool
inMove move [] = False
inMove move (x:xs)
    |   move == x = True
    |   otherwise = inMove move xs

-- Checks if a move is in the simple move section of the moves tuple
inSimpleMoves :: Move -> ([Move], [Move]) -> Bool
inSimpleMoves move (simple ,jump)
    |   inMove move simple = True   
    |   otherwise = False

-- Checks if a move is in the jump move section of the moves tuple
inJumpMoves :: Move -> ([Move], [Move]) -> Bool
inJumpMoves move (simple ,jump)
    |   inMove move jump = True   
    |   otherwise = False