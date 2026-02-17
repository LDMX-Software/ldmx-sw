"""development file for python config parameter sets

Just run this file to run the tests.

    [denv] python3 processor.py -v

The tests are at the bottom and the code that would be
in the configuration pyton module is at the top.
"""

import copy
import types
import warnings
from dataclasses import Field, dataclass, field
from typing import Any


def check_list(attr, l, dimension, entry_type):
    if dimension == 1:
        if entry_type is not Any:
            for e in l:
                if type(e) is not entry_type:
                    raise TypeError(
                        f"Entry {e} in parameter {attr} is not of expected type {entry_type}"
                    )
    elif dimension == 2:
        for ll in l:
            check_list(attr, ll, 1, entry_type)
    else:
        raise Exception(f"List with dimension {dimension} not supported.")


def validate_and_set_attr(self, attr, val):
    # first, we check if the attr is a legacy name
    legacy_remap = getattr(self.__class__, "__legacy__", None)
    if legacy_remap is not None and attr in legacy_remap:
        new_attr = legacy_remap[attr]
        warnings.warn(
            f"Legacy name '{attr}' has been replaced by '{new_attr}'.",
            DeprecationWarning,
            # stacklevel=2
        )

        # the attr is now updated to new name
        attr = new_attr
        member, _dot, submember = attr.partition(".")
        if submember:
            # we are reaching into a sub-class and so we recurse into it
            if member not in self.__dataclass_fields__:
                raise KeyError(
                    f"Attribute {member} not a member of {self.__class__.__name__}"
                )
            # validate intermediate type?
            # I can't think of a reason to do this because we only reach
            # here in the case where a developer is remapping a legacy
            # parameter into a new submember parameter

            return validate_and_set_attr(self.__dict__[member], submember, val)
        # no submember, just fall through after we have changed 'attr'

    if attr not in self.__dataclass_fields__:
        raise KeyError(f"Attribute {attr} not a member of {self.__class__.__name__}")

    expected_type = self.__dataclass_fields__[attr].type
    if isinstance(val, list):
        expected_dimension = self.__dataclass_fields__[attr].metadata["dimension"]
        expected_entry_type = self.__dataclass_fields__[attr].metadata["entry_type"]
        check_list(attr, val, expected_dimension, expected_entry_type)
    elif expected_type is not Any and not isinstance(val, expected_type):
        raise TypeError(
            f"Attribute {attr} should be type {expected_type} instead of type {type(val)}."
        )

    self.__dict__[attr] = val


class create_default:
    def __init__(self, o):
        self._default = o

    def __call__(self):
        """we want a _full copy_ of the default so the new object can edit it as they wish"""
        return copy.deepcopy(self._default)


def parameter_set(
    _class=None, *, post_init=None, helpers=[], required_base=None,
    **required_parameters
):
    def _decorator_impl(cls):
        if required_base is not None and required_base not in cls.__bases__:
            raise TypeError(f"{cls.__name__} is required to have base {required_base}")

        passed_post_init = (
            post_init
            if post_init is not None
            else lambda self: None
        )

        orig_post_init = (
            cls.__post_init__
            if hasattr(cls, "__post_init__")
            else lambda self: None
        )

        # update post_init function to include some kind of common tasks
        def _full_post_init(self):
            # make sure all dataclass fields are present in __dict__
            # since that is the mechanism we use to grab them in C++
            for name, field in self.__dataclass_fields__.items():
                if name not in self.__dict__:
                    self.__dict__[name] = getattr(self, name)
            passed_post_init(self)
            orig_post_init(self)

        cls.__post_init__ = _full_post_init

        # update setattr function to validate stuff as well
        cls.__setattr__ = validate_and_set_attr

        # add additional annotations that are required parameters
        for name, value in required_parameters.items():
            if type(value) is list:
                cls.__annotations__[name] = list[Any]
            elif type(value) is Field:
                cls.__annotations__[name] = type(value.default)
            else:
                cls.__annotations__[name] = type(value)
            setattr(cls, name, value)

        # add additional helpers
        for name, func in helpers:
            setattr(cls, name, func)

        # process class annotations so that mutable classes
        # are given to dataclass as a default_factory instead of a default
        for name, the_type in cls.__annotations__.items():
            the_value = getattr(cls, name, None)
            simple_types = (bool, int, float, str)

            if the_type in simple_types:
                # type is simple and can be left alone for dataclass to handle
                continue

            if isinstance(the_value, Field):
                # value already specified as dataclass field,
                # assume user knows what they are doing and skip
                continue

            if isinstance(the_type, types.GenericAlias) and the_type.__origin__ is list:
                # special list deduction to do early warnings on bad dimensions
                # and check entries in default value
                type_args = getattr(the_type, "__args__", ())
                dimension = len(type_args)
                if dimension == 0:
                    raise TypeError(
                        "Python configuration parameter_sets need to annotate some content for each list."
                    )
                if dimension > 2:
                    raise TypeError(
                        "Python configuration parameter_sets are limited to 1D or 2D lists."
                    )

                entry_type = type_args[0]
                if not all(entry_type is t for t in type_args):
                    raise TypeError(
                        "Python configuration parameter_sets must have all entries be the same type."
                    )

                field_kwargs = dict(
                    metadata=dict(entry_type=entry_type, dimension=dimension)
                )
                if the_value is not None:
                    check_list(name, the_value, dimension, entry_type)
                    field_kwargs["default_factory"] = create_default(the_value)
                setattr(cls, name, field(**field_kwargs))
            else:
                # type is not simple, not already a dataclasses.Field, and not a list so we wrap it
                # in a creation function to get around dataclass's prevention on using mutable defaults
                setattr(cls, name, field(default_factory=create_default(the_value)))

        return dataclass(slots=False)(cls)

    if _class is None:
        return _decorator_impl
    else:
        return _decorator_impl(_class)
