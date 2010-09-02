def removePiece(tcord, tpiece, opcolor):
    print "REMOVE:", tcord, tpiece, opcolor

def addPiece(tcord, piece, color):
    print "ADD:", tcord, piece, color

def movePiece(fcord, tcord, piece, color):
    print "MOVE:", fcord, tcord, piece, color

from pychess.Utils.const import *
def parsemove(board, arBoard, move): 
    flag = move >> 12
    fcord = (move >> 6) & 63
    tcord = move & 63
        
    fpiece = arBoard[fcord]
    tpiece = arBoard[tcord]
        
    opcolor = 1 - board.color
        
    #board.opchecked = None
    #    board.checked = None
        
    # Capture
    if tpiece != EMPTY:
        if board.variant == FISCHERRANDOMCHESS:
                # don't capture _our_ piece when castling king steps on rook!
            if flag not in (KING_CASTLE, QUEEN_CASTLE):
                removePiece(tcord, tpiece, opcolor)
        else:
            removePiece(tcord, tpiece, opcolor)
        
    if fpiece == PAWN:
        if flag == ENPASSANT:
            takenPawnC = tcord + (board.color == WHITE and -8 or 8)
            removePiece (takenPawnC, PAWN, opcolor)
                
        elif flag in PROMOTIONS:
            piece = flag - 2 # The flags has values: 7, 6, 5, 4
            removePiece(fcord, PAWN, board.color)
            addPiece(tcord, piece, board.color)
                
        #if fpiece == PAWN and abs(fcord-tcord) == 16:
        #    board.setEnpassant ((fcord + tcord) / 2)
        #else: board.setEnpassant (None)
        
    """if flag in (KING_CASTLE, QUEEN_CASTLE):
        if flag == QUEEN_CASTLE:
            if board.variant == FISCHERRANDOMCHESS:
                if board.color == WHITE:
                    rookf = board.ini_rooks[0][0]
                    rookt = D1
                else:
                    rookf = board.ini_rooks[1][0]
                    rookt = D8
                # don't move our rook yet
            else:
                rookf = fcord - 4
                rookt = fcord - 1
                board._move (rookf, rookt, ROOK, board.color)
        else:
            if board.variant == FISCHERRANDOMCHESS:
                if board.color == WHITE:
                    rookf = board.ini_rooks[0][1]
                    rookt = F1
                else:
                    rookf = board.ini_rooks[1][1]
                    rookt = F8
                    # don't move our rook yet
            else:
                rookf = fcord + 3
                rookt = fcord + 1
                board._move (rookf, rookt, ROOK, board.color)
        board.hasCastled[board.color] = True
    """
    
        
    # Clear castle flags
    """if board.color == WHITE:
        if fpiece == KING:
            if board.castling & W_OOO:
                    board.hash ^= W_OOOHash
                    board.castling &= ~W_OOO
                    
            if board.castling & W_OO:
                    board.hash ^= W_OOHash
                    board.castling &= ~W_OO
                    
        if fpiece == ROOK:
                if fcord == board.ini_rooks[0][1]: #H1
                    if board.castling & W_OO:
                        board.hash ^= W_OOHash
                        board.castling &= ~W_OO
                    
                elif fcord == board.ini_rooks[0][0]: #A1
                    if board.castling & W_OOO:
                        board.hash ^= W_OOOHash
                        board.castling &= ~W_OOO
            
        if tpiece == ROOK:
                if tcord == board.ini_rooks[1][1]: #H8
                    if board.castling & B_OO:
                        board.hash ^= B_OOHash
                        board.castling &= ~B_OO
            
                elif tcord == board.ini_rooks[1][0]: #A8
                    if board.castling & B_OOO:
                        board.hash ^= B_OOOHash
                        board.castling &= ~B_OOO
    else:
        if fpiece == KING:
                if board.castling & B_OOO:
                    board.hash ^= B_OOOHash
                    board.castling &= ~B_OOO
                    
                if board.castling & B_OO:
                    board.hash ^= B_OOHash
                    board.castling &= ~B_OO
            
        if fpiece == ROOK:
                if fcord == board.ini_rooks[1][1]: #H8
                    if board.castling & B_OO:
                        board.hash ^= B_OOHash
                        board.castling &= ~B_OO
            
                elif fcord == board.ini_rooks[1][0]: #A8
                    if board.castling & B_OOO:
                        board.hash ^= B_OOOHash
                        board.castling &= ~B_OOO
            
        if tpiece == ROOK:
                if tcord == board.ini_rooks[0][1]: #H1
                    if board.castling & W_OO:
                        board.hash ^= W_OOHash
                        board.castling &= ~W_OO
                    
                elif tcord == board.ini_rooks[0][0]: #A1
                    if board.castling & W_OOO:
                        board.hash ^= W_OOOHash
                        board.castling &= ~W_OOO
    """
    
    if not flag in PROMOTIONS:
        if board.variant == FISCHERRANDOMCHESS:
            if flag in (KING_CASTLE, QUEEN_CASTLE):
                if tpiece == EMPTY:
                    movePiece(fcord, tcord, KING, board.color)
                    movePiece(rookf, rookt, ROOK, board.color)
                else:
                    removePiece(rookf, ROOK, board.color)
                    if flag == KING_CASTLE:
                        movePiece(fcord, rookt+1, KING, board.color)
                    else:
                        movePiece(fcord, rookt-1, KING, board.color)
                    addPiece(rookt, ROOK, board.color)
            else:
                movePiece(fcord, tcord, fpiece, board.color)
        else:
            movePiece(fcord, tcord, fpiece, board.color)
        
    #setColor(opcolor)
    #updateBoard()
