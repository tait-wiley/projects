module Moves where

import Checkers.Types

-- Tait Wiley, 10064664

-- Main function of moves, takes a gamestate, and returns a tuple of simple moves and jump moves
moves :: GameState -> ([Move], [Move])
moves gs = (simpleMoves gs, jumpMoves gs)



-----------------------------------------------------------------------------------------------------
--Code for simple moves



-- simpleMoves, takes a gamestate and returns a list of simple moves possible
-- the king moves are checked for a repeated state before being accepted
-- Combines the returns of kMoves and pMoves (king moves and pawn moves)
simpleMoves :: GameState -> [Move]
simpleMoves gs = (repeatState (history gs) (kMoves gs)) ++ (pMoves gs)      



-----------------------------------------------------------------------------------------------------
-- Code for Pawn simple moves, used by Pawn moves



-- pMoves, takes a gamestate and returns a list of all the simple pawn moves possible
-- Checks for which players turn it is, then iterates through the pawns checking their possible moves
pMoves :: GameState -> [Move]
pMoves gs
    |   (status gs) == Turn Red = pieceIter gs (redPieces gs) pMove
    |   (status gs) == Turn Black = pieceIter gs (blackPieces gs) pMove
    |   otherwise = []

-- pMove, takes a gamestate and a coordinate, returns a list of simple pawn moves 
-- possible from that coordinate, (for the current players turn).
pMove :: GameState -> Coord -> [Move]
pMove gs pawn
    |   (status gs) == Turn Red = redPMove gs pawn
    |   (status gs) == Turn Black = blackPMove gs pawn
    |   otherwise = []

-- redPMove, takes a gamestate and a red pawn Coord, returning the list of simple moves
-- it can take.
redPMove :: GameState -> Coord -> [Move]
redPMove gs pawn = lOfl (redPRight gs pawn) (redPLeft gs pawn) 

-- blackPMove, takes a gameState and a black pawn Coord, returning the list of simple
-- moves it can take
blackPMove :: GameState -> Coord -> [Move]
blackPMove gs pawn = lOfl (blackPRight gs pawn) (blackPLeft gs pawn) 

-- redPLeft, takes a gamestate and a red pawn Coord,
-- returns a red pawn left move (if possible)
redPLeft :: GameState -> Coord -> Move
redPLeft gs (x,y)
    |   (y-1 == 0) && onBoard (x-1, y-1) && (not (occupied gs (x-1, y-1))) = [P (x, y), K (x-1, y-1)]  
    |   onBoard (x-1, y-1) && (not (occupied gs (x-1, y-1))) = [P (x, y), P (x-1, y-1)]  
    |   otherwise = []

-- redPRight, takes a gamestate and a red pawn Coord,
-- returns a red pawn right move (if possible)
redPRight :: GameState -> Coord -> Move
redPRight gs (x,y)
    |   (y-1 == 0) && onBoard (x+1, y-1) && (not (occupied gs (x+1, y-1))) = [P (x, y), K (x+1, y-1)]  
    |   onBoard (x+1, y-1) && (not (occupied gs (x+1, y-1))) = [P (x, y), P (x+1, y-1)]  
    |   otherwise = []

-- blackPLeft, takes a gamestate and a black pawn Coord,
-- returns a black pawn left move (if possible)
blackPLeft :: GameState -> Coord -> Move
blackPLeft gs (x,y)
    |   (y+1 == 7) && onBoard (x-1, y+1) && (not (occupied gs (x-1, y+1))) = [P (x, y), K (x-1, y+1)]  
    |   onBoard (x-1, y+1) && (not (occupied gs (x-1, y+1))) = [P (x, y), P (x-1, y+1)]  
    |   otherwise = []

-- blackPRight, takes a gamestate and a black pawn Coord,
-- returns a black pawn right move (if possible)
blackPRight :: GameState -> Coord -> Move
blackPRight gs (x,y)
    |   (y+1 == 7) && onBoard (x+1, y+1) && (not (occupied gs (x+1, y+1))) = [P (x, y), K (x+1, y+1)]  
    |   onBoard (x+1, y+1) && (not (occupied gs (x+1, y+1))) = [P (x, y), P (x+1, y+1)]  
    |   otherwise = []




