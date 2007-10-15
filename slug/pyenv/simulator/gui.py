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
                
                self.codelist = gui.List(width=WIDTH, height=400)

                self.td(self.codelist)
                self.tr()
                
                self.stepbutton = gui.Button("Step")
                self.td(self.stepbutton)

        self.app = gui.App()
        self.t = DebugConsole()

        self.t.stepbutton.connect(gui.CLICK, self.pressed_step, None)

        rlines = open("robot.py").readlines()
        for i in range(len(rlines)):
            self.t.codelist.add(rlines[i][:-1], value=i)
        self.t.codelist.repaint()

        c = gui.Container(align=-1,valign=-1)
        c.add(self.t, 640, 0)
        self.app.init(c)

        self.mask = pygame.Rect(640, 0, 200, 640)

    def showline(self, no):
        if no != self.curline:
            self.altered = True
            self.t.codelist.group.value = no-1
            self.t.codelist.repaint()
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
