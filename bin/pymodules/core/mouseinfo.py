class MouseInfo:
    def __init__(self, x, y, rel_x, rel_y):
        self.setInfo(x, y, rel_x, rel_y)
        
    def setInfo(self, x, y, rel_x, rel_y):
        self.x = x
        self.y = y
        self.rel_x = rel_x
        self.rel_y = rel_y

"""
class MouseInfo:
    instance = None

    class __impl:
        def __init__(self, x, y, rel_x, rel_y):
            self.setInfo(x, y, rel_x, rel_y)
            
        def setInfo(self, x, y, rel_x, rel_y):
            self.x = x
            self.y = y
            self.rel_x = rel_x
            self.rel_y = rel_y
            
        def test(self):
            return id(self), self.x, self.y

    def __init__(self, x, y, rel_x, rel_y):
        
        if MouseInfo.instance is None:
            MouseInfo.instance = MouseInfo.__impl(x, y, rel_x, rel_y)
        else:
            MouseInfo.instance.setInfo(x, y, rel_x, rel_y)
            
        self.__dict__['_MouseInfo__instance'] = MouseInfo.instance

    def __getattr__(self, attr):
        return getattr(self.instance, attr)

    def __setattr__(self, attr, value):
        return setattr(self.instance, attr, value)

if __name__ == '__main__':
    s1 = MouseInfo(5, 3, 10, 10)
    s1.x = 7
    print s1, id(s1), s1.test()

    s2 = MouseInfo(6, 6, 11, 11)
    print s2, id(s2), s2.test()
"""
