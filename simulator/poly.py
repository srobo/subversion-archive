import pygame
import math

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

class poly:
    def __init__(self, points, direction, width):
        self.points = []
        for point in points:
            self.points.append(list(point))

        self.colour = WHITE
        self.direction = list(direction)
        self.width = width
        self.rotation = 0.0
    
    def blit(self, surface, dirty):
        dirty.append(pygame.draw.polygon(surface, self.colour, self.points,
            self.width))

    def move(self):
        for i in range(len(self.points)):
            self.points[i][0] += self.direction[0]
            self.points[i][1] += self.direction[1]
    
    def rotate(self, rads, centre, screen, dirty):
        #Find center of rotation
        #It's relative to p0

        cx = centre[0]
        cy = centre[1]

        p0x = self.points[0][0]
        p0y = self.points[0][1]
        

        x = p0x# + cx * math.cos(self.rotation) + \
               #   cy * math.sin(self.rotation)
        y = p0y# + cx * math.sin(self.rotation) + \
               #   cy * math.cos(self.rotation)
        
        dirty.append(pygame.draw.circle(screen, (255, 0, 0), (x, y), 1))


        print "Centre: %f, %f" % (x, y)

        #Now move each point rads relative to xy
        for i in range(len(self.points)):
            px = self.points[i][0] - x
            py = self.points[i][1] - y

            if py == 0 and px == 0:
                continue

            r = math.sqrt(px*px+py*py)
            if px == 0:
                if py > 0:
                    a = 3 * (math.pi / 2)
                else:
                    a = 1 * (math.pi / 2)
            else:
                a = math.atan(py/px)

            if px < 0:
                a = math.pi + a

            #Now in polar, rotate point
            a += rads

            #Back to cartesian
            print "From %f,%f (%f, %f)" % (px,py, r, a),
            px = r * math.cos(a)
            py = r * math.sin(a)
            print " to %f,%f (%f, %f)" % (px, py, r, a)

            self.points[i][0] = px + x
            self.points[i][1] = py + y

        self.rotation += rads


    def wipe(self, surface, dirty):
        dirty.append(pygame.draw.polygon(surface, BLACK, self.points, 
            self.width))

    def collide(self, other):
        #Taken from the seahorse book
        opoints = other.points
        for i in range(len(opoints)):
            p1 = opoints[i]
            p2 = opoints[(i+1)%len(opoints)] #Each pair in turn of
            for j in range(len(self.points)):
                p3 = self.points[j]
                p4 = self.points[(j+1)%len(self.points)]

                #do the lines overlap?
                #First a quick rejection test
                if((max(p1[0], p2[0]) >= min(p3[0], p4[0]) and \
                    max(p3[0], p4[0]) >= min(p1[0], p2[0]) and \
                    max(p1[1], p2[1]) >= min(p3[1], p4[1]) and \
                    max(p3[1], p4[1]) >= min(p1[1], p2[1]))):
                    
                    #Do they straddle?
                    z1 = ((p3[0] - p1[0])*(p2[1] - p1[1])) - \
                            ((p3[1] - p1[1]) * (p2[0] - p1[0]))
                    
                    if z1 < 0:
                        s1 = -1
                    elif z1 > 0:
                        s1 = 1
                    else:
                        s1 = 0

                    z2 = ((p4[0] - p1[0]) * (p2[1] - p2[1])) - \
                            ((p4[1] - p1[1]) * (p2[0] - p1[0]))
                    
                    if z2 < 0:
                        s2 = -1
                    elif z2 > 0:
                        s2 = 1
                    else:
                        s2 = 0

                    if (s1==0 or s2==0) or (s1 != s2):
                        #They overlap
                        print "overlap"
                        return True

        return False

if __name__ == "__main__":
    pygame.init()
    screen = pygame.display.set_mode((640, 480))
    clock = pygame.time.Clock()

    polys = []
    polys.append(poly([(100, 100), (200, 100), (200, 200)], (0, 0.1), 1))
    polys.append(poly([(300, 100), (250, 200), (300, 300)], (0.1, 0), 1))

    polys.append(poly([(5, 5), (635, 5), (635, 475), (5, 475)], (0, 0), 10))

    while True:
        clock.tick(5)
        dirty = []

        for i in range(len(polys)):
            poly = polys[i]
            opolys = polys[:i] + polys[i+1:]

            poly.wipe(screen, dirty)
            if i == 0:
                poly.rotate(0.1, (0, 0), screen, dirty)
            poly.move()

            for o in opolys:
                if poly.collide(o):
                    #poly.direction[0] *= -1
                    #poly.direction[1] *= -1
                    print "Collidey death"

            poly.blit(screen, dirty)

        pygame.display.update(dirty)