------------------------------------------------------------------------------------------------------
-- Code for king simple moves, used by simpleMoves



-- kMoves, takes a gamestate and returns the list of all possible king moves (of current player)
-- Checks which player's turn it is, then iterates through the appropriate pieces,
-- returning a list of their moves
kMoves :: GameState -> [Move]
kMoves gs
    |   (status gs) == Turn Red = pieceIter gs (redKings gs) kMove
    |   (status gs) == Turn Black = pieceIter gs (blackKings gs) kMove
    |   otherwise = []

-- kMove, takes a gamestate and a king Coord, and returns all simple moves possible for that king
kMove :: GameState -> Coord -> [Move]
kMove gs king = listAdd (kSE gs king) (listAdd (kSW gs king)  (lOfl (kNE gs king) (kNW gs king)))

-- kSE, takes a gamestate and a king Coord, returns a SE king move (if possible)
kSE :: GameState -> Coord -> Move
kSE gs (x,y)
    |   onBoard (x+1, y+1) && (not (occupied gs (x+1, y+1))) = [K (x, y), K (x+1, y+1)]  
    |   otherwise = []


-- kSW, takes a gamestate and a king Coord, returns a SW king move (if possible)
kSW :: GameState -> Coord -> Move
kSW gs (x,y)
    |   onBoard (x-1, y+1) && (not (occupied gs (x-1, y+1))) = [K (x, y), K (x-1, y+1)]  
    |   otherwise = []


-- kNE, takes a gamestate and a king Coord, returns a NE king move (if possible)
kNE :: GameState -> Coord -> Move
kNE gs (x,y)
    |   onBoard (x+1, y-1) && (not (occupied gs (x+1, y-1))) = [K (x, y), K (x+1, y-1)]  
    |   otherwise = []

-- kNW, takes a gamestate and a king Coord, returns a NW king move (if possible)
kNW :: GameState -> Coord -> Move
kNW gs (x,y)
    |   onBoard (x-1, y-1) && (not (occupied gs (x-1, y-1))) = [K (x, y), K (x-1, y-1)]  
    |   otherwise = []



------------------------------------------------------------------------------------------------
-- jumpMoves -- Code format for this section taken Tutorial 11



-- jumpMoves, takes a gamestate and returns a list of possible jump moves
-- Combines the returns of jumpKing and jumpPiece (the list of king jumps and pawn jumps)
jumpMoves:: GameState -> [Move]
jumpMoves gs
                | _status gs == Red
                    = (jumpKing (redKings gs) gs) ++ (jumpPiece (redPieces gs) gs)
                | _status gs == Black
                    = (jumpKing (blackKings gs) gs) ++ (jumpPiece (blackPieces gs) gs)
                | otherwise = [] 



------------------------------------------------------------------------------------------------
-- Code for pawn jumps, used by jumpPiece



-- jumpPiece, takes a piecestate and a gamestate, and returns a list of all possible jumps for pawns
-- returns of the form [P (x, y):ys], where (x,y) is determined from the Coord in the piecestate
-- and ys is determined by jumpPiece2
jumpPiece :: PieceState -> GameState -> [Move]
jumpPiece ps gs = [P (x, y):ys | (x, y) <- ps, ys <- jumpPiece2 (x, y) [] (x, y) gs]

