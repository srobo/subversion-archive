# Routines for extracting BOMs from schematics
import subprocess, tempfile, os, sys, parts_db, schem

PARTS_DB = os.path.expanduser("~/.sr/tools/bom/sr_component_lib")
if not os.path.exists( PARTS_DB ):
    print "Parts DB not found at \"%s\"" % PARTS_DB
    sys.exit(1)

STOCK_OUT = 0
STOCK_OK = 1
STOCK_UNKNOWN = 2

class PartGroup(list):
    """A set of parts
    One might call this a "BOM line" """
    def __init__(self, part, name = "", designators = [] ):
        list.__init__(self)

        for x in designators:
            self.append( (name, designators) )

        self.part = part
        self.name = name

    def stockcheck(self):
        """Check the distributor has enough parts in stock."""
        s = self.part.stockcheck()
        if s == None:
            return None

        if s < self.order_num():
            return False
        return True

    def order_num(self):
        """Returns the number of parts to order from the distributor.
        e.g. if we need 5002 components from a 5000 component reel, this
        will return 2."""

        if self.part.stockcheck() == None:
            "Unable to discover details from distributor..."
            return None

        n = len(self)
        if n == 0:
            return 0

        # Change n to be in distributor units, rather than component units
        # (e.g. number of reels rather than number of components)
        d = n / self.part.get_dist_units()
        if n % self.part.get_dist_units() > 0:
            d = d + 1
        n = d

        if n < self.part.get_min_order():
            "Round up to minimum order"
            n = self.part.get_min_order()
        elif (n % self.part.get_increments()) != 0:
            n = n + (self.part.get_increments() - (n % self.part.get_increments()))

        # Some (hopefully) sane assertions
        assert n % self.part.get_increments() == 0
        assert n >= self.part.get_min_order()

        return n

class Bom(dict):
    def stockcheck(self):
        """Check that all items in the schematic are in stock.
        Returns list of things that aren't in stock."""

        for pg in self.values():
            a = pg.stockcheck()

            if a == None:
                yield (STOCK_UNKNOWN, pg.part)
            elif not a:
                yield (STOCK_OUT, pg.part)
            else:
                yield (STOCK_OK, pg.part)


class BoardBom(Bom):
    """BOM object.
    Groups parts with the same srcode into PartGroups.
    Dictionary keys are sr codes."""
    def __init__(self, db, fname, name ):
        """fname is the schematic to load from.  
        db is the parts database object.
        name is the name to give the schematic."""
        Bom.__init__(self)
        self.db = db
        self.name = name

        s = schem.open_schem(fname)

        for des,srcode in s.iteritems():
            if not self.has_key(srcode):
                self[srcode] = PartGroup( db[srcode], name )
            self[srcode].append((name,des))


class MultiBoardBom(Bom):
    def __init__(self, db):
        Bom.__init__(self)

        self.db = db

        # Array of 2-entry lists
        # 0: Number of boards
        # 1: Board
        self.boards = []

    def add_boards(self, board, num):
        """Add num boards to the collection.
        board must be a BoardBom instance."""

        # Already part of this collection?
        found = False
        for n in xrange(len(self.boards)):
            t = self.boards[n] 
            if t[1] == board:
                t[0] = t[0] + num
                found = True
                break

        if not found:
            self.boards.append( [num, board] )

        #### Update our PartGroup dictionary
        self.clear()
        
        for num, board in self.boards:
            
            # Mmmmm.  Horrible.
            for i in range(num):
                for srcode, bpg in board.iteritems():

                    if not self.has_key( srcode ):
                        self[srcode] = PartGroup( bpg.part )

                    self[srcode] += bpg

        
