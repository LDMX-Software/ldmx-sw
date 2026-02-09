from ._parameter_set import parameter_set, field
from .ldmxcfg import processor


@parameter_set
class MyParams:
    foo: str = 'bar'


@processor("hello", "world")
class MyClass:
    one: int = 1
    two: float = 2.0
    name: str = 'foo'
    vec: list[int] = [1, 2, 3]
    vec2d: list[float,float] = [[0.0, 1.0],[-1.0,0.0]]
    p: MyParams = field(default_factory=MyParams)
    __legacy__ = {
        'className': 'class_name', # test we can remap required parameters
        'Name': 'name', # test we can remap regular parameters
        'pfoo': 'p.foo' # test we can remap into submember parameter sets
    }


import unittest

class TestParameter(unittest.TestCase):
    def assertListEqual(self, lhs, rhs):
        self.assertEqual(len(lhs), len(rhs))
        for l, r in zip(lhs, rhs):
            self.assertEqual(l, r)

    def assertMyClass(self, c, *, one, two, name, vec):
        self.assertEqual(c.class_name, 'hello')
        self.assertEqual(c.one, one)
        self.assertEqual(c.two, two)
        self.assertEqual(c.name, name)
        self.assertListEqual(c.vec, vec)

    def test_defaults(self):
        c = MyClass()
        self.assertMyClass(c, one = 1, two = 2.0, name = 'foo', vec = [1, 2, 3])

    def test_change_after_creation(self):
        c = MyClass()
        c.one = 2
        self.assertMyClass(c, one = 2, two = 2.0, name = 'foo', vec = [1, 2, 3])
        c.name = 'bar'
        self.assertMyClass(c, one = 2, two = 2.0, name = 'bar', vec = [1, 2, 3])
        c.two *= 2
        self.assertMyClass(c, one = 2, two = 4.0, name = 'bar', vec = [1, 2, 3])
        c.vec = [4, 5]
        self.assertMyClass(c, one = 2, two = 4.0, name = 'bar', vec = [4, 5])
        self.assertEqual(c.p.foo, 'bar')
        c.p.foo = 'baz'
        self.assertEqual(c.p.foo, 'baz')

    def test_change_during_creation(self):
        c = MyClass(one = 2, two = 3.0, name = 'bar', vec = [4, 5], p = MyParams(foo = 'baz'))
        self.assertMyClass(c, one = 2, two = 3.0, name = 'bar', vec = [4, 5])
        self.assertEqual(c.p.foo, 'baz')

    def test_basic_errors(self):
        with self.assertRaises(TypeError):
            MyClass(one = 2.0)

        with self.assertRaises(TypeError):
            MyClass(vec = 2.0)

        with self.assertRaises(TypeError):
            MyClass(vec = [1.0])

        with self.assertRaises(TypeError):
            MyClass(vec = [1, 1.0])
            
        c = MyClass()
        with self.assertRaises(TypeError):
            c.one = 2.0

        with self.assertRaises(TypeError):
            c.vec = 2.0

        with self.assertRaises(KeyError):
            c.dne = 'does not exist'

        with self.assertRaises(KeyError):
            c.p.dne = 'does not exist'

        with self.assertRaises(TypeError):
            c.p.foo = 1

        with self.assertRaises(TypeError):
            c = MyClass(dne = 'does not exist')

        with self.assertRaises(TypeError):
            c = MyClass(p = 1)
    
    def test_meta_errors(self):
        with self.assertRaises(TypeError):
            @parameter_set()
            class BadVec:
                vec: list = [1, 1.0]

        with self.assertRaises(TypeError):
            @parameter_set()
            class Bad2DVec:
                vec: list = [[2, 3], 1]

    def test_legacy_remap(self):
        c = MyClass()
        with self.assertWarns(DeprecationWarning):
            c.pfoo = 'baz'
        self.assertEqual(c.p.foo, 'baz')
        with self.assertWarns(DeprecationWarning):
            c.Name = 'NewName'
        self.assertEqual(c.name, 'NewName')
        with self.assertWarns(DeprecationWarning):
            c.className = 'OldClass'
        self.assertEqual(c.class_name, 'OldClass')


if __name__ == '__main__':
    unittest.main()