-- jumpPiece2, takes the start Coord of the pawn, a list of the spots it has jumped over, 
-- its current Coord on the board, and a gamestate.  
-- Returns a list of jump moves
-- updates the rem list each time a piece is jumped, so it can no longer jump that spot when called recursively
jumpPiece2 :: Coord -> [Coord] -> Coord -> GameState -> [Move]
jumpPiece2 start rem (x, y) gs 
    |   kinged gs (x,y) = jumpKing2 start rem (x,y) gs
    |   otherwise = [(if y'' == 0 || y'' == 7 then K else P) (x'', y''):ys | ((x', y'),(x'',y'')) <-  
            (if status gs == Turn Red then [((x+1,y-1),(x+2,y-2)),((x-1,y-1),(x-2,y-2))] else [((x+1,y+1),(x+2,y+2)),((x-1,y+1),(x-2,y+2))]),
            not ((x',y') `elem` rem) &&
            opponent_occupied(x', y') gs &&
            (notoccupied (x'', y'') gs || start == (x'',y'')) &&
            onBoard(x'',y''),
            ys <- jump_over(jumpPiece2 start ((x',y'):rem) (x'', y'') gs)]

-- jumpKing, takes a piecestate and a gamestate, and returns a list of all possible king jumps
-- returns of the form [K (x, y):ys], where (x,y) is determined from the Coord in the piecestate
-- ys is determined by jumpKing 2
jumpKing :: PieceState -> GameState -> [Move]
jumpKing ps gs = [K (x, y):ys | (x, y) <- ps, ys <- jumpKing2 (x, y) [] (x, y) gs]

-- jumpKing2, takes the start coordinate of a king, a list of jumped spots, the current spot on the board, and a gamestate
-- returns a list of jump moves for the king
-- updates the rem list with the spots it has jumped over when it calls recursively
jumpKing2 :: Coord -> [Coord] -> Coord -> GameState -> [Move]
jumpKing2 start rem (x, y) gs = 
    [K (x'', y''):ys | ((x', y'),(x'',y'')) <-  [((x+1,y+1),(x+2,y+2)),((x-1,y+1),(x-2,y+2)),((x+1,y-1),(x+2,y-2)),((x-1,y-1),(x-2,y-2))],
    not ((x',y') `elem` rem) &&
    opponent_occupied(x', y') gs &&
    (notoccupied (x'', y'') gs || start == (x'',y'')) &&
    onBoard(x'',y''),
    ys <- jump_over(jumpKing2 start ((x',y'):rem) (x'', y'') gs)]

-- jump_over, small formatting function for the jump functions
-- returns the paramater for anything other than an empty list, in which case it
-- returns a list with an empty list as the element 
jump_over [] = [[]]
jump_over z = z



-----------------------------------------------------------------------------------------------------------------------------------------
-- Code for checking for a repeated state



-- repeatState, takes a history,  a set of moves, and returns that set of moves minus any moves that cause a repeated state
-- Calls historyCheck the history and movement set given
repeatState :: [Move] -> [Move] -> [Move]
repeatState [] moveList = moveList
repeatState _ [] = []
repeatState history (m1:mRest)
    |   historyCheck history [m1] == True = repeatState history mRest
    |   otherwise = m1 : (repeatState history mRest)

-- history Check, takes a history, and a movement set (initialized to the potential move)
-- returns a Bool
-- if the last Coord of the latest move in the history = the first move of the potentia move,
-- the history move is merged with the potential move in the movement list
-- otherwise, the move in history is moved to the movement list
-- this process repeats until one of the lists is empty
historyCheck :: [Move] -> [Move] -> Bool
historyCheck _ [] = True
historyCheck [] _ = False
historyCheck (hLatest:hRest) (mPot:mRest)
    |   getEnd hLatest == getStart mPot = historyCheck (hRest) (moveCleanse (setStart mPot ((getStart hLatest)):mRest))
    |   otherwise = historyCheck (hRest) (mPot:hLatest:mRest)

-- takes a list of moves, and deletes those that have the same start and end
moveCleanse :: [Move] -> [Move]
moveCleanse [] = []
moveCleanse (x:xs)
    |   getStart x == getEnd x = moveCleanse xs
    |   otherwise = x: moveCleanse xs

-- moveMerge, takes two moves and combines them
moveMerge :: Move -> Move -> Move
moveMerge (x:xs) (y:ys) = x:ys

-- getStart, takes a move and returns it's start Coord
getStart :: Move -> PorK Coord
getStart (x:xs) = x

