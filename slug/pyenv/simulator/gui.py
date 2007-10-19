import pygame
from pgu import gui

WHITE = (255,255,255)
BLACK = (0, 0, 0)

class GUI:
    def __init__(self, step):
        self.step = step
        self.altered = False
        self.curline = 0
        self.form = gui.Form()
        
        class DebugConsole(gui.Table):
            def __init__(self,**params):
                WIDTH = 200
                HEIGHT = 640

                gui.Table.__init__(self, **params)
                
                self.tr()
                self.td(gui.Label("Debug Console", color=WHITE), colspan=1)
                self.tr()
                
                self.codelist = gui.TextArea("", width=200, height=400,size=10)

                self.td(self.codelist)
                self.tr()
                
                self.stepbutton = gui.Button("Step")
                self.td(self.stepbutton)

        self.app = gui.App()
        self.t = DebugConsole()

        self.t.stepbutton.connect(gui.CLICK, self.pressed_step, None)

        self.rlines = [a.rstrip() for a in open("robot.py").readlines()]
        print "\n".join(self.rlines)

        c = gui.Container(align=-1,valign=-1)
        c.add(self.t, 640, 0)
        self.app.init(c)

        self.mask = pygame.Rect(640, 0, 200, 640)

    def showline(self, no):
        if no != self.curline:
            rlines = self.rlines

            no = no - 1

            start = no - 5
            if start < 0: start = 0
            end = no + 5
            if end >= len(rlines): end = len(rlines)-1

            body = "\n".join(rlines[start:no])
            body = body + "\n--> " + rlines[no]
            body = body + "\n".join(rlines[no+1:end])
            self.t.codelist.value = body

            self.altered = True
            self.curline = no

    def pressed_step(self, arg):
        self.step()

    def process_event(self, e):
        self.altered = True
        self.app.event(e)

    def drawgui(self, screen, dirty):
        if self.altered:
            screen.fill(BLACK, self.mask)
            self.app.paint(screen)
            dirty.append(self.mask)
            self.altered = False
