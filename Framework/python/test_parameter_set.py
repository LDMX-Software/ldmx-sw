from . import Processor, processor
from ._parameter_set import field, parameter_set


@parameter_set
class MyParams:
    foo: str = "what"


@processor("hello", "world")
class MyClass(Processor):
    one: int = 1
    two: float = 2.0
    name: str = "foo"
    vec: list[int] = [1, 2, 3]
    vec2d: list[float, float] = [[0.0, 1.0], [-1.0, 0.0]]
    p: MyParams = MyParams()
    __legacy__ = {
        "className": "class_name",  # test we can remap required parameters
        "Name": "name",  # test we can remap regular parameters
        "pfoo": "p.foo",  # test we can remap into submember parameter sets
    }


@parameter_set
class RequireList:
    the_list: list[int]


import unittest


class TestParameter(unittest.TestCase):
    def assertMyClass(self, c, **kwargs):
        """We need the __dict__ member to hold all of the parameters
        we want on the C++ side so we compare the __dict__ to a construction
        of the defaults with any updates we expect passed in through
        key-word arguments"""

        dict_defaults = {
            "class_name": "hello",
            "module_name": "world",
            "instance_name": "hello",
            "histograms": [],
            "one": 1,
            "two": 2.0,
            "name": "foo",
            "vec": [1, 2, 3],
            "p": MyParams(),
            "vec2d": [[0.0, 1.0], [-1.0, 0.0]],
        }
        dict_defaults.update(kwargs)
        self.assertDictEqual(c.__dict__, dict_defaults)

    def test_required_list(self):
        o = RequireList([1, 2, 3])
        self.assertEqual(o.the_list, [1, 2, 3])

        with self.assertRaises(TypeError):
            o2 = RequireList()


    def test_fail_require_after_opt(self):
        with self.assertRaises(TypeError):
            @parameter_set
            class RequireList2(MyParams):
                the_list: list[int]

    def test_defaults(self):
        c = MyClass()
        self.assertMyClass(c)

    def test_change_required_parameter(self):
        c = MyClass()
        self.assertMyClass(c)
        c.class_name = "baz"
        c.instance_name = "cowabunga"
        self.assertMyClass(c, class_name="baz", instance_name="cowabunga")

    def test_change_after_creation(self):
        c = MyClass()
        c.one = 2
        self.assertMyClass(c, one=2)
        c.name = "bar"
        self.assertMyClass(c, one=2, name="bar")
        c.two *= 2
        self.assertMyClass(c, one=2, two=4.0, name="bar")
        c.vec = [4, 5]
        self.assertMyClass(c, one=2, two=4.0, name="bar", vec=[4, 5])
        self.assertEqual(c.p.foo, "what")
        c.p.foo = "baz"
        self.assertEqual(c.p.foo, "baz")

    def test_change_during_creation(self):
        c = MyClass(one=2, two=3.0, name="bar", vec=[4, 5], p=MyParams(foo="baz"))
        self.assertMyClass(
            c, one=2, two=3.0, name="bar", vec=[4, 5], p=MyParams(foo="baz")
        )
        self.assertEqual(c.p.foo, "baz")

    def test_basic_errors(self):
        with self.assertRaises(TypeError):
            MyClass(one=2.0)

        with self.assertRaises(TypeError):
            MyClass(vec=2.0)

        with self.assertRaises(TypeError):
            MyClass(vec=[1.0])

        with self.assertRaises(TypeError):
            MyClass(vec=[1, 1.0])

        c = MyClass()
        with self.assertRaises(TypeError):
            c.one = 2.0

        with self.assertRaises(TypeError):
            c.vec = 2.0

        with self.assertRaises(KeyError):
            c.dne = "does not exist"

        with self.assertRaises(KeyError):
            c.p.dne = "does not exist"

        with self.assertRaises(TypeError):
            c.p.foo = 1

        with self.assertRaises(TypeError):
            c = MyClass(dne="does not exist")

        with self.assertRaises(TypeError):
            c = MyClass(p=1)

    def test_meta_errors(self):
        with self.assertRaises(TypeError):

            @parameter_set()
            class BadVec:
                vec: list[int] = [1, 1.0]

        with self.assertRaises(TypeError):

            @parameter_set()
            class Bad2DVec:
                vec: list[int] = [[2, 3], 1]

    def test_legacy_remap(self):
        c = MyClass()
        with self.assertWarns(DeprecationWarning):
            c.pfoo = "baz"
        self.assertEqual(c.p.foo, "baz")
        with self.assertWarns(DeprecationWarning):
            c.Name = "NewName"
        self.assertEqual(c.name, "NewName")
        with self.assertWarns(DeprecationWarning):
            c.className = "OldClass"
        self.assertEqual(c.class_name, "OldClass")


if __name__ == "__main__":
    unittest.main()