-- getEnd, takes a move and returns it's end Coord
getEnd :: Move -> PorK Coord
getEnd [a] = a
getEnd (x:xs) = getEnd xs

-- set Start, takes a move, a Coord, and sets it's first coord as the given coord
setStart :: Move -> PorK Coord -> Move
setStart (x:xs) y = (y:xs)



--------------------------------------------------------------------------------------------------
-- General functions used by the main functions



-- pieceIter, takes a piecestate, iterates through the pieces and applys a checkers function to them, returning a list of moves
pieceIter :: GameState -> PieceState -> (GameState -> Coord -> [Move]) -> [Move]
pieceIter gs [] f = []
pieceIter gs (x : xs) f = (f gs x) ++ (pieceIter gs xs f)

-- pieceStateCheck Takes a spot on the board and a piecestate, returns if that spot is occupied. 
pieceStateCheck :: Coord -> [Coord] -> Bool
pieceStateCheck _ [] = False
pieceStateCheck spot (x : xs)
    |   spot == x = True
    |   otherwise = pieceStateCheck spot (xs)

--occupied, takes a gamestate and coordinate, checks if there is a piece there, returns True/False
occupied :: GameState -> Coord -> Bool
occupied gs spot
    |   pieceStateCheck spot (blackPieces gs) == True = True
    |   pieceStateCheck spot (redPieces gs) == True = True
    |   pieceStateCheck spot (blackKings gs) == True = True
    |   pieceStateCheck spot (redKings gs) == True = True
    |   otherwise = False

--onboard, takes a spot on the board, checks if it is within the 8/8 spot allowance
onBoard :: Coord -> Bool
onBoard (x,y)
    |   x > 7 || x < 0 = False
    |   y > 7 || y < 0 = False 
    |   otherwise = True

-- kinged, takes a gamestate and a Coord, checks if a pawn of the current player's turn
-- would be kinged on that Coord
kinged :: GameState -> Coord -> Bool
kinged gs (x,y)
    |   status gs == Turn Red && y == 0 = True
    |   status gs == Turn Black && y == 7 = True
    |   otherwise = False

-- list of list, takes 2 lists and returns a list of them
lOfl :: [a] -> [a] -> [[a]]
lOfl [] [] = []
lOfl [] x = [x]
lOfl x [] = [x]
lOfl x y = [x, y]

-- adds a list to a list of lists, but does not add the empty list []
listAdd :: [a] -> [[a]] -> [[a]]
listAdd [] [] = []
listAdd [] x = x
listAdd x [] = [x]
listAdd x y = x:y

-- returns the length of the list
listL :: [a] -> Int
listL [] = 0
listL [a] = 1
listL (x:xs) = 1 + listL xs

-- toCoord, takes a PorK Coord and returns just the Coord
toCoord :: PorK Coord -> Coord
toCoord (K (x,y)) = (x,y)
toCoord (P (x,y)) = (x,y)

-- _status,  takes a gamestate and returns the current player turn
_status :: GameState -> Player
_status st
    | status st == Turn Red = Red
    | otherwise = Black

-- dir, takes a gamestate and returns the play direction as an int
dir :: GameState -> Int
dir g
    | _status g == Red = -1
    | otherwise = 1

-- notoccupied, takes a Coord and a gamestate, and returns False if there is a piece occupying the Coord
notoccupied :: (Int, Int) -> GameState -> Bool
notoccupied pos st
    | pos `elem` (redPieces st) = False
    | pos `elem` (blackPieces st) = False
    | pos `elem` (redKings st) = False
    | pos `elem` (blackKings st) = False
    | otherwise = True

-- opponent_occupied, takes a Coord and a GameState, and returns wheather an opponents
-- piece is on the Coord, (determined by the current turn status)
opponent_occupied :: Coord -> GameState -> Bool
opponent_occupied xy st 
                | _status st == Red
                    = xy `elem` blackPieces st || xy `elem` blackKings st
                | _status st == Black
                    = xy `elem` redPieces st || xy `elem` redKings st
                | otherwise = False

























