#!/usr/bin/env python

import math
import cairo


class Display:
    def __init__(self):
        self.WIDTH = 256
        self.HEIGHT =  72
        self.XINC = False
        self.YINC = False
        self.ramx = 0
        self.ramy = 0

        self.surface = cairo.ImageSurface (cairo.FORMAT_ARGB32, self.WIDTH, self.HEIGHT)
        self.ctx = cairo.Context(self.surface)

        


    def data(self,data):
        val = (data&0x0f)/15.0
        print val
        self.ctx.set_source_rgb(val, val, val)
        self.ctx.set_line_width(0.01)

        if False: # tried the antialiasing fix as rob suggested
            self.ctx.rectangle(self.ramx+0.5,self.ramy+0.5,1,1)
        else:
            self.ctx.rectangle(self.ramx,self.ramy,1,1)
        self.ctx.fill()
        


        #auto increment counter position
        if self.XINC and not self.YINC:
            self.ramx = self.ramx+1
            if self.ramx>self.WIDTH:
                self.ramx =0

        elif self.YINC and not self.XINC:
            self.ramy = self.ramy+1
            if self.ramy>self.HEIGHT:
                self.ramy =0

        elif self.YINC and self.XINC:
            self.ramx = self.ramx+1 # go accross
            if self.ramx>self.WIDTH:
                self.ramx =0    
        
                self.ramy = self.ramy+1 # only drop as reach end
                if self.ramy>self.HEIGHT:
                    self.ramy =0

        

    def command(self,caddr,cdata):
        if caddr==0x15:#scan setup - GSADDINC
            # [1:0] = yinc,xinc
            if (cdata&0x01):
                self.XINC = True
            else:
                self.XINC = False
            if (cdata&0x02):
                self.YINC = True
            else:
                self.YINC = False

        elif caddr==0x13: # Ramxstart
            self.ramx = cdata
        elif caddr==0x14: # Ramystart
            self.ramy = cdata
        else:
            raise MyError("Unimplemented command%x"%caddr)
    



    def test2(self):
        self.ctx.set_source_rgb(1, 0, 0)
        self.ctx.set_line_width(0.01)
        for i in range(0,self.WIDTH):
            for j in  range(0,self.HEIGHT):
                if (i%2==0)and(j%2==0):
                    print i,j
                    self.ctx.rectangle(i,j,1,1)
                    self.ctx.fill()
        

    def test(self):
        self.ctx.scale (self.WIDTH/1.0, self.HEIGHT/1.0) # Normalizing the canvas
        pat = cairo.LinearGradient (0.0, 0.0, 0.0, 1.0)
        pat.add_color_stop_rgba (1, 0.7, 0, 0, 0.5) # First stop, 50% opacity
        pat.add_color_stop_rgba (0, 0.9, 0.7, 0.2, 1) # Last stop, 100% opacity

        self.ctx.rectangle (0, 0, 1, 1) # Rectangle(x0, y0, x1, y1)
        self.ctx.set_source (pat)
        self.ctx.fill ()

        self.ctx.translate (0.1, 0.1) # Changing the current transformation matrix

        self.ctx.move_to (0, 0)
        self.ctx.arc (0.2, 0.1, 0.1, -math.pi/2, 0) # Arc(cx, cy, radius, start_angle, stop_angle)
        self.ctx.line_to (0.5, 0.1) # Line to (x,y)
        self.ctx.curve_to (0.5, 0.2, 0.5, 0.4, 0.2, 0.8) # Curve(x1, y1, x2, y2, x3, y3)
        self.ctx.close_path ()
        
        self.ctx.set_source_rgb (0.3, 0.2, 0.5) # Solid color
        self.ctx.set_line_width (0.02)
        self.ctx.stroke ()
        

    def render(self):
        self.surface.write_to_png ("example2.png") # Output to PNG
    def makegobj(self):
        #return a widget or something???
        pass
    


if __name__ == "__main__":
    scr = Display()
    #scr.test()
    
    scr.command(0x15,0x2+0x1) # auto increment
    

    for i in range(0,scr.HEIGHT*scr.WIDTH):
        if i%2 ==0:
            bright = 0x15
        else:
            bright = 0
        scr.data(bright)

    scr.command(0x13,10)
    scr.command(0x14,20)
    for j in range(0,40):
        scr.data(0)
    
    

    
    scr.render()
