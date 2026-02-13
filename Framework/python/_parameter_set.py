"""development file for python config parameter sets

Just run this file to run the tests.

    [denv] python3 processor.py -v

The tests are at the bottom and the code that would be
in the configuration pyton module is at the top.
"""

import warnings
from dataclasses import dataclass, field
from typing import Any

def check_list(l, dimension, entry_type):
    if dimension == 1:
        if entry_type is not Any and any(not isinstance(e, entry_type) for e in l):
            raise TypeError(f'An entry in the list is not of expected type {entry_type}')
    elif dimension == 2:
        for ll in l:
            check_list(ll, 1, entry_type)
    else:
        raise Exception(f'List with dimension {dimension} not supported.')

def validate_and_set_attr(self, attr, val):
    # first, we check if the attr is a legacy name
    legacy_remap = getattr(self.__class__, '__legacy__', None)
    if legacy_remap is not None and attr in legacy_remap:
        new_attr = legacy_remap[attr]
        warnings.warn(
            f"Legacy name '{attr}' has been replaced by '{new_attr}'.",
            DeprecationWarning,
            #stacklevel=2
        )

        # the attr is now updated to new name
        attr = new_attr
        member, _dot, submember = attr.partition('.')
        if submember:
            # we are reaching into a sub-class and so we recurse into it
            if member not in self.__dataclass_fields__:
                raise KeyError(f'Attribute {member} not a member of {self.__class__.__name__}')
            # validate intermediate type?
            # I can't think of a reason to do this because we only reach
            # here in the case where a developer is remapping a legacy
            # parameter into a new submember parameter
    
            return validate_and_set_attr(
                self.__dict__[member],
                submember,
                val
            )
        # no submember, just fall through after we have changed 'attr'

    if attr not in self.__dataclass_fields__:
        raise KeyError(f'Attribute {attr} not a member of {self.__class__.__name__}')

    expected_type = self.__dataclass_fields__[attr].type
    if isinstance(val, list):
        expected_dimension  = self.__dataclass_fields__[attr].metadata['dimension']
        expected_entry_type = self.__dataclass_fields__[attr].metadata['entry_type']
        check_list(val, expected_dimension, expected_entry_type)
    elif not isinstance(val, expected_type):
        raise TypeError(f'Attribute {attr} should be type {expected_type} instead of type {type(val)}.')

    self.__dict__[attr] = val


class create_list:
    def __init__(self, l):
        self._list = l

    def __call__(self):
        return self._list


def parameter_set(
    _class = None, *,
    post_init = None,
    helpers = [],
    **required_parameters
):
    def _decorator_impl(cls):
        # update post_init function to include library loading
        if post_init is not None:
            org_post_init = (
                getattr(cls, '__post_init__')
                if hasattr(cls, '__post_init__') else
                lambda self: None
            )
            def _full_post_init(self):
                # make sure all dataclass fields are present in __dict__
                # since that is the mechanism we use to grab them in C++
                for name, field in self.__dataclass_fields__.items():
                    if name not in self.__dict__:
                        self.__dict__[name] = getattr(self, name)
                post_init(self)
                org_post_init(self)
    
            cls.__post_init__ = _full_post_init
        
        # update setattr function to validate stuff as well
        cls.__setattr__ = validate_and_set_attr

        # add additional annotations that are required parameters
        for name, value in required_parameters.items():
            if type(value) is list:
                cls.__annotations__[name] = list[Any]
                setattr(cls, name,
                        field(
                            default_factory = create_list(value),
                            init=False,
                            metadata = {
                                'entry_type': Any,
                                'dimension': 1
                            }
                        )
                    )
            else:
                cls.__annotations__[name] = type(value)
                setattr(cls, name, field(default = value, init=False))

        # add additional helpers
        for name, func in helpers:
            setattr(cls, name, func)

        # process class variables so that mutable classes (mainly list)
        # are given to dataclass as a default_factory instead of a default
        for var in vars(cls):
            # filter out special "dunder" class variables
            # that start and end with double underscore
            if var.startswith('__') and var.endswith('__'):
                continue
            # only look at variables that have annotations with them
            # (as is done with dataclass)
            if var not in cls.__annotations__:
                continue
            the_value = getattr(cls, var)
            if isinstance(the_value, list):
                type_args = cls.__annotations__[var].__args__

                dimension = len(type_args)
                if dimension == 0:
                    raise TypeError('Python configuration parameter_sets need to annotate some content for each list.')
                if dimension > 2:
                    raise TypeError('Python configuration parameter_sets are limited to 1D or 2D lists.')

                entry_type = type_args[0]
                if not all(entry_type is t for t in type_args):
                    raise TypeError('Python configuration parameter_sets must have all entries be the same type.')

                check_list(the_value, dimension, entry_type)
                setattr(
                    cls, var,
                    field(
                        default_factory=create_list(the_value),
                        metadata = {
                            'entry_type': entry_type,
                            'dimension': dimension
                        }
                    )
                )
        return dataclass(slots=False)(cls)

    if _class is None:
        return _decorator_impl
    else:
        return _decorator_impl(_class)
