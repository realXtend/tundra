class MouseInfo:
    def __init__(self, x, y, x_rel, y_rel):
        self.setInfo(x, y, x_rel, y_rel)
        
    def setInfo(self, x, y, x_rel, y_rel):
        self.x = x
        self.y = y
        self.x_rel = x_rel
        self.y_rel = y_rel

"""
class MouseInfo:
    instance = None

    class __impl:
        def __init__(self, x, y, x_rel, y_rel):
            self.setInfo(x, y, x_rel, y_rel)
            
        def setInfo(self, x, y, x_rel, y_rel):
            self.x = x
            self.y = y
            self.x_rel = x_rel
            self.y_rel = y_rel
            
        def test(self):
            return id(self), self.x, self.y

    def __init__(self, x, y, x_rel, y_rel):
        
        if MouseInfo.instance is None:
            MouseInfo.instance = MouseInfo.__impl(x, y, x_rel, y_rel)
        else:
            MouseInfo.instance.setInfo(x, y, x_rel, y_rel)
            
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
